#include "Waveguide.h"

#include "BSONDocument.h"
#include "BSONSerializer.h"

#include <QtCore/QDebug>
#include <QSharedData>

namespace Hyperspace {

class WaveguideData : public QSharedData
{
public:
    WaveguideData() { }
    WaveguideData(const WaveguideData &other)
        : QSharedData(other), interface(other.interface) { }
    ~WaveguideData() { }

    QByteArray interface;
};

Waveguide::Waveguide()
    : d(new WaveguideData())
{
}

Waveguide::Waveguide(const Waveguide& other)
    : d(other.d)
{
}

Waveguide::~Waveguide()
{

}

Waveguide& Waveguide::operator=(const Waveguide& rhs)
{
    if (this==&rhs) {
        // Protect against self-assignment
        return *this;
    }

    d = rhs.d;
    return *this;
}

bool Waveguide::operator==(const Waveguide& other) const
{
    return d->interface == other.interface();
}

QByteArray Waveguide::interface() const
{
    return d->interface;
}

void Waveguide::setInterface(const QByteArray& i)
{
    d->interface = i;
}

QByteArray Waveguide::serialize() const
{
    Util::BSONSerializer s;
    s.appendInt32Value("y", (int32_t) Protocol::MessageType::Waveguide);
    s.appendASCIIString("i", d->interface);
    s.appendEndOfDocument();

    return s.document();
}

Waveguide Waveguide::fromBinary(const QByteArray &data)
{
    Util::BSONDocument doc(data);
    if (Q_UNLIKELY(!doc.isValid())) {
        qWarning() << "Waveguide BSON document is not valid!";
        return Waveguide();
    }
    if (Q_UNLIKELY(doc.int32Value("y") != (int32_t) Protocol::MessageType::Waveguide)) {
        qWarning() << "Received message is not a Waveguide";
        return Waveguide();
    }

    Waveguide w;
    w.setInterface(doc.byteArrayValue("i"));

    return w;
}

}
