#include <HemeraTest/Test>

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QPointer>
#include <QtCore/QProcess>
#include <QtCore/QThread>
#include <QtCore/QTimer>

#include <HyperspaceCore/BSONDocument>
#include <HyperspaceCore/BSONSerializer>
#include <HyperspaceCore/BSONStreamReader>

#include <hyperspaceconfig.h>

using namespace Hyperspace;

class BSONBasics : public Hemera::Test::Test
{
    Q_OBJECT

public:
    BSONBasics(QObject *parent = 0)
        : Test(parent)
    { }

private Q_SLOTS:
    void initTestCase();
    void init();

    void testBSONDocument();
    void testParseBSONFromPython();
    void testSerializeBSONToPython();

    void cleanup();
    void cleanupTestCase();

private:
};

void BSONBasics::initTestCase()
{
    initTestCaseImpl();
}

void BSONBasics::init()
{
    initImpl();
}

void BSONBasics::testBSONDocument()
{
    Util::BSONSerializer s;
    s.appendInt32Value("y", 42);
    s.appendASCIIString("i", "the things");
    s.appendBinaryValue("p", "binary things");
    s.appendEndOfDocument();

    Util::BSONDocument doc = s.document();
    QVERIFY(doc.isValid());
    QCOMPARE(doc.int32Value("y"), (qint32)42);
    QCOMPARE(doc.byteArrayValue("i"), QByteArray("the things"));
    QCOMPARE(doc.byteArrayValue("p"), QByteArray("binary things"));
}

void BSONBasics::testParseBSONFromPython()
{
    // This bytearray is kindly provided by python.
    QProcess proc;
    proc.start(QStringLiteral("%1/tests/bson-basics-generate.py").arg(StaticConfig::sourceDirectory()));
    QVERIFY(proc.waitForFinished());
    QVERIFY(proc.exitCode() == 0);
    Util::BSONDocument doc(proc.readAll());
    QVERIFY(doc.isValid());
    QCOMPARE(doc.int32Value("y"), (qint32)42);
    QCOMPARE(doc.byteArrayValue("i"), QByteArray("the things"));
    QCOMPARE(doc.byteArrayValue("p"), QByteArray("binary things"));
}

void BSONBasics::testSerializeBSONToPython()
{
    Util::BSONSerializer s;
    s.appendInt32Value("y", 42);
    s.appendASCIIString("i", "the things");
    s.appendBinaryValue("p", "binary things");
    s.appendEndOfDocument();

    Util::BSONDocument doc = s.document();
    QVERIFY(doc.isValid());

    QProcess proc;
    proc.start(QStringLiteral("%1/tests/bson-basics-parse.py").arg(StaticConfig::sourceDirectory()));
    QVERIFY(proc.waitForStarted());
    proc.write(doc.toByteArray());
    proc.write("\n");
    QVERIFY(proc.waitForFinished());
    QCOMPARE(proc.exitCode(), 0);

    QList< QByteArray > values = proc.readAll().split('\n');
    // Excess newline
    values.removeLast();
    QCOMPARE(values.size(), 3);
    QCOMPARE(values[0].toInt(), 42);
    QCOMPARE(values[1], QByteArray("the things"));
    QCOMPARE(values[2], QByteArray("binary things"));
}

void BSONBasics::cleanup()
{
    cleanupImpl();
}

void BSONBasics::cleanupTestCase()
{
    cleanupTestCaseImpl();
}

QTEST_MAIN(BSONBasics)
#include "bson-basics.cpp.moc.hpp"
