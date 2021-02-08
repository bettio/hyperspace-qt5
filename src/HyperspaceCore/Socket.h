#ifndef HYPERSPACE_SOCKET_H
#define HYPERSPACE_SOCKET_H

#include <HemeraCore/AsyncInitObject>

namespace Hyperspace {

class Socket : public Hemera::AsyncInitObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Socket)

    Q_PRIVATE_SLOT(d, void writeQueue())

public:
    explicit Socket(const QString &serverPath, QObject* parent = nullptr);
    explicit Socket(int fd, QObject* parent = nullptr);
    virtual ~Socket();

public Q_SLOTS:
    int write(QByteArray data, int fd = -1);

protected:
    virtual void initImpl() override final;

Q_SIGNALS:
    void readyRead(const QByteArray &payload, int fd);
    void disconnected();

private:
    class Private;
    Private * const d;
};
}

#endif // HYPERSPACE_SOCKET_H
