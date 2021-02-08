#ifndef HYPERSPACE_ABSTRACTWAVETARGET_P_H
#define HYPERSPACE_ABSTRACTWAVETARGET_P_H

#include <HyperspaceCore/AbstractWaveTarget>
#include <HyperspaceCore/Gate>

namespace Hyperspace {

typedef void (Hyperspace::AbstractWaveTarget::* WaveFunction)(quint64 waveId,
                                                              const ByteArrayHash &attributes, const QByteArray &payload);

class AbstractWaveTargetPrivate
{
public:
    AbstractWaveTargetPrivate() : gate(Q_NULLPTR) {}
    virtual ~AbstractWaveTargetPrivate() {}

    Gate *gate;

    // NOTE: Maybe we should make this public? I don't think so though. It's potentially dangerous and ugly, and I can't
    //       see the real world usage for this for the final developer.
    /**
     * @brief Stub for implementing a hook chain
     *
     * When a AbstractWaveTarget is assigned to a Gate, one of its implementation might decide to do something.
     * In this case, it can reimplement this hook, which will be called whenever the assignment takes place.
     *
     * @note Never attempt to call this hook manually! It is only meant to be reimplemented and called by a third
     *       party. Undefined behavior and a high likelihood of crashes lie ahead if you attempt to do so.
     */
    virtual void assignedToGateHook();

    QByteArray interface;
};

}

#endif
