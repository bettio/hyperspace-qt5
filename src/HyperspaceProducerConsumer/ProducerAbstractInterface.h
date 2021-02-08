#ifndef _HYPERSPACE_PLUS_PROVIDERABSTRACTINTERFACE_H_
#define _HYPERSPACE_PLUS_PROVIDERABSTRACTINTERFACE_H_

#include <HyperspaceCore/AbstractWaveTarget>

#include <QtCore/QDateTime>

namespace Hyperspace
{

enum class Retention {
    Unknown = 0,
    Discard = 1,
    Volatile = 2,
    Stored = 3
};

enum class Reliability {
    Unknown = 0,
    Unreliable = 1,
    Guaranteed = 2,
    Unique = 3
};

namespace ProducerConsumer
{

class ProducerAbstractInterface : public AbstractWaveTarget
{
    Q_OBJECT

    public:
        enum DispatchResult {
            Success,
            CouldNotConvertPayload,
            IndexNotFound
        };

        ProducerAbstractInterface(const QByteArray &interface, QObject *parent);
        virtual ~ProducerAbstractInterface();

    protected:
        virtual void waveFunction(const Wave &wave) override;

        void insertTransition(int state, QByteArray token, int newState);
        void insertDispatchState(int state, int dispatchIndex);

        int dispatchIndex(const QList<QByteArray> &inputTokens);

        virtual void populateTokensAndStates() = 0;
        virtual Hyperspace::ProducerConsumer::ProducerAbstractInterface::DispatchResult dispatch(int i, const QByteArray &value, const QList<QByteArray> &inputTokens) = 0;

        void sendRawDataOnEndpoint(const QByteArray &value, const QByteArray &target, const QHash<QByteArray, QByteArray> &attributes = QHash<QByteArray, QByteArray>());

        void sendDataOnEndpoint(const QByteArray &value, const QByteArray &target, const QHash<QByteArray, QByteArray> &attributes = QHash<QByteArray, QByteArray>());
        void sendDataOnEndpoint(double value, const QByteArray &target, const QHash<QByteArray, QByteArray> &attributes = QHash<QByteArray, QByteArray>());
        void sendDataOnEndpoint(int value, const QByteArray &target, const QHash<QByteArray, QByteArray> &attributes = QHash<QByteArray, QByteArray>());
        void sendDataOnEndpoint(qint64 value, const QByteArray &target, const QHash<QByteArray, QByteArray> &attributes = QHash<QByteArray, QByteArray>());
        void sendDataOnEndpoint(bool value, const QByteArray &target, const QHash<QByteArray, QByteArray> &attributes = QHash<QByteArray, QByteArray>());
        void sendDataOnEndpoint(const QString &value, const QByteArray &target, const QHash<QByteArray, QByteArray> &attributes = QHash<QByteArray, QByteArray>());
        void sendDataOnEndpoint(const QDateTime &value, const QByteArray &target, const QHash<QByteArray, QByteArray> &attributes = QHash<QByteArray, QByteArray>());

        bool payloadToValue(const QByteArray &payload, QByteArray *value);
        bool payloadToValue(const QByteArray &payload, int *value);
        bool payloadToValue(const QByteArray &payload, qint64 *value);
        bool payloadToValue(const QByteArray &payload, bool *value);
        bool payloadToValue(const QByteArray &payload, double *value);
        bool payloadToValue(const QByteArray &payload, QString *value);
        bool payloadToValue(const QByteArray &payload, QDateTime *value);

    private:
        class Private;
        Private *const d;
};

}

}

#endif
