/*
 *
 */

#include "fakehyperdrive.h"

#include <HyperspaceCore/BSONStreamReader>
#include <HyperspaceCore/BSONDocument>

#include <QtCore/QDebug>
#include <QtCore/QFile>

#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

#include <QtTest/qbenchmark.h>

class FakeHyperdrive::Private
{
public:
    Private() : lastRebound(0) {}

    QLocalServer *hyperServer;
    QLocalSocket *socket;
    Hyperspace::Rebound lastRebound;
    Hyperspace::Util::BSONStreamReader bsonStream;
};

FakeHyperdrive::FakeHyperdrive(QObject* parent)
    : Hemera::AsyncInitObject(parent)
    , d(new Private)
{
    qRegisterMetaType<Hyperspace::Wave>();
}

FakeHyperdrive::~FakeHyperdrive()
{
}

void FakeHyperdrive::initImpl()
{
    if (QFile::exists(QStringLiteral("/tmp/hyperdrive-gates-autotests"))) {
        QFile::remove(QStringLiteral("/tmp/hyperdrive-gates-autotests"));
    }

    d->hyperServer = new QLocalServer(this);
    if (!d->hyperServer->listen(QStringLiteral("/tmp/hyperdrive-gates-autotests"))) {
        setInitError(QStringLiteral("register"), QStringLiteral("Could not set up QLocalServer"));
    }

    connect(d->hyperServer, &QLocalServer::newConnection, [this] () {
        qDebug() << "Fake hyperdrive got a connection";
        d->socket = d->hyperServer->nextPendingConnection();

        connect(d->socket, &QLocalSocket::readyRead, [this] {
            QByteArray data = d->socket->read(d->socket->bytesAvailable());
            d->bsonStream.enqueueData(data);
            while (d->bsonStream.canReadDocument()) {
                QByteArray docData = d->bsonStream.dequeueDocumentData();
                Hyperspace::Util::BSONDocument doc(docData);

                if (doc.int32Value("y") == (int32_t) Hyperspace::Protocol::MessageType::Waveguide) {
                    qDebug() << "Interfaces registered successfully." << Hyperspace::Waveguide::fromBinary(docData).interface();
                } else if (doc.int32Value("y")  == (int32_t) Hyperspace::Protocol::MessageType::Rebound) {
                    d->lastRebound = Hyperspace::Rebound::fromBinary(docData);
                    Q_EMIT gotRebound();
                } else {
                    qWarning() << "Message malformed on the hyperdrive!";
                    return;
                }
            }
        });
    });

    setReady();
}

Hyperspace::Rebound FakeHyperdrive::lastRebound() const
{
    return d->lastRebound;
}

void FakeHyperdrive::sendWave(const Hyperspace::Wave &wave)
{
    QBENCHMARK_ONCE {
        d->socket->write(wave.serialize());
    }
}
