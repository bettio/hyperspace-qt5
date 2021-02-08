#include "hyperspace2cpp.h"

#include <QtCore/QDebug>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QRegularExpression>

#include <hyperspaceconfig.h>

Hyperspace2Cpp::Hyperspace2Cpp(const QString &sourceFile, QObject *parent)
    : Hemera::Generators::BaseGenerator(true, parent)
    , m_sourceFile(sourceFile)
    , m_lastAcceptableState(0)
{
}

Hyperspace2Cpp::~Hyperspace2Cpp()
{
}

void Hyperspace2Cpp::initImpl()
{
    connect(this, &Hyperspace2Cpp::ready, this, &Hyperspace2Cpp::parseInterfaceFile);

    setThingsToDo(1);
    setReady();
}

void Hyperspace2Cpp::setClassName(const QString &className)
{
    m_className = className;
}

void Hyperspace2Cpp::setGeneratedBaseFileName(const QString &baseName)
{
    m_generatedFileBaseName = baseName;
}

void Hyperspace2Cpp::setGeneratedClassName(const QString &generatedClassName)
{
    m_generatedClassName = generatedClassName;
}

void Hyperspace2Cpp::setHeaderFile(const QString &headerFile)
{
    m_headerFile = headerFile;
}

QString Hyperspace2Cpp::methodNameFor(const QString &endpoint, const QString &defaultMethodName)
{
    QString methodName = defaultMethodName;

    if (methodName.isEmpty()) {
        // create method name
        int previousIndex = 0;
        for (int i = endpoint.indexOf(QLatin1Char('/'), 0); i >= 0; i = endpoint.indexOf(QLatin1Char('/'), i+1)) {
            QString portionName = endpoint.mid(previousIndex+1, endpoint.indexOf(QLatin1Char('/'), i+1)- previousIndex - 1)
                                    .remove(QLatin1Char('%')).remove(QLatin1Char('{')).remove(QLatin1Char('}'));

            previousIndex = endpoint.indexOf(QLatin1Char('/'), i+1);
            portionName[0] = portionName[0].toUpper();
            methodName.append(portionName);
        }
    } else {
        methodName[0] = methodName[0].toUpper();
    }

    return methodName;
}

QString Hyperspace2Cpp::dataTypeFor(const QString &value, bool addConstness)
{
    QString dataType;
    if (value == QStringLiteral("integer")) {
        dataType = QStringLiteral("int");
    } else if (value == QStringLiteral("longinteger")) {
        dataType = QStringLiteral("qint64");
    } else if (value == QStringLiteral("double")) {
        dataType = QStringLiteral("double");
    } else if (value == QStringLiteral("datetime")) {
        if (addConstness) {
            dataType = QStringLiteral("const QDateTime &");
        } else {
            dataType = QStringLiteral("QDateTime");
        }
    } else if (value == QStringLiteral("string")) {
        if (addConstness) {
            dataType = QStringLiteral("const QString &");
        } else {
            dataType = QStringLiteral("QString");
        }
    } else if (value == QStringLiteral("boolean")) {
        dataType = QStringLiteral("bool");
    } else if (value == QStringLiteral("binary")) {
        if (addConstness) {
            dataType = QStringLiteral("const QByteArray &");
        } else {
            dataType = QStringLiteral("QByteArray");
        }
    } else {
        terminateWithError(QStringLiteral("Type %1 unspecified!").arg(value));
    }

    return dataType;
}

QString Hyperspace2Cpp::bsonSerializationFor(const QString &name, const QString &dataType)
{
    if (dataType == QStringLiteral("int")) {
        return QStringLiteral("appendInt32Value(\"%1\", d->%1)").arg(name);
    } else if (dataType == QStringLiteral("qint64")) {
        return QStringLiteral("appendInt64Value(\"%1\", d->%1)").arg(name);
    } else if (dataType == QStringLiteral("QString")) {
        return QStringLiteral("appendString(\"%1\", d->%1)").arg(name);
    } else if (dataType == QStringLiteral("QDateTime")) {
        return QStringLiteral("appendDateTime(\"%1\", d->%1)").arg(name);
    } else if (dataType == QStringLiteral("QByteArray")) {
        return QStringLiteral("appendBinaryValue(\"%1\", d->%1)").arg(name);
    } else if (dataType == QStringLiteral("bool")) {
        return QStringLiteral("appendBooleanValue(\"%1\", d->%1)").arg(name);
    } else if (dataType == QStringLiteral("double")) {
        return QStringLiteral("appendDoubleValue(\"%1\", d->%1)").arg(name);
    } else {
        terminateWithError(QStringLiteral("Type %1 in aggregate interface doesn't have a BSON serialize method").arg(dataType));
    }

    return QString();
}

