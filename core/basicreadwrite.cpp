#include "basicreadwrite.h"
#include "Segments/qcubicsegment1d.h"
#include "Animators/SmartPath/smartpathcontainer.h"

bool gRead(QIODevice* src, QString& targetStr) {
    uint nChars;
    bool err = 1 > src->read(rcChar(&nChars),
                             sizeof(uint));
    if(err) return false;
    if(nChars == 0) {
        targetStr = "";
    } else {
        ushort *chars = new ushort[nChars];
        err = 1 > src->read(rcChar(chars),
                            nChars*sizeof(ushort));
        if(err) {
            targetStr = "";
        } else {
            targetStr = QString::fromUtf16(chars, static_cast<int>(nChars));
        }
        delete[] chars;
    }
    return err;
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

bool gWrite(QIODevice *dst, const bool &value) {
    return dst->write(rcConstChar(&value), sizeof(bool)) > 0;
}

bool gRead(QIODevice *src, qCubicSegment1D &value) {
    return src->read(rcChar(&value), sizeof(qCubicSegment1D)) > 0;
}

bool gWrite(QIODevice *dst, const qCubicSegment1D &value) {
    return dst->write(rcConstChar(&value), sizeof(qCubicSegment1D)) > 0;
}

bool gRead(QIODevice *src, SmartPath &value) {
    value.read(src);
}

bool gWrite(QIODevice *dst, const SmartPath &value) {
    value.write(dst);
}
