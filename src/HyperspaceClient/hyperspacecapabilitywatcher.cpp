/*
 *
 */

#include "hyperspacecapabilitywatcher_p.h"

#include "hyperspaceglobaldiscoverywatcher_p.h"

#include <QtCore/QSet>

namespace Hyperspace {

void CapabilityWatcher::Private::onCapabilityAnnounced(const QByteArray &capability)
{
    Q_EMIT q->capabilityStatusChanged(capability, Status::Announced);
}

void CapabilityWatcher::Private::onCapabilityExpired(const QByteArray &capability)
{
    Q_EMIT q->capabilityStatusChanged(capability, Status::Expired);
}

void CapabilityWatcher::Private::onCapabilityPurged(const QByteArray &capability)
{
    Q_EMIT q->capabilityStatusChanged(capability, Status::Purged);
}

void CapabilityWatcher::Private::onCapabilityIntrospectionChanged(const QByteArray &capability)
{
    Q_EMIT q->capabilityIntrospectionChanged(capability);
}

CapabilityWatcher::CapabilityWatcher(const QByteArray &capability, WatchedEvents watchMode, QObject* parent)
    : AsyncInitObject(parent)
    , d(new Private(this))
{
    QList<QByteArray> oneCapList;
    oneCapList.append(capability);
    d->capabilities = oneCapList;
    d->watchMode = watchMode;
}

CapabilityWatcher::CapabilityWatcher(const QList< QByteArray > &capabilities, WatchedEvents watchMode, QObject* parent)
    : AsyncInitObject(parent)
    , d(new Private(this))
{
    d->capabilities = capabilities;
    d->watchMode = watchMode;
}

CapabilityWatcher::~CapabilityWatcher()
{
    GlobalDiscoveryWatcher::instance()->unregisterCapabilities(this, d->capabilities);

    delete d;
}

void CapabilityWatcher::initImpl()
{
    GlobalDiscoveryWatcher::instance()->registerCapabilities(this, d->capabilities);
    setReady();
}

CapabilityWatcher::WatchedEvents CapabilityWatcher::watchedEvents() const
{
    return d->watchMode;
}

void CapabilityWatcher::addCapabilities(const QList< QByteArray > &capabilities)
{
    // Remove duplicates first
    QList< QByteArray > caps;
    QList< QByteArray >::iterator i = caps.begin();
    while (i != caps.end()) {
        if (d->capabilities.contains(*i)) {
            i = caps.erase(i);
        } else {
            ++i;
        }
    }

    if (caps.isEmpty()) {
        return;
    }

    GlobalDiscoveryWatcher::instance()->registerCapabilities(this, caps);
    d->capabilities.append(caps);
}

void CapabilityWatcher::removeCapabilities(const QList< QByteArray > &capabilities)
{
    // Remove not contained first
    QSet< QByteArray > caps = capabilities.toSet();
    caps = caps.intersect(d->capabilities.toSet());

    for (const QByteArray &capability : caps) {
        d->capabilities.removeOne(capability);
    }

    GlobalDiscoveryWatcher::instance()->unregisterCapabilities(this, caps.toList());
}

}