QStringList Hyperspace2Cpp::producerMethodImplementation(const QString &generatedClassName, const QString &methodName, const QString &dataType, const QString &callArguments,
                                                  const QString &endpoint, const QStringList &parameters, const QString &reliability, const QString &retention, int expiry, bool serialize)
{
    QStringList ret;

    // Method implementation
    ret.append(QStringLiteral("void %1::%2(%3 value%4)").arg(generatedClassName, methodName, dataType, callArguments));
    ret.append(QStringLiteral("{"));
    ret.append(QStringLiteral("    QByteArray endpoint(\"%1\");").arg(endpoint));
    for (const QString &parameter : parameters) {
        ret.append(QStringLiteral("    endpoint = endpoint.replace(\"%{%1}\", %1);").arg(parameter));
    }
    if (m_interfaceType == DataStreamType) {
        ret.append(QStringLiteral("    QHash<QByteArray, QByteArray> attributes;"));
        if (reliability == QStringLiteral("unique")) {
            ret.append(QStringLiteral("    attributes.insert(\"reliability\", QByteArray::number(static_cast<int>(Hyperspace::Reliability::Unique)));"));
        } else if (reliability == QStringLiteral("guaranteed")) {
            ret.append(QStringLiteral("    attributes.insert(\"reliability\", QByteArray::number(static_cast<int>(Hyperspace::Reliability::Guaranteed)));"));
        } else {
            // Reliability defaults to unreliable
            ret.append(QStringLiteral("    attributes.insert(\"reliability\", QByteArray::number(static_cast<int>(Hyperspace::Reliability::Unreliable)));"));
        }

        if (retention == QStringLiteral("stored")) {
            ret.append(QStringLiteral("    attributes.insert(\"retention\", QByteArray::number(static_cast<int>(Hyperspace::Retention::Stored)));"));
            if (expiry > 0) {
                ret.append(QStringLiteral("    attributes.insert(\"expiry\", QByteArray::number(%1));").arg(expiry));
            }
        } else if (retention == QStringLiteral("volatile")) {
            ret.append(QStringLiteral("    attributes.insert(\"retention\", QByteArray::number(static_cast<int>(Hyperspace::Retention::Volatile)));"));
            if (expiry > 0) {
                ret.append(QStringLiteral("    attributes.insert(\"expiry\", QByteArray::number(%1));").arg(expiry));
            }
        } else {
            // Retention defaults to discard
            ret.append(QStringLiteral("    attributes.insert(\"retention\", QByteArray::number(static_cast<int>(Hyperspace::Retention::Discard)));"));
        }

        if (serialize) {
            ret.append(QStringLiteral("    sendRawDataOnEndpoint(value.serialize(), endpoint, attributes);"));
        } else {
            ret.append(QStringLiteral("    sendDataOnEndpoint(value, endpoint, attributes);"));
        }
    } else {
        if (serialize) {
            ret.append(QStringLiteral("    sendRawDataOnEndpoint(value.serialize(), endpoint);"));
        } else {
            ret.append(QStringLiteral("    sendDataOnEndpoint(value, endpoint);"));
        }
    }
    ret.append(QStringLiteral("}"));
    ret.append(QStringLiteral(""));
    return ret;
}

QStringList Hyperspace2Cpp::producerUnsetMethodImplementation(const QString &generatedClassName, const QString &methodName, const QString &callArguments,
                                                  const QString &endpoint, const QStringList &parameters)
{
    QStringList ret;

    // Method implementation
    ret.append(QStringLiteral("void %1::un%2(%3)").arg(generatedClassName, methodName, callArguments.mid(2)));
    ret.append(QStringLiteral("{"));
    ret.append(QStringLiteral("    QByteArray endpoint(\"%1\");").arg(endpoint));
    for (const QString &parameter : parameters) {
        ret.append(QStringLiteral("    endpoint = endpoint.replace(\"%{%1}\", %1);").arg(parameter));
    }
    ret.append(QStringLiteral("    sendDataOnEndpoint(QByteArray(), endpoint);"));
    ret.append(QStringLiteral("}"));
    ret.append(QStringLiteral(""));
    return ret;
}

