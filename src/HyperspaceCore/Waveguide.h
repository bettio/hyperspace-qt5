#ifndef HYPERSPACE_WAVEGUIDE_H
#define HYPERSPACE_WAVEGUIDE_H

#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QSharedDataPointer>

#include <HyperspaceCore/Global>

namespace Hyperspace {

class WaveguideData;

class HYPERSPACE_QT5_EXPORT Waveguide {
public:
    /**
     * @brief Constructs a Waveguide
     */
    Waveguide();
    Waveguide(const Waveguide &other);
    ~Waveguide();

    Waveguide &operator=(const Waveguide &rhs);
    bool operator==(const Waveguide &other) const;
    inline bool operator!=(const Waveguide &other) const { return !operator==(other); }

    QByteArray interface() const;
    void setInterface(const QByteArray &i);

    QByteArray serialize() const;
    static Waveguide fromBinary(const QByteArray &data);

private:
    QSharedDataPointer<WaveguideData> d;
};

}

#endif // HYPERSPACE_WAVEGUIDE_H
