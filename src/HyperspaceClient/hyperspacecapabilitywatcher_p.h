#ifndef HYPERSPACE_CAPABILITYWATCHER_P_H
#define HYPERSPACE_CAPABILITYWATCHER_P_H

#include "hyperspacecapabilitywatcher.h"

namespace Hyperspace {

class CapabilityWatcher::Private
{
public:
    Private(CapabilityWatcher *q) : q(q) {}

    void onCapabilityAnnounced(const QByteArray &capability);
    void onCapabilityExpired(const QByteArray &capability);
    void onCapabilityPurged(const QByteArray &capability);

    void onCapabilityIntrospectionChanged(const QByteArray &capability);

    CapabilityWatcher *q;

    QList<QByteArray> capabilities;
    WatchedEvents watchMode;
};

}

#endif // HYPERSPACE_CAPABILITYWATCHER_H