void Hyperspace2Cpp::addProducerErrorWave(const QString& endpoint, const QString& dataType, const QString& dataTypeNoConst, const QString& methodName,
                                          const QString& signalAdditionalArguments, const QString& failedSignalArguments)
{
    // Add signal
    m_signalsPayload.append(QStringLiteral("    void %1Failed(%2 payload%3);").arg(methodName, dataType, signalAdditionalArguments));

    // Add dispatch
    m_dispatchPayload.append(QStringLiteral("        // %1").arg(endpoint));
    m_dispatchPayload.append(QStringLiteral("        case %1: {").arg(m_lastAcceptableState));
    m_dispatchPayload.append(QStringLiteral("            %1 value;").arg(dataTypeNoConst));
    m_dispatchPayload.append(QStringLiteral("            if (!payloadToValue(payload, &value)) return CouldNotConvertPayload;"));
    m_dispatchPayload.append(QStringLiteral("            Q_EMIT %1Failed(value%2);").arg(methodName, failedSignalArguments));
    m_dispatchPayload.append(QStringLiteral("            return Success;"));
    m_dispatchPayload.append(QStringLiteral("        }"));

    // for beauty
    m_populateTokensAndStatesPayload.append(QStringLiteral("    // %1").arg(endpoint));
    QString endpointWithoutParams = endpoint;
    endpointWithoutParams.replace(QRegularExpression(QStringLiteral("%{[a-zA-Z0-9]*}")), QString());

    // Iterate
    int previousStateIndex = 0;
    for (int i = endpointWithoutParams.indexOf(QLatin1Char('/'), 1); i != 0; i = endpointWithoutParams.indexOf(QLatin1Char('/'), i+1)) {
        QString partialEndpoint = endpointWithoutParams.mid(0, i);
        if (m_stateStorage.contains(partialEndpoint)) {
            previousStateIndex = m_stateStorage.indexOf(partialEndpoint) + 1;
            continue;
        }

        m_stateStorage.append(partialEndpoint);

        QString stateString = partialEndpoint.split(QLatin1Char('/')).last();
        m_populateTokensAndStatesPayload.append(QStringLiteral("    insertTransition(%1, \"%2\", %3);")
                .arg(previousStateIndex).arg(stateString).arg(m_stateStorage.count()));
        previousStateIndex = m_stateStorage.count();
    }

    // Add transition.
    m_populateTokensAndStatesPayload.append(QStringLiteral("    insertDispatchState(%1, %2);").arg(previousStateIndex).arg(m_lastAcceptableState));
    // for beauty
    m_populateTokensAndStatesPayload.append(QString());

    ++m_lastAcceptableState;
}

void Hyperspace2Cpp::parseInterfaceFile()
{
    m_interface = QJsonDocument::fromJson(payload(m_sourceFile)).object();
    if (m_interface.isEmpty()) {
        terminateWithError(tr("Interface is not a valid JSON file!"));
        return;
    }

    if (m_interface.contains(QStringLiteral("interface_name"))) {
        m_interfaceName = m_interface.value(QStringLiteral("interface_name")).toString();
    } else {
        m_interfaceName = m_interface.value(QStringLiteral("interface")).toString();
        qWarning() << m_interfaceName << ": interface is deprecated, use interface_name";
    }

    if (m_interface.value(QStringLiteral("type")).toString() == QStringLiteral("properties")) {
        m_interfaceType = PropertiesType;
    } else if (m_interface.value(QStringLiteral("type")).toString() == QStringLiteral("datastream")) {
        m_interfaceType = DataStreamType;
    } else {
        terminateWithError(QStringLiteral("Type %1 unknown!").arg(m_interface.value(QStringLiteral("type")).toString()));
    }

    bool aggregate = m_interface.value(QStringLiteral("aggregate")).toBool(false);

    if (m_interface.value(QStringLiteral("quality")).toString() == QStringLiteral("consumer")) {
        if (m_className.isEmpty() || m_headerFile.isEmpty()) {
            terminateWithError(tr("Class Name or Header File not supplied!"));
            return;
        }
        if (m_generatedClassName.isEmpty()) {
            // Auto-generate
            m_generatedClassName = QStringLiteral("%1Consumer").arg(m_interfaceName.split(QLatin1Char('.')).last());
        }
        if (m_generatedFileBaseName.isEmpty()) {
            // Auto-generate
            m_generatedFileBaseName = m_generatedClassName.toLower();
        }
        if (aggregate) {
            parseAggregatedConsumer();
        } else {
            parseConsumer();
        }
    } else if (m_interface.value(QStringLiteral("quality")).toString() == QStringLiteral("producer")) {
        if (m_generatedClassName.isEmpty()) {
            // Auto-generate
            m_generatedClassName = QStringLiteral("%1Producer").arg(m_interfaceName.split(QLatin1Char('.')).last());
        }
        if (m_generatedFileBaseName.isEmpty()) {
            // Auto-generate
            m_generatedFileBaseName = m_generatedClassName.toLower();
        }
        if (aggregate) {
            parseAggregatedProducer();
        } else {
            parseProducer();
        }
    } else {
        terminateWithError(QStringLiteral("Quality %1 unknown!").arg(m_interface.value(QStringLiteral("quality")).toString()));
    }
}

