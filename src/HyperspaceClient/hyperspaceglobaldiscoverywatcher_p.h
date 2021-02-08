/*
 *
 */

#ifndef HYPERSPACE_GLOBALDISCOVERYWATCHER_H
#define HYPERSPACE_GLOBALDISCOVERYWATCHER_H

#include <QtCore/QObject>
#include <QtCore/QQueue>

#include "hyperspacecapabilitywatcher.h"

class QLocalSocket;
namespace Hyperspace {

class CapabilityWatcher;

class GlobalDiscoveryWatcher : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(GlobalDiscoveryWatcher)

public:
    GlobalDiscoveryWatcher();
    virtual ~GlobalDiscoveryWatcher();

    static GlobalDiscoveryWatcher *instance();

    void registerCapabilities(CapabilityWatcher *watcher, const QList<QByteArray> &capabilities);
    void unregisterCapabilities(CapabilityWatcher *watcher, const QList<QByteArray> &capabilities);

private Q_SLOTS:
    void processQueue();

private:
    QLocalSocket *m_socket;
    QMultiHash< QByteArray, CapabilityWatcher* > m_watchedCapabilities;
    QHash< QByteArray, CapabilityWatcher::Status > m_cachedCapabilityStatus;
    QQueue< QByteArray > m_requests;
    QMultiHash< QByteArray, CapabilityWatcher* > m_introspectionDataWatchers;

    void callOrEnqueueRequest(const QByteArray &data);
};

}

#endif // HYPERSPACE_GLOBALDISCOVERYWATCHER_H
