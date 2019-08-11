#ifndef BASICREADWRITE_H
#define BASICREADWRITE_H
#include <QString>
#include <QIODevice>
#include "castmacros.h"
struct qCubicSegment1D;
class SmartPath;
class BrushPolyline;
struct AutoTiledSurface;

class ReadSrc {
public:
    ReadSrc(QIODevice* const src) : mSrc(src) {

    }

    void read(void* const data, const qint64& size) {
        mSrc->read(rcChar(data), size);
    }
private:
    QIODevice* const mSrc;
};

class WriteDst : public QIODevice {
public:
    WriteDst(QIODevice* const dst) : mDst(dst) {

    }

    void write(const void* const data, const qint64& size) {
        mDst->write(rcConstChar(data), size);
    }
private:
    QIODevice* const mDst;
};

extern QString gReadString(QIODevice * src);
extern bool gReadBool(QIODevice * src);

extern bool gRead(QIODevice* src, QString& targetStr);
extern bool gWrite(QIODevice* dst, const QString& strToWrite);

extern bool gRead(QIODevice* src, bool& value);
extern bool gWrite(QIODevice* dst, const bool value);

extern bool gRead(QIODevice* src, qCubicSegment1D& value);
extern bool gWrite(QIODevice* dst, const qCubicSegment1D& value);

extern bool gRead(QIODevice* src, SmartPath& value);
extern bool gWrite(QIODevice* dst, const SmartPath& value);
#endif // BASICREADWRITE_H