void Hyperspace2Cpp::parseConsumer()
{
    // Build the tree.
    for (const QJsonValue &value : m_interface.value(QStringLiteral("mappings")).toArray()) {
        QJsonObject mapping = value.toObject();
        QString endpoint = mapping.value(QStringLiteral("path")).toString();

        if (!endpoint.startsWith(QLatin1Char('/')) || endpoint.endsWith(QLatin1Char('/'))) {
            terminateWithError(tr("Mappings should always start with / and have no trailing slash! %1").arg(endpoint));
            return;
        }

        // Manage methods
        QString methodName(QStringLiteral("%1%2").arg(m_interfaceType == PropertiesType ? QStringLiteral("set") : QStringLiteral("receive"),
                                                      methodNameFor(endpoint, mapping.value(QStringLiteral("method_name")).toString())));
        QString dataType = dataTypeFor(mapping.value(QStringLiteral("type")).toString());
        if (dataType.isEmpty()) {
            return;
        }
        // Deduct method arguments
        QString callArguments;
        if (endpoint.contains(QStringLiteral("%{"))) {
            QStringList endpointTokens = endpoint.mid(1).split(QLatin1Char('/'));
            for (int i = 0; i < endpointTokens.count(); i++) {
                if (endpointTokens.at(i).startsWith(QStringLiteral("%{"))) {
                    callArguments.append(QStringLiteral(", inputTokens[%1]").arg(i));
                }
            }
        }
        // Add dispatch
        m_dispatchPayload.append(QStringLiteral("        // %1").arg(endpoint));
        m_dispatchPayload.append(QStringLiteral("        case %1: {").arg(m_lastAcceptableState));
        m_dispatchPayload.append(QStringLiteral("            %1 value;").arg(dataType));
        if (mapping.value(QStringLiteral("allow_unset")).toBool()) {
            if (m_interface.value(QStringLiteral("type")).toString() != QStringLiteral("properties")) {
                terminateWithError(tr("allow_unset can be used only with properties interface types").arg(endpoint));
                return;
            }
            m_dispatchPayload.append(QStringLiteral("            if (payload.isEmpty()) {"));
            m_dispatchPayload.append(QStringLiteral("                parent()->un%1(%2);").arg(methodName).arg(callArguments.mid(2)));
            m_dispatchPayload.append(QStringLiteral("                return Success;"));
            m_dispatchPayload.append(QStringLiteral("            }"));
        }
        m_dispatchPayload.append(QStringLiteral("            if (!payloadToValue(payload, &value)) return CouldNotConvertPayload;"));
        m_dispatchPayload.append(QStringLiteral("            parent()->%1(value%2);").arg(methodName).arg(callArguments));
        m_dispatchPayload.append(QStringLiteral("            return Success;"));
        m_dispatchPayload.append(QStringLiteral("        }"));

        // for beauty
        m_populateTokensAndStatesPayload.append(QStringLiteral("    // %1").arg(endpoint));
        endpoint.replace(QRegularExpression(QStringLiteral("%{[a-zA-Z0-9]*}")), QString());

        // Iterate
        int previousStateIndex = 0;
        for (int i = endpoint.indexOf(QLatin1Char('/'), 1); i != 0; i = endpoint.indexOf(QLatin1Char('/'), i+1)) {
            QString partialEndpoint = endpoint.mid(0, i);
            if (m_stateStorage.contains(partialEndpoint)) {
                previousStateIndex = m_stateStorage.indexOf(partialEndpoint) + 1;
                continue;
            }

            m_stateStorage.append(partialEndpoint);

            QString stateString = partialEndpoint.split(QLatin1Char('/')).last();
            m_populateTokensAndStatesPayload.append(QStringLiteral("    insertTransition(%1, \"%2\", %3);")
                                                    .arg(previousStateIndex).arg(stateString).arg(m_stateStorage.count()));
            previousStateIndex = m_stateStorage.count();
            // TODO: add to file.
        }

        // Add transition.
        m_populateTokensAndStatesPayload.append(QStringLiteral("    insertDispatchState(%1, %2);").arg(previousStateIndex).arg(m_lastAcceptableState));
        // for beauty
        m_populateTokensAndStatesPayload.append(QString());

        ++m_lastAcceptableState;
    }

    writeConsumerPayload();
}

