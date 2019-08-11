#include "basicreadwrite.h"

QString gReadString(QIODevice *src) {
    QString result;
    uint nChars;
    src->read(rcChar(&nChars), sizeof(uint));
    if(nChars == 0) result = "";
    else {
        ushort * const chars = new ushort[nChars];
        src->read(rcChar(chars), nChars*sizeof(ushort));
        result = QString::fromUtf16(chars, static_cast<int>(nChars));
        delete[] chars;
    }
    return result;
}

bool gReadBool(QIODevice * src) {
    bool result;
    gRead(src, result);
    return result;
}

bool gRead(QIODevice* src, QString& targetStr) {
    targetStr = gReadString(src);
    return true;
}

bool gWrite(QIODevice* dst, const QString& strToWrite) {
    uint nChars = static_cast<uint>(strToWrite.length());
    bool err = 1 > dst->write(rcChar(&nChars), sizeof(uint));
    if(err) return false;
    if(nChars == 0) return true;
    err = 1 > dst->write(rcConstChar(strToWrite.utf16()),
                         nChars*sizeof(ushort));
    return !err;
}

bool gRead(QIODevice *src, bool &value) {
    return src->read(rcChar(&value), sizeof(bool)) > 0;
}

bool gWrite(QIODevice *dst, const bool value) {
    return dst->write(rcConstChar(&value), sizeof(bool)) > 0;
}

#include "Segments/qcubicsegment1d.h"
bool gRead(QIODevice *src, qCubicSegment1D &value) {
    return src->read(rcChar(&value), sizeof(qCubicSegment1D)) > 0;
}

bool gWrite(QIODevice *dst, const qCubicSegment1D &value) {
    return dst->write(rcConstChar(&value), sizeof(qCubicSegment1D)) > 0;
}

#include "Animators/SmartPath/smartpath.h"
bool gRead(QIODevice *src, SmartPath &value) {
    return value.read(src);
}

bool gWrite(QIODevice *dst, const SmartPath &value) {
    return value.write(dst);
}
