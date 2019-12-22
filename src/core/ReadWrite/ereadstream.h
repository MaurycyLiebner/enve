#ifndef EREADSTREAM_H
#define EREADSTREAM_H

#include <QIODevice>

#include "efuturepos.h"

class SimpleBrushWrapper;
struct iValueRange;

class eReadFutureTable {
    friend class eReadStream;
    eReadFutureTable(QIODevice* const main);

    bool seek(const eFuturePos& pos) {
        mFutureId = int(pos.fFutureTable) + 1;
        return mMain->seek(pos.fMain);
    }

    eFuturePos readFuturePos() {
        return mFutures.at(mFutureId++);
    }

    void read();
private:
    int mFutureId = 0;
    QList<eFuturePos> mFutures;
    QIODevice* const mMain;
};

class eReadStream {
public:
    eReadStream(const int evFileVersion, QIODevice* const src);
    eReadStream(QIODevice* const src);

    void readFutureTable();

    eFuturePos readFuturePos();

    bool seek(const eFuturePos& pos);

    void readCheckpoint(const QString& errMsg);

    inline qint64 read(void* const data, const qint64 len) {
        return mSrc->read(reinterpret_cast<char*>(data), len);
    }

    eReadStream& operator>>(bool &val);
    eReadStream& operator>>(int &val);
    eReadStream& operator>>(uint& val);
    eReadStream& operator>>(uint64_t& val);
    eReadStream& operator>>(iValueRange& val);
    eReadStream& operator>>(qreal &val);
    eReadStream& operator>>(QPointF &val);
    eReadStream& operator>>(QMatrix &val);
    eReadStream& operator>>(QColor& val);
    eReadStream& operator>>(QString &val);
    eReadStream& operator>>(SimpleBrushWrapper*& brush);

    int evFileVersion() const;
private:
    const int mEvFileVersion;
    QIODevice* const mSrc;
    eReadFutureTable mFutureTable;
};

#endif // EREADSTREAM_H