void Hyperspace2Cpp::parseAggregatedConsumer()
{
}

void Hyperspace2Cpp::writeConsumerPayload()
{
    QString headerPayload = QString::fromLatin1(payload(QStringLiteral("%1/hyperspaceconsumerinterface.h.in")
                                                .arg(Hyperspace::StaticConfig::hyperspaceDataDir())));
    headerPayload = headerPayload.arg(m_generatedClassName, m_className, m_headerFile);

    QString implPayload = QString::fromLatin1(payload(QStringLiteral("%1/hyperspaceconsumerinterface.cpp.in")
                                              .arg(Hyperspace::StaticConfig::hyperspaceDataDir())));
    implPayload = implPayload.arg(m_generatedClassName, m_className, m_generatedFileBaseName, m_interfaceName,
                                  m_populateTokensAndStatesPayload.join(QLatin1Char('\n')),
                                  m_dispatchPayload.join(QLatin1Char('\n')));

    writeFile(QStringLiteral("%1.h").arg(m_generatedFileBaseName), headerPayload.toLatin1());
    writeFile(QStringLiteral("%1.cpp").arg(m_generatedFileBaseName), implPayload.toLatin1());

    oneThingLessToDo();
}

void Hyperspace2Cpp::writeAggregatedConsumerPayload()
{
}

