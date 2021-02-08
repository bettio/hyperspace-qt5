#include <QtCore/QCommandLineParser>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QStringList>

#include <HemeraCore/Operation>

#include "hyperspace2cpp.h"

#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    app.setApplicationName(Hyperspace2Cpp::tr("Hyperspace C++ Consumer/Producer interface Generator"));
    app.setOrganizationDomain(QStringLiteral("com.ispirata.Hemera"));
    app.setOrganizationName(QStringLiteral("Ispirata"));

    QCommandLineParser parser;
    parser.setApplicationDescription(Hyperspace2Cpp::tr("Hyperspace C++ Consumer/Producer interface Generator"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("interface"), Hyperspace2Cpp::tr("Interface file (JSON)."));

    // Add options
    parser.addOptions({
        {
            QStringList{QStringLiteral("l"), QStringLiteral("parent-class-name")},
            Hyperspace2Cpp::tr("Class name of the main implementation. Makes sense for consumer only."),
            Hyperspace2Cpp::tr("Class Name")
        },
        {
            QStringList{QStringLiteral("i"), QStringLiteral("header")},
            Hyperspace2Cpp::tr("Header file of the main implementation. Makes sense for consumer only."),
            Hyperspace2Cpp::tr("Header file")
        },
        {
            QStringList{QStringLiteral("c"), QStringLiteral("generated-class-name")},
            Hyperspace2Cpp::tr("Output name for the generated class. Optional."),
            Hyperspace2Cpp::tr("Output name")

        },
        {
            QStringList{QStringLiteral("a"), QStringLiteral("generated-file-basename")},
            Hyperspace2Cpp::tr("Output base name for the generated files. Optional."),
            Hyperspace2Cpp::tr("Output base name")

        }
    });

    // Process the actual command line arguments given by the user
    parser.process(app);

    QString interfaceFile = parser.positionalArguments().first();

    Hyperspace2Cpp ag(interfaceFile);
    ag.setClassName(parser.value(QStringLiteral("l")));
    ag.setHeaderFile(parser.value(QStringLiteral("i")));
    ag.setGeneratedClassName(parser.value(QStringLiteral("c")));
    ag.setGeneratedBaseFileName(parser.value(QStringLiteral("a")));

    Hemera::Operation *op = ag.init();

    QObject::connect(op, &Hemera::Operation::finished, [op] {
        if (op->isError()) {
            std::cerr << Hyperspace2Cpp::tr("Hyperspace2Cpp could not be initialized. %1: %2")
                         .arg(op->errorName(), op->errorMessage()).toStdString() << std::endl;
            QCoreApplication::instance()->exit(1);
        }
    });

    return app.exec();
}

