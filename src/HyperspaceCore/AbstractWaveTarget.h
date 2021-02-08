/*
 *
 */

#ifndef HYPERSPACE_ABSTRACTWAVETARGET_H
#define HYPERSPACE_ABSTRACTWAVETARGET_H

#include <HyperspaceCore/Global>

#include <HyperspaceCore/Fluctuation>
#include <HyperspaceCore/Rebound>

/**
 * @defgroup HyperspaceCore Hyperspace Core
 *
 * Hyperspace Core contains all the basic types and mechanism to enable Hyperspace usage, both as a server and as a client,
 * for Hemera applications.
 *
 * It is contained in the Hyperspace:: namespace.
 */

namespace Hyperspace {

class Gate;

class AbstractWaveTargetPrivate;
/**
 * @class AbstractWaveTarget
 * @ingroup HyperspaceCore
 * @headerfile HyperspaceCore/AbstractWaveTarget.h <HyperspaceCore/AbstractWaveTarget>
 *
 * @brief The base class for Applications to act as Wave targets.
 *
 * AbstractWaveTarget is the most basic method for applications to expose resources over Hyperspace. It is rarely
 * used: most of the times you'll want one of the higher level alternatives contained in other Hyperspace modules
 * instead.
 *
 * Applications must reimplement AbstractWaveTarget to process any incoming Wave. For each Wave method, a pure
 * virtual function is given, and it shall return at any time a Rebound via sendRebound.
 *
 * A Wave Target, to be exported, must be registered into a Gate. Each Wave Target has its own path, which is
 * always relative to the base target path of its Gate.
 *
 * @par Asynchronicity
 * Wave targets are designed to be asynchronous, so the reply can be delayed while the application processes the
 * request. For this reason, no return value is expected from any Wave method, but sendRebound should be called
 * instead.
 *
 * @sa Hyperspace::Rebound
 * @sa Hyperspace::Gate
 */
class HYPERSPACE_QT5_EXPORT AbstractWaveTarget : public QObject
{
    Q_OBJECT

    Q_DECLARE_PRIVATE_D(d_w_ptr, AbstractWaveTarget)
    Q_DISABLE_COPY(AbstractWaveTarget)

public:
    AbstractWaveTarget(const QByteArray &interface, Gate *assignedGate, QObject *parent = nullptr);
    AbstractWaveTarget(const QByteArray &interface, QObject *parent = nullptr);
    virtual ~AbstractWaveTarget();

    QByteArray interface() const;

    bool isReady() const;

Q_SIGNALS:
    void ready();

protected Q_SLOTS:
    /**
     * @brief Send a rebound for a received wave
     *
     * Call this method upon processing a wave. The rebound must have the same ID of its corresponding Wave.
     */
    void sendRebound(const Hyperspace::Rebound &rebound);

    /**
     * @brief Send a fluctuation for this target
     *
     * Call this method whenever the internal representation (e.g.: GET) of the target changes.
     */
    void sendFluctuation(const QByteArray &targetPath, const Fluctuation &payload);


protected:
    AbstractWaveTargetPrivate * const d_w_ptr;

    virtual void waveFunction(const Wave &wave) = 0;

private:
    friend class Gate;
    friend class GatePrivate;
};

}

#endif // HYPERSPACE_ABSTRACTWAVE_H
