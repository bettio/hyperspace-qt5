#ifndef _BSON_STREAMREADER_H_
#define _BSON_STREAMREADER_H_

#include <QtCore/QByteArray>
#include <QtCore/QList>

namespace Hyperspace
{

namespace Util
{

class BSONStreamReader
{
    public:
        BSONStreamReader();
        void enqueueData(QByteArray newData);
        bool canReadDocument() const;
        QByteArray dequeueDocumentData();

    private:
        QByteArrayList m_documents;
        QByteArray m_dataBacklog;
        int m_neededSize;
};

} // Util
} // Hyperspace

#endif
