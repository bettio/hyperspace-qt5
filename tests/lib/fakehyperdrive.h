/*
 *
 */

#ifndef FAKEHYPERDRIVE_H
#define FAKEHYPERDRIVE_H

#include <QtCore/QObject>

#include <HemeraCore/AsyncInitObject>

#include <HyperspaceCore/Global>
#include <HyperspaceCore/Rebound>
#include <HyperspaceCore/Wave>
#include <HyperspaceCore/Waveguide>

class FakeHyperdrive : public Hemera::AsyncInitObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FakeHyperdrive)

public:
    FakeHyperdrive(QObject *parent);
    virtual ~FakeHyperdrive();

    virtual void initImpl();

    Hyperspace::Rebound lastRebound() const;

public Q_SLOTS:
    void sendWave(const Hyperspace::Wave &wave);

Q_SIGNALS:
    void gotRebound();

private:
    class Private;
    Private * const d;
};

Q_DECLARE_METATYPE(Hyperspace::Wave)

#endif // FAKEHYPERDRIVE_H
