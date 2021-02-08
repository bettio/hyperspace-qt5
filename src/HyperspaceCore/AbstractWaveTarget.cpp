/*
 *
 */

#include "AbstractWaveTarget_p.h"

#include <QtCore/QDebug>
#include <QtCore/QSharedData>

#include <HyperspaceCore/Fluctuation>
#include <HyperspaceCore/Gate>
#include <HyperspaceCore/Rebound>
#include <HyperspaceCore/Wave>

namespace Hyperspace
{

void AbstractWaveTargetPrivate::assignedToGateHook()
{
    // Nothing, just here for the base implementation
}

AbstractWaveTarget::AbstractWaveTarget(const QByteArray &interface, Gate *assignedGate, QObject *parent)
    : QObject(parent)
    , d_w_ptr(new AbstractWaveTargetPrivate)
{
    Q_D(AbstractWaveTarget);
    d->interface = interface;

    if (assignedGate) {
        d->gate = assignedGate;
        if (assignedGate->isReady()) {
            assignedGate->assignWaveTarget(this);
            emit ready();

        } else {
            connect(assignedGate, &Hemera::AsyncInitObject::ready, this, [this] {
                Q_D(AbstractWaveTarget);
                if (d->gate) {
                    d->gate->assignWaveTarget(this);
                    emit ready();
                }
            });
        }
    }
}

AbstractWaveTarget::AbstractWaveTarget(const QByteArray &interface, QObject *parent)
    : AbstractWaveTarget(interface, Gate::defaultGate(), parent)
{
}

AbstractWaveTarget::~AbstractWaveTarget()
{
    Q_D(AbstractWaveTarget);
    if (d->gate && d->gate->isReady()) {
        d->gate->unassignWaveTarget(d->interface);
    }
    delete d_w_ptr;
}

QByteArray AbstractWaveTarget::interface() const
{
    Q_D(const AbstractWaveTarget);

    return d->interface;

}

bool AbstractWaveTarget::isReady() const
{
    Q_D(const AbstractWaveTarget);

    return d->gate && d->gate->isReady();
}

void AbstractWaveTarget::sendRebound(const Rebound &rebound)
{
    Q_D(AbstractWaveTarget);
    if (isReady()) {
        d->gate->sendRebound(rebound);
    } else {
        qWarning() << "Discarded rebound: Gate was not set or ready yet.";
    }
}

void AbstractWaveTarget::sendFluctuation(const QByteArray &targetPath, const Fluctuation &payload)
{
    Q_D(AbstractWaveTarget);
    // fluctuations are discarded if the wave target is not bound to any gate
    if (d->gate) {
        if (isReady()) {
            d->gate->sendFluctuation(d->interface, targetPath, payload);
        } else {
            // gate is not yet ready, we'll defer them until it is not ready
            connect(d->gate, &Hemera::AsyncInitObject::ready, this, [this, targetPath, payload] {
                Q_D(AbstractWaveTarget);
                d->gate->sendFluctuation(d->interface, targetPath, payload);
            });
        }
    } else {
        // discarded fluctuations are just bad
        qWarning() << "hypespace warning: discarded fluctuation: " << targetPath << " payload: " << payload.payload();
    }
}

}

#include "moc_AbstractWaveTarget.cpp"
