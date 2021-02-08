/*
 *
 */

#include "AbstractWaveTarget_p.h"
#include "BSONDocument.h"
#include "BSONStreamReader.h"
#include "Socket.h"
#include "Waveguide.h"

#include <HemeraCore/Operation>
#include <HemeraCore/Literals>

#include <QtCore/QDebug>
#include <QtCore/QLoggingCategory>
#include <QtCore/QTimer>

#include <functional>

Q_LOGGING_CATEGORY(hyperspaceGateDC, "hyperspace.gate", DEBUG_MESSAGES_DEFAULT_LEVEL)

namespace Hyperspace {

class Gate::Private
{
    public:
        Private(Gate *gate) : q(gate) {}

        Gate *q;

        QList <QByteArray> interfaces;
        QHash <QByteArray, AbstractWaveTarget *> registeredTargets;

        Socket *socket;
        Util::BSONStreamReader bsonStream;

        static Gate *defaultGate;

        void sendInterfaces();
};

Gate *Gate::Private::defaultGate;

void Gate::sendRebound(const Rebound &rebound)
{
    qCDebug(hyperspaceGateDC) << "Sending rebound" << rebound.id() << (quint16)rebound.response();

    d->socket->write(rebound.serialize());
}

void Gate::sendFluctuation(const QByteArray &interface, const QByteArray &targetPath, const Fluctuation &f)
{
    Fluctuation fluctuation = f;

    fluctuation.setInterface(interface);
    fluctuation.setTarget(targetPath);

    d->socket->write(fluctuation.serialize());
}

void Gate::sendWaveguide(const QByteArray &interface, const Waveguide &w)
{
    qCWarning(hyperspaceGateDC) << "Sending waveguide" << interface;
    Waveguide waveguide = w;

    waveguide.setInterface(interface);

    d->socket->write(waveguide.serialize());
}

void Gate::waveFunction(const Wave &wave)
{
    AbstractWaveTarget *target = d->registeredTargets.value(wave.interface());
    if (target) {
        target->waveFunction(wave);
    } else {
        sendRebound(Rebound(wave.id(), ResponseCode::NotFound));
    }
}

void Gate::initImpl()
{
    // Connect socket
#ifdef ENABLE_TEST_CODEPATHS
    if (Q_UNLIKELY(qgetenv("RUNNING_AUTOTESTS").toInt() == 1)) {
        qCInfo(hyperspaceGateDC) << "Connecting to gate for autotests";
        d->socket = new Socket(QStringLiteral("/tmp/hyperdrive-gates-autotests"), this);
    } else {
#endif
        d->socket = new Socket(QStringLiteral("/run/hyperdrive/gates"), this);
#ifdef ENABLE_TEST_CODEPATHS
    }
#endif

    // Handle the function pointer overload...
//     void (QLocalSocket::*errorSignal)(QLocalSocket::LocalSocketError) = &QLocalSocket::error;
//     connect(d->socket, errorSignal, [this] (QLocalSocket::LocalSocketError socketError) {
//         qCWarning(hyperspaceGateDC) << "Internal socket error: " << socketError;
//
//         if (!isReady()) {
//             setInitError(Hemera::Literals::literal(Hemera::Literals::Errors::failedRequest()),
//                          QStringLiteral("Could not connect to Hyperdrive."));
//         }
//     });


    QObject::connect(d->socket, &Socket::readyRead, this, [this] (QByteArray data, int fd) {
        d->bsonStream.enqueueData(data);
        while (d->bsonStream.canReadDocument()) {
            Wave wave = Wave::fromBinary(d->bsonStream.dequeueDocumentData());

            qCDebug(hyperspaceGateDC) << "Got a wave with id" << wave.id();
            waveFunction(wave);
        }
    });

    // Monitor socket
    QObject::connect(d->socket->init(), &Hemera::Operation::finished, this, [this] (Hemera::Operation *op) {
        if (op->isError()) {
            setInitError(QLatin1String(Hemera::Literals::Errors::failedRequest()),
                            QString::fromLatin1("Could not bring up socket: %1: %2").arg(op->errorName(), op->errorMessage()));
            return;
        }

        d->sendInterfaces();

        setReady();
    });
}

void Gate::Private::sendInterfaces()
{
    qDebug() << "Gate: send interfaces: " << interfaces;

    for (QByteArray interface : interfaces) {
        q->sendWaveguide(interface, Waveguide());
    }
}

Gate::Gate(QObject *parent)
    : AsyncInitObject(parent)
    , d(new Private(this))
{
}

Gate::~Gate()
{
    for (AbstractWaveTarget *target : d->registeredTargets) {
        target->d_func()->gate = nullptr;
    }

    delete d;
}

QList<QByteArray> Gate::interfaces() const
{
    return d->interfaces;
}

void Gate::assignWaveTarget(AbstractWaveTarget *target)
{
    if (target->d_func()->gate != this) {
        qCWarning(hyperspaceGateDC) << "Attempting to assign a resource to a Gate twice!";
        return;
    }

    target->d_func()->gate = this;
    d->registeredTargets.insert(target->interface(), target);
    d->interfaces.append(target->interface());

    sendWaveguide(target->interface(), Waveguide());
}

AbstractWaveTarget* Gate::unassignWaveTarget(const QByteArray& path)
{
    return d->registeredTargets.take(path);
}

Gate *Gate::defaultGate()
{
    if (!Private::defaultGate) {
        Private::defaultGate = new Gate();
        Private::defaultGate->init();
    }

    return Private::defaultGate;
}

}

#include "moc_Gate.cpp"
