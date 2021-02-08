/*
 *
 */

#include "hyperspaceglobaldiscoverywatcher_p.h"

#include "hyperspacecapabilitywatcher_p.h"

#include <HyperspaceCore/Global>

#include <QtCore/QDataStream>
#include <QtCore/QGlobalStatic>
#include <QtCore/QTimer>
#include <QtCore/QDebug>
#include <QtCore/QLoggingCategory>

#include <QtNetwork/QLocalSocket>

Q_LOGGING_CATEGORY(hyperspaceGlobalDiscoveryWatcherDC, "hyperspace.client.globaldiscoverywatcher", DEBUG_MESSAGES_DEFAULT_LEVEL)

namespace Hyperspace {

Q_GLOBAL_STATIC(GlobalDiscoveryWatcher, globalDiscoveryWatcher)

GlobalDiscoveryWatcher::GlobalDiscoveryWatcher()
    : QObject(Q_NULLPTR)
    , m_socket(new QLocalSocket(this))
{
    // Init here, we can't really be async due to the fact we're a singleton.

    // Monitor socket
    connect(m_socket, &QLocalSocket::stateChanged, [this] (QLocalSocket::LocalSocketState socketState) {
        if (socketState == QLocalSocket::ConnectedState) {
            // We are ready! Send stuff stuck in the queue now.
            // To do that, we need the event loop to settle down before we can write to the socket.
            QTimer::singleShot(0, this, SLOT(processQueue()));
        }
    });

    // Handle the function pointer overload...
    void (QLocalSocket::*errorSignal)(QLocalSocket::LocalSocketError) = &QLocalSocket::error;
    connect(m_socket, errorSignal, [this] (QLocalSocket::LocalSocketError socketError) {
        qCWarning(hyperspaceGlobalDiscoveryWatcherDC) << "Internal socket error: " << socketError;
    });

    connect(m_socket, &QIODevice::readyRead, [this] {
        QByteArray data;
        data.reserve(m_socket->bytesAvailable());
        while (m_socket->bytesAvailable() > 0) {
            data.append(m_socket->read(m_socket->bytesAvailable()));
        }

        typedef void (Hyperspace::CapabilityWatcher::Private::* ProcessCapabilityMethod)(const QByteArray &capability);

        auto processCapabilities = [this] (ProcessCapabilityMethod m, CapabilityWatcher::WatchedEvent event, const QList<QByteArray> &capabilities) {
            for (const QByteArray &capability : capabilities) {
                QMultiHash< QByteArray, CapabilityWatcher* >::const_iterator i = m_watchedCapabilities.constFind(capability);
                while (i != m_watchedCapabilities.constEnd() && i.key() == capability) {
                    if (i.value()->watchedEvents() & event) {
                        (i.value()->d->*m)(capability);
                    }
                    ++i;
                }
            }
        };

        QDataStream in(&data, QIODevice::ReadOnly);
        // Sockets may buffer several requests in case they're concurrent: unroll the whole DataStream.
        while (!in.atEnd()) {
            quint8 command;
            in >> command;

            if (command == Protocol::Discovery::announced()) {
                QList< QByteArray > capabilities;
                in >> capabilities;
                processCapabilities(&Hyperspace::CapabilityWatcher::Private::onCapabilityAnnounced,
                                    CapabilityWatcher::WatchedEvent::Announced, capabilities);
            } else if (command == Protocol::Discovery::expired()) {
                QList< QByteArray > capabilities;
                in >> capabilities;
                processCapabilities(&Hyperspace::CapabilityWatcher::Private::onCapabilityExpired,
                                    CapabilityWatcher::WatchedEvent::Expired, capabilities);
            } else if (command == Protocol::Discovery::purged()) {
                QList< QByteArray > capabilities;
                in >> capabilities;
                processCapabilities(&Hyperspace::CapabilityWatcher::Private::onCapabilityPurged,
                                    CapabilityWatcher::WatchedEvent::Purged, capabilities);
            } else if (command == Protocol::Discovery::changed()) {
                QList< QByteArray > capabilities;
                in >> capabilities;
                processCapabilities(&Hyperspace::CapabilityWatcher::Private::onCapabilityIntrospectionChanged,
                                    CapabilityWatcher::WatchedEvent::Introspection, capabilities);

                // Anybody listening to introspection data?
                for (const QByteArray &capability : capabilities) {
                    if (m_introspectionDataWatchers.contains(capability)) {
                        // Trigger introspect!
                        QByteArray msg;
                        QDataStream out(&msg, QIODevice::WriteOnly);

                        out << Hyperspace::Protocol::Discovery::introspectRequest() << capability;

                        m_socket->write(msg);
                    }
                }
            } else if (command == Protocol::Discovery::introspectReply()) {
                QByteArray capability;
                QList< QByteArray > servicesUrl;
                in >> capability >> servicesUrl;

                // Update our watchers
                QMultiHash< QByteArray, CapabilityWatcher* >::const_iterator i = m_watchedCapabilities.constFind(capability);
                while (i != m_watchedCapabilities.constEnd() && i.key() == capability) {
                    Q_EMIT i.value()->capabilityIntrospectionDataChanged(capability, servicesUrl);
                }
            } else {
                qCWarning(hyperspaceGlobalDiscoveryWatcherDC) << "Message malformed!" << data.size() << data.toHex();
                return;
            }
        }
    });

#ifdef ENABLE_TEST_CODEPATHS
    if (Q_UNLIKELY(qgetenv("RUNNING_AUTOTESTS").toInt() == 1)) {
        qCDebug(hyperspaceGlobalDiscoveryWatcherDC) << "Connecting to gate for autotests";
        m_socket->connectToServer(QStringLiteral("/tmp/hyperdrive/discovery/clients-autotests"));
    } else {
#endif
        m_socket->connectToServer(QStringLiteral("/run/hyperdrive/discovery/clients"));
#ifdef ENABLE_TEST_CODEPATHS
    }
#endif
}

GlobalDiscoveryWatcher::~GlobalDiscoveryWatcher()
{
}

GlobalDiscoveryWatcher *GlobalDiscoveryWatcher::instance()
{
    return globalDiscoveryWatcher;
}

void GlobalDiscoveryWatcher::processQueue()
{
    while (!m_requests.isEmpty()) {
        m_socket->write(m_requests.dequeue());
    }
}

void GlobalDiscoveryWatcher::callOrEnqueueRequest(const QByteArray &data)
{
    if (m_socket->state() == QLocalSocket::ConnectedState) {
        m_socket->write(data);
    } else {
        m_requests.enqueue(data);
    }
}

void GlobalDiscoveryWatcher::registerCapabilities(CapabilityWatcher *watcher, const QList<QByteArray> &capabilities)
{
    // Subscribe
    QByteArray msg;
    QDataStream out(&msg, QIODevice::WriteOnly);

    out << Protocol::Discovery::subscribe() << capabilities;

    m_socket->write(msg);

    // Add to hash
    for (const QByteArray &capability : capabilities) {
        m_watchedCapabilities.insert(capability, watcher);
        if (watcher->watchedEvents() & CapabilityWatcher::WatchedEvent::IntrospectionData) {
            m_introspectionDataWatchers.insert(capability, watcher);
        }
    }
}

void GlobalDiscoveryWatcher::unregisterCapabilities(CapabilityWatcher *watcher, const QList<QByteArray> &capabilities)
{
    // Unsubscribe
    // Subscribe
    QByteArray msg;
    QDataStream out(&msg, QIODevice::WriteOnly);

    out << Protocol::Discovery::unsubscribe() << capabilities;

    m_socket->write(msg);

    // Remove from hash
    for (const QByteArray &capability : capabilities) {
        m_watchedCapabilities.remove(capability, watcher);
        if (watcher->watchedEvents() & CapabilityWatcher::WatchedEvent::IntrospectionData) {
            m_introspectionDataWatchers.remove(capability, watcher);
        }
    }
}

}
