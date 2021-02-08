/*
 *
 */

#ifndef HYPERSPACE_GATE_H
#define HYPERSPACE_GATE_H

#include <HemeraCore/AsyncInitObject>

#include <HyperspaceCore/AbstractWaveTarget>

#include <HyperspaceCore/Waveguide>

namespace Hyperspace {

class AbstractWaveTarget;

/**
 * @class Gate
 * @ingroup HyperspaceCore
 * @headerfile HyperspaceCore/Gate.h <HyperspaceCore/Gate>
 *
 * @brief The base class for Applications to expose Gates.
 *
 * Gate is the most basic method for applications to create Gates over Hyperspace. It is rarely
 * used: most of the times you'll want one of the higher level alternatives contained in other Hyperspace modules
 * instead.
 *
 * This implementation of Gate does not do anything to register Wave targets onto Gates. If you wish to use a Gate
 * with different semantics, you should reimplement Gate and define your own semantics for registering
 * Wave Targets. This is done due to the fact that different Gates have very different semantics for registering
 * Wave Targets.
 *
 * @sa Hyperspace::AbstractWaveTarget
 */
class Gate : public Hemera::AsyncInitObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Gate)

public:
    virtual ~Gate();

    /// @returns The interfaces this Gate exposes
    QList<QByteArray> interfaces() const;

    static Gate *defaultGate();

protected:
    explicit Gate(QObject *parent = nullptr);

    virtual void initImpl() override;

    virtual void waveFunction(const Wave &wave);

    virtual void sendRebound(const Rebound &rebound);
    virtual void sendFluctuation(const QByteArray &interface, const QByteArray &targetPath, const Fluctuation &payload);
    virtual void sendWaveguide(const QByteArray &interface, const Waveguide &w);

    /**
     * @brief Assigns an AbstractWaveTarget to this gate.
     *
     * @note If the Wave Target already has an associated Gate, this procedure will attempt to overwrite the assignment.
     *       This, though, might lead to an undefined behavior. Never try assigning a Wave Target to more than
     *       one gate!
     *
     * @note Once a Wave Target is assigned to a Gate, the Gate effectively becomes its parent. This means the Gate,
     *       upon destruction, will delete all of its assigned targets. Never delete an AbstractWaveTarget manually once
     *       it has been assigned or, worse, attempt deleting it from within the Target itself!
     *
     * @p target The Wave Target to assign to this gate.
     * @p path The path assigned to the target
     */
    void assignWaveTarget(AbstractWaveTarget *target);
    /**
     * @brief Removes an assignment from this gate.
     *
     * @note Changing a Wave Target's gate is unsupported. This function should be called only if your Wave Target is released
     *       before the Gate is.
     *
     * @note When calling this method, the garbage collection Gate performs will not take place. You take back the ownership
     *       of the returned object and you are responsible for its deletion.
     *
     * @p path The path to unassign.
     *
     * @returns A pointer to the now unassigned @ref AbstractWaveTarget, or null if no target was assigned to the given path.
     */
    AbstractWaveTarget *unassignWaveTarget(const QByteArray &path);

private:
    friend class AbstractWaveTarget;
    friend class SocketGateEmulation;

    class Private;
    Private *const d;
};

}

#endif // HYPERSPACE_GATE_H
