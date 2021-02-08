#include "Socket.h"
#include <Literals>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <QtCore/QDebug>
#include <QtCore/QLoggingCategory>
#include <QtCore/QSocketNotifier>

#define BUFFER_SIZE 8192

Q_LOGGING_CATEGORY(hyperspaceSocketDC, "hyperspace.socket", DEBUG_MESSAGES_DEFAULT_LEVEL)

inline ssize_t
sock_fd_write(int sock, void *buf, ssize_t buflen, int fd)
{
    ssize_t     size;
    struct msghdr   msg;
    struct iovec    iov;
    union {
        struct cmsghdr  cmsghdr;
        char        control[CMSG_SPACE(sizeof (int))];
    } cmsgu;
    struct cmsghdr  *cmsg;

    iov.iov_base = buf;
    iov.iov_len = buflen;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    if (fd != -1) {
        msg.msg_control = cmsgu.control;
        msg.msg_controllen = sizeof(cmsgu.control);

        cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_len = CMSG_LEN(sizeof (int));
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;

        *((int *) CMSG_DATA(cmsg)) = fd;
    } else {
        msg.msg_control = NULL;
        msg.msg_controllen = 0;
    }

    size = sendmsg(sock, &msg, 0);

    if (size < 0) {
        // Return error appropriately
        return -errno;
    }

    return size;
}

inline ssize_t
sock_fd_read(int sock, void *buf, ssize_t bufsize, int *fd)
{
    ssize_t     size;

    struct msghdr   msg;
    struct iovec    iov;
    union {
        struct cmsghdr  cmsghdr;
        char        control[CMSG_SPACE(sizeof (int))];
    } cmsgu;
    struct cmsghdr  *cmsg;

    iov.iov_base = buf;
    iov.iov_len = bufsize;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsgu.control;
    msg.msg_controllen = sizeof(cmsgu.control);
    size = recvmsg (sock, &msg, 0);
    if (size < 0) {
        // Return error appropriately
        return -errno;
    }

    if (fd) {
        cmsg = CMSG_FIRSTHDR(&msg);
        if (cmsg && cmsg->cmsg_len == CMSG_LEN(sizeof(int))) {
            if (cmsg->cmsg_level != SOL_SOCKET) {
                fprintf (stderr, "invalid cmsg_level %d\n",
                        cmsg->cmsg_level);
            }
            if (cmsg->cmsg_type != SCM_RIGHTS) {
                fprintf (stderr, "invalid cmsg_type %d\n",
                        cmsg->cmsg_type);
            }

            *fd = *((int *) CMSG_DATA(cmsg));
        } else {
            *fd = -1;
        }
    }

    return size;
}

namespace Hyperspace {

class Socket::Private
{
public:
    Private(Socket *q) : q(q), socketFd(-1), lastFd(-1), socketReadyToWrite(true) {}

    Socket *q;

    QString serverPath;

    int socketFd;
    QSocketNotifier *notifier;
    QSocketNotifier *writeNotifier;

    QList< QPair< QByteArray, int > > messageQueue;

    QByteArray internalBuffer;
    int lastFd;
    bool socketReadyToWrite;