void Hyperspace2Cpp::parseProducer()
{
    for (const QJsonValue &value : m_interface.value(QStringLiteral("mappings")).toArray()) {
        QJsonObject mapping = value.toObject();
        QString endpoint = mapping.value(QStringLiteral("path")).toString();

        if (!endpoint.startsWith(QLatin1Char('/')) || endpoint.endsWith(QLatin1Char('/'))) {
            terminateWithError(tr("Mappings should always start with / and have no trailing slash! %1").arg(endpoint));
            return;
        }

        // Manage methods
        QString methodName(QStringLiteral("%1%2").arg(m_interfaceType == PropertiesType ? QStringLiteral("set") : QStringLiteral("stream"),
                                                      methodNameFor(endpoint, mapping.value(QStringLiteral("method_name")).toString())));
        QString dataType = dataTypeFor(mapping.value(QStringLiteral("type")).toString(), true);
        QString reliability = mapping.value(QStringLiteral("reliability")).toString();
        if (m_interfaceType == DataStreamType && !reliability.isEmpty()
            && reliability != QStringLiteral("unreliable") && reliability != QStringLiteral("guaranteed") && reliability != QStringLiteral("unique")) {
            terminateWithError(tr("Producer mappings reliability must be unreliable, guaranteed or unique (%1 : %2)").arg(endpoint, reliability));
            return;
        }
        QString retention = mapping.value(QStringLiteral("retention")).toString();
        if (m_interfaceType == DataStreamType && !retention.isEmpty()
            && retention != QStringLiteral("discard") && retention != QStringLiteral("volatile") && retention != QStringLiteral("stored")) {
            terminateWithError(tr("Producer mappings retention must be discard, volatile or stored (%1 : %2)").arg(endpoint, retention));
            return;
        }
        int expiry = mapping.value(QStringLiteral("expiry")).toInt();
        // Deduct method arguments
        QString callArguments;
        int previousIndex = 0;
        QStringList parameters;
        // Needed for error wave
        QString failedSignalArguments;
        int tokenIndex = 0;
        for (int i = endpoint.indexOf(QLatin1Char('/'), 0); i >= 0; i = endpoint.indexOf(QLatin1Char('/'), i+1)) {
            QString portionName = endpoint.mid(previousIndex+1, endpoint.indexOf(QLatin1Char('/'), i+1)- previousIndex - 1);
            previousIndex = endpoint.indexOf(QLatin1Char('/'), i+1);

            if (!portionName.contains(QStringLiteral("%{"))) {
                ++tokenIndex;
                continue;
            }
            portionName.remove(QLatin1Char('%')).remove(QLatin1Char('{')).remove(QLatin1Char('}'));
            parameters.append(portionName);
            callArguments.append(QStringLiteral(", const QByteArray &%1").arg(portionName));

            failedSignalArguments.append(QStringLiteral(", inputTokens[%1]").arg(tokenIndex));
            ++tokenIndex;
        }

        // Method declaration
        m_methodsDeclarationPayload.append(QStringLiteral("    void %1(%2 value%3);").arg(methodName, dataType, callArguments));

        m_methodsImplementationPayload.append(producerMethodImplementation(m_generatedClassName, methodName, dataType, callArguments,
                                                                           endpoint, parameters, reliability, retention, expiry));

        if (mapping.value(QStringLiteral("allow_unset")).toBool()) {
           if (m_interface.value(QStringLiteral("type")).toString() != QStringLiteral("properties")) {
               terminateWithError(tr("allow_unset can be used only with properties interface types").arg(endpoint));
               return;
           }

           m_methodsDeclarationPayload.append(QStringLiteral("    void un%1(%2);").arg(methodName, callArguments.mid(2)));


           m_methodsImplementationPayload.append(producerUnsetMethodImplementation(m_generatedClassName, methodName, callArguments,
                                                                           endpoint, parameters));
        }

        // Error wave
        if (m_interfaceType == DataStreamType && (retention == QStringLiteral("discard") || retention.isEmpty())) {
            QString dataTypeNoConst = dataTypeFor(mapping.value(QStringLiteral("type")).toString());
            addProducerErrorWave(endpoint, dataType, dataTypeNoConst, methodName, callArguments, failedSignalArguments);
        }
    }

    writeProducerPayload();
}

