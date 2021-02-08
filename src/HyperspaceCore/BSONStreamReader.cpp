#include "BSONStreamReader.h"

#include "BSONDocument.h"

using namespace Hyperspace::Util;

BSONStreamReader::BSONStreamReader()
:    m_neededSize(0)
{
}

void BSONStreamReader::enqueueData(QByteArray data)
{
    if (!m_dataBacklog.isEmpty()) {
        data.prepend(m_dataBacklog);
        m_dataBacklog.clear();
    }

    while (!data.isEmpty()) {
        // we were waiting a new message
        if (!m_neededSize) {
        //TODO: handle minimum BSON document size
            if (data.count() >= 4) {
                BSONDocument tmpDoc(data);
                m_neededSize = tmpDoc.size();

            // message is really small, just enqueue it
            } else {
                m_dataBacklog.append(data);
                return;
            }
        }


        // we are at the begining and the document has the right size
        // or it is bigger
        if (m_neededSize <= data.size()) {
            bool biggerChunk = (m_neededSize < data.size());
            QByteArray documentData = biggerChunk ? data.left(m_neededSize) : data;

            m_documents.append(documentData);

            if (biggerChunk) {
                data.remove(0, m_neededSize);
                m_neededSize = 0;
            } else {
                m_neededSize = 0;
                return;
            }
        } else {
            m_dataBacklog.append(data);
            return;
        }
    }
}

bool BSONStreamReader::canReadDocument() const
{
    return !m_documents.isEmpty();
}

QByteArray BSONStreamReader::dequeueDocumentData()
{
    return m_documents.takeFirst();
}