    // Q_PRIVATE_SLOT
    void writeQueue();
};

void Socket::Private::writeQueue()
{
    qCDebug(hyperspaceSocketDC) << Q_FUNC_INFO;

    if (Q_UNLIKELY(!q->isReady())) {
        // We are ready!
        q->setReady();
    }

    // Do we have data to be written?
    QByteArray toBeWritten;
    int fdToBeWritten = -1;
    if (!internalBuffer.isEmpty()) {
        toBeWritten = internalBuffer;
        fdToBeWritten = lastFd;

        internalBuffer.clear();
        lastFd = -1;
    } else if (!messageQueue.isEmpty()) {
        toBeWritten = messageQueue.first().first;
        fdToBeWritten = messageQueue.first().second;

        messageQueue.removeAt(0);
    } else {
        // Nothing to do. Disable the notifier (and empty the cache, to be sure)
        writeNotifier->setEnabled(false);
        internalBuffer.clear();
        lastFd = -1;
        return;
    }

    // go
    int written = sock_fd_write(socketFd, toBeWritten.data(), toBeWritten.size(), fdToBeWritten);

    if (Q_UNLIKELY(written < 0)) {
        int error = 0 - written;
        qCWarning(hyperspaceSocketDC) << "Writing to socket failed with error: " << error;
        qCDebug(hyperspaceSocketDC) << "Dropping buffered payload!";

        // If a write error occurred, let's just drop the payload.
    } else if (written < toBeWritten.size()) {
        internalBuffer = toBeWritten.right(toBeWritten.size() - written);
        lastFd = fdToBeWritten;
    }

    // Enable the notifier
    writeNotifier->setEnabled(true);
}

Socket::Socket(const QString& serverPath, QObject* parent)
    : AsyncInitObject(parent)
    , d(new Private(this))
{
    d->serverPath = serverPath;
}

Socket::Socket(int fd, QObject* parent)
    : AsyncInitObject(parent)
    , d(new Private(this))
{
    d->socketFd = fd;
}

Socket::~Socket()
{
    if (d->socketFd > 0) {
        ::close(d->socketFd);
    }

    delete d;
}

void Socket::initImpl()
{
    int len, flags;
    bool socketExists = d->socketFd > 0;
    struct sockaddr_un remote;

    if (!socketExists) {
        if ((d->socketFd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
            setInitError(QLatin1String(Hemera::Literals::Errors::badRequest()), QString::fromLatin1(strerror(errno)));
            return;
        }
    }

    flags = fcntl(d->socketFd,F_GETFL,0);
    fcntl(d->socketFd, F_SETFL, flags | O_NONBLOCK);

    if (!socketExists) {
        remote.sun_family = AF_UNIX;
        // We need size() +1, as \0 is not counted in size.
        strncpy(remote.sun_path, d->serverPath.toLatin1().constData(), d->serverPath.size() + 1);
        qCInfo(hyperspaceSocketDC) << "Connecting to " << remote.sun_path;
        len = strlen(remote.sun_path) + sizeof(remote.sun_family);
        if (::connect(d->socketFd, (struct sockaddr *)&remote, len) == -1) {
            if (errno == EINPROGRESS) {
                // We assume it's alright to be here, the notifier will do its job.
            } else {
                setInitError(QLatin1String(Hemera::Literals::Errors::badRequest()), QString::fromLatin1(strerror(errno)));
                return;
            }
        }
    }

    // Raise our read notifier
    d->notifier = new QSocketNotifier(d->socketFd, QSocketNotifier::Read, this);
    connect(d->notifier, &QSocketNotifier::activated, this, [this] {
        // Read data
        int fd = -1;
        QByteArray buf;
        ssize_t dataRead;
        while (true) {
            buf.resize(buf.size() + BUFFER_SIZE);
            // It might be that fd has been set on a previous call, if we're really unlucky.
            if (fd > 0) {
                dataRead = sock_fd_read(d->socketFd, buf.data() + (buf.size() - BUFFER_SIZE), BUFFER_SIZE, NULL);
            } else {
                dataRead = sock_fd_read(d->socketFd, buf.data() + (buf.size() - BUFFER_SIZE), BUFFER_SIZE, &fd);
            }

            // Resize the ba again
            buf.resize(buf.size() - BUFFER_SIZE + dataRead);

            if (dataRead == 0) {
                if (buf.size() - BUFFER_SIZE >= 0) {
                    Q_EMIT readyRead(buf, fd);
                }

                qCInfo(hyperspaceSocketDC) << "Connection closed";
                d->notifier->setEnabled(false);
                Q_EMIT disconnected();
                return;
            } else if (dataRead < 0) {
                // Handle errors
                int error = 0 - dataRead;
                qCWarning(hyperspaceSocketDC) << "Dataread failed with " << error;
                qCDebug(hyperspaceSocketDC) << "Dropping buffered payload!";
                break;
            }

            // We move forward in the cycle only if we still have things to get from the socket.
            int count;
            ioctl(d->socketFd, FIONREAD, &count);
            if (count == 0) {
                break;
            }
        }

        // Handle our buffer
        Q_EMIT readyRead(buf, fd);
    });

    // Raise our write notifier
    d->writeNotifier = new QSocketNotifier(d->socketFd, QSocketNotifier::Write, this);
    connect(d->writeNotifier, SIGNAL(activated(int)), this, SLOT(writeQueue()));

    if (socketExists) {
        // We're already ready
        setReady();
    }
}

int Socket::write(QByteArray data, int fd)
{
    d->messageQueue.append(qMakePair(data, fd));

    // Force the queue only if the write notifier is not enabled.
    if (!d->writeNotifier->isEnabled()) {
        d->writeQueue();
    }

    return data.size();
}

}

#include "moc_Socket.cpp"