void Hyperspace2Cpp::parseAggregatedProducer()
{
    QString callArguments;
    QString equalityChain;
    QString aggregateRetention;
    QString aggregateReliability;
    QString aggregateTargetParameter;
    int aggregateExpiry = -1;

    m_dataMethodsDeclarationPayload.append(QStringLiteral("        QByteArray serialize() const;"));

    QStringList serializeImplementation;
    serializeImplementation.append(QStringLiteral("QByteArray %1::Data::serialize() const").arg(m_generatedClassName));
    serializeImplementation.append(QStringLiteral("{"));
    serializeImplementation.append(QStringLiteral("    Hyperspace::Util::BSONSerializer s;"));

    for (const QJsonValue &value : m_interface.value(QStringLiteral("mappings")).toArray()) {
        QJsonObject mapping = value.toObject();
        QString endpoint = mapping.value(QStringLiteral("path")).toString();

        if (!endpoint.startsWith(QLatin1Char('/')) || endpoint.endsWith(QLatin1Char('/'))) {
            terminateWithError(tr("Mappings should always start with / and have no trailing slash! %1").arg(endpoint));
            return;
        }

        if (!((endpoint.count(QStringLiteral("%{")) == 1 && endpoint.count(QLatin1Char('/')) == 2)
             || (endpoint.count(QStringLiteral("%{")) == 0 && endpoint.count(QLatin1Char('/')) == 1 ))) {
            terminateWithError(tr("Aggregate interface mappings can only have a depth of 2 with one parameter or 1 with no parameters: %1").arg(endpoint));
            return;
        }

        // Field name
        QString fieldName = endpoint.split(QLatin1Char('/'), QString::SkipEmptyParts).last();
        QString fieldNameUpper = fieldName;
        fieldNameUpper[0] = fieldNameUpper[0].toUpper();

        // Manage methods
        QString setterName = QStringLiteral("set%1").arg(fieldNameUpper);
        QString memberName = fieldName;

        QString dataType = dataTypeFor(mapping.value(QStringLiteral("type")).toString());
        QString constDataType = dataTypeFor(mapping.value(QStringLiteral("type")).toString(), true);


        QString reliability = mapping.value(QStringLiteral("reliability")).toString();
        if (m_interfaceType == DataStreamType && !reliability.isEmpty()
            && reliability != QStringLiteral("unreliable") && reliability != QStringLiteral("guaranteed") && reliability != QStringLiteral("unique")) {
            terminateWithError(tr("Producer mappings reliability must be unreliable, guaranteed or unique (%1 : %2)").arg(endpoint, reliability));
            return;
        } else if (aggregateReliability.isEmpty()) {
            // Store it to check later
            aggregateReliability = reliability;
        } else if (aggregateReliability != reliability) {
            terminateWithError(tr("Aggregate producer mappings reliability must be the same in all mappings (%1 : %2)").arg(endpoint, reliability));
            return;
        }
        QString retention = mapping.value(QStringLiteral("retention")).toString();
        if (m_interfaceType == DataStreamType && !retention.isEmpty()
            && retention != QStringLiteral("discard") && retention != QStringLiteral("volatile") && retention != QStringLiteral("stored")) {
            terminateWithError(tr("Producer mappings retention must be discard, volatile or stored (%1 : %2)").arg(endpoint, retention));
            return;
        } else if (aggregateRetention.isEmpty()) {
            // Store it to check later
            aggregateRetention = retention;
        } else if (aggregateRetention != retention) {
            terminateWithError(tr("Aggregate producer mappings retention must be the same in all mappings (%1 : %2)").arg(endpoint, retention));
            return;
        }

        int expiry = mapping.value(QStringLiteral("expiry")).toInt();
        if (aggregateExpiry == -1) {
            // Store it to check later
            aggregateExpiry = expiry;
        } else if (aggregateExpiry != expiry) {
            terminateWithError(tr("Aggregate producer mappings expiry must be the same in all mappings (%1 : %2)").arg(endpoint).arg(expiry));
            return;
        }

        if (endpoint.contains(QStringLiteral("%{"))) {
            int paramBeginIndex = endpoint.indexOf(QStringLiteral("%{"));
            int paramEndIndex = endpoint.indexOf(QStringLiteral("}"));
            QString targetParameter = endpoint.mid(paramBeginIndex + 2, paramEndIndex - (paramBeginIndex + 2));
            if (aggregateTargetParameter.isEmpty()) {
                aggregateTargetParameter = targetParameter;
                callArguments.append(QStringLiteral(", const QByteArray &%1").arg(targetParameter));
            } else if (aggregateTargetParameter != targetParameter) {
                terminateWithError(tr("The parameter in the aggregate mapping must be the same in all mappings (%1 : %2)").arg(endpoint, targetParameter));
                return;
            }
        }

        m_dataMethodsDeclarationPayload.append(QStringLiteral("        %1 %2() const;").arg(dataType, memberName));
        m_dataMethodsDeclarationPayload.append(QStringLiteral("        void %1(%2 value);").arg(setterName, constDataType));
        m_dataCopyConstructorPayload.append(QStringLiteral("        , %1(other.%1)").arg(memberName));
        m_dataMembersPayload.append(QStringLiteral("    %1 %2;").arg(dataType, memberName));
        equalityChain.append(QStringLiteral("(d->%1 == other.%1()) && ").arg(memberName));

        serializeImplementation.append(QStringLiteral("    s.%1;").arg(bsonSerializationFor(memberName, dataType)));

        // Getter
        m_methodsImplementationPayload.append(QStringLiteral("%1 %2::Data::%3() const").arg(dataType, m_generatedClassName, memberName));
        m_methodsImplementationPayload.append(QStringLiteral("{"));
        m_methodsImplementationPayload.append(QStringLiteral("    return d->%1;").arg(memberName));
        m_methodsImplementationPayload.append(QStringLiteral("}"));
        m_methodsImplementationPayload.append(QStringLiteral(""));

        // Setter
        m_methodsImplementationPayload.append(QStringLiteral("void %1::Data::%2(%3 value)").arg(m_generatedClassName, setterName, constDataType));
        m_methodsImplementationPayload.append(QStringLiteral("{"));
        m_methodsImplementationPayload.append(QStringLiteral("    d->%1 = value;").arg(memberName));
        m_methodsImplementationPayload.append(QStringLiteral("}"));
        m_methodsImplementationPayload.append(QStringLiteral(""));
    }

    // Remove the last " && "
    equalityChain.chop(4);
    m_dataEqualityOperatorPayload.append(QStringLiteral("    return %1;").arg(equalityChain));

    serializeImplementation.append(QStringLiteral("    s.appendEndOfDocument();"));
    serializeImplementation.append(QStringLiteral(""));
    serializeImplementation.append(QStringLiteral("    return s.document();"));
    serializeImplementation.append(QStringLiteral("}"));
    serializeImplementation.append(QStringLiteral(""));

    m_methodsImplementationPayload.append(serializeImplementation);

    QString methodName;
    if (m_interfaceType == DataStreamType) {
        methodName = QStringLiteral("streamData");
    } else {
        methodName = QStringLiteral("setData");
    }
    QString aggregateEndpoint;
    QStringList parameters;
    if (!aggregateTargetParameter.isEmpty()) {
        aggregateEndpoint.append(QStringLiteral("/%{%1}").arg(aggregateTargetParameter));
        parameters << aggregateTargetParameter;
    }
    QString dataType = QStringLiteral("const Data &");

    m_methodsDeclarationPayload.append(QStringLiteral("    void %1(const Data &value%2);").arg(methodName, callArguments));

    m_methodsImplementationPayload.append(producerMethodImplementation(m_generatedClassName, methodName, dataType, callArguments,
                                          aggregateEndpoint, parameters, aggregateReliability, aggregateRetention, aggregateExpiry, true));

    writeAggregatedProducerPayload();

}

