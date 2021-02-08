#ifndef HYPERSPACE2CPP_H
#define HYPERSPACE2CPP_H

#include <HemeraGenerators/BaseGenerator>

#include <QtCore/QJsonObject>

class Hyperspace2Cpp : public Hemera::Generators::BaseGenerator
{
    Q_OBJECT

public:
    enum InterfaceType {
        PropertiesType,
        DataStreamType
    };

    explicit Hyperspace2Cpp(const QString &sourceFile, QObject *parent = nullptr);
    virtual ~Hyperspace2Cpp();

    void setHeaderFile(const QString &headerFile);
    void setClassName(const QString &className);
    void setGeneratedClassName(const QString &generatedClassName);
    void setGeneratedBaseFileName(const QString &baseName);

protected Q_SLOTS:
    virtual void initImpl() override final;

private Q_SLOTS:
    void parseInterfaceFile();

    void parseConsumer();
    void parseAggregatedConsumer();
    void parseProducer();
    void parseAggregatedProducer();

    void writeConsumerPayload();
    void writeAggregatedConsumerPayload();
    void writeProducerPayload();
    void writeAggregatedProducerPayload();

private:
    QString methodNameFor(const QString &endpoint, const QString &methodName = QString());
    QString dataTypeFor(const QString &type, bool addConstness = false);
    QStringList producerMethodImplementation(const QString &generatedClassName, const QString &methodName, const QString &dataType, const QString &callArguments, const QString &endpoint,
                                             const QStringList &parameters, const QString &reliability, const QString &retention, int expiry, bool serialize = false);
    QStringList producerUnsetMethodImplementation(const QString &generatedClassName, const QString &methodName, const QString &callArguments,
                                                  const QString &endpoint, const QStringList &parameters);
    QString bsonSerializationFor(const QString &name, const QString &dataType);
    void addProducerErrorWave(const QString& endpoint, const QString& dataType, const QString& dataTypeNoConst, const QString& methodName,
                              const QString& signalAdditionalArguments, const QString& failedSignalArguments);

    QString m_sourceFile;
    QString m_headerFile;
    QString m_className;
    QString m_generatedClassName;
    QString m_generatedFileBaseName;
    QString m_interfaceName;
    QString m_dataEqualityOperatorPayload;
    InterfaceType m_interfaceType;

    QJsonObject m_interface;
    QStringList m_stateStorage;
    int m_lastAcceptableState;

    QStringList m_populateTokensAndStatesPayload;
    QStringList m_dispatchPayload;
    QStringList m_signalsPayload;

    QStringList m_dataMembersPayload;
    QStringList m_dataMethodsDeclarationPayload;
    QStringList m_dataCopyConstructorPayload;
    QStringList m_methodsDeclarationPayload;
    QStringList m_methodsImplementationPayload;
};

#endif // HYPERSPACE2CPP_H