void Hyperspace2Cpp::writeProducerPayload()
{
    QString headerPayload = QString::fromLatin1(payload(QStringLiteral("%1/hyperspaceproducerinterface.h.in")
                                                .arg(Hyperspace::StaticConfig::hyperspaceDataDir())));
    headerPayload = headerPayload.arg(m_generatedClassName, m_methodsDeclarationPayload.join(QLatin1Char('\n')), m_signalsPayload.join(QLatin1Char('\n')));

    QString implPayload = QString::fromLatin1(payload(QStringLiteral("%1/hyperspaceproducerinterface.cpp.in")
                                              .arg(Hyperspace::StaticConfig::hyperspaceDataDir())));
    implPayload = implPayload.arg(m_generatedClassName, m_generatedFileBaseName, m_interfaceName,
                                  m_methodsImplementationPayload.join(QLatin1Char('\n')),
                                  m_populateTokensAndStatesPayload.join(QLatin1Char('\n')),
                                  m_dispatchPayload.join(QLatin1Char('\n')));

    writeFile(QStringLiteral("%1.h").arg(m_generatedFileBaseName), headerPayload.toLatin1());
    writeFile(QStringLiteral("%1.cpp").arg(m_generatedFileBaseName), implPayload.toLatin1());

    oneThingLessToDo();
}

void Hyperspace2Cpp::writeAggregatedProducerPayload()
{
    QString headerPayload = QString::fromLatin1(payload(QStringLiteral("%1/hyperspaceaggregateproducerinterface.h.in")
                                                .arg(Hyperspace::StaticConfig::hyperspaceDataDir())));
    headerPayload = headerPayload.arg(m_generatedClassName, m_methodsDeclarationPayload.join(QLatin1Char('\n')),
                                      m_dataMethodsDeclarationPayload.join(QLatin1Char('\n')));

    QString implPayload = QString::fromLatin1(payload(QStringLiteral("%1/hyperspaceaggregateproducerinterface.cpp.in")
                                              .arg(Hyperspace::StaticConfig::hyperspaceDataDir())));
    implPayload = implPayload.arg(m_generatedClassName, m_generatedFileBaseName, m_interfaceName,
                                  m_methodsImplementationPayload.join(QLatin1Char('\n')), m_dataCopyConstructorPayload.join(QLatin1Char('\n')),
                                  m_dataMembersPayload.join(QLatin1Char('\n')), m_dataEqualityOperatorPayload);

    writeFile(QStringLiteral("%1.h").arg(m_generatedFileBaseName), headerPayload.toLatin1());
    writeFile(QStringLiteral("%1.cpp").arg(m_generatedFileBaseName), implPayload.toLatin1());

    oneThingLessToDo();

}

#include "hyperspace2cpp.moc"
