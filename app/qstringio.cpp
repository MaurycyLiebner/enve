#include "qstringio.h"

bool readQString(QIODevice* src,
                 QString& targetStr) {
    uint nChars;
    bool err = 1 > src->read(reinterpret_cast<char*>(&nChars),
                             sizeof(uint));
    if(err) return false;
    if(nChars == 0) {
        targetStr = "";
    } else {
        ushort *chars = new ushort[nChars];
        err = 1 > src->read(reinterpret_cast<char*>(chars),
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

bool writeQString(QIODevice* dst,
                  const QString& strToWrite) {
    uint nChars = static_cast<uint>(strToWrite.length());
    bool err = 1 > dst->write(reinterpret_cast<char*>(&nChars), sizeof(uint));
    if(err) return false;
    if(nChars == 0) return true;
    err = 1 > dst->write(reinterpret_cast<const char*>(strToWrite.utf16()),
                         nChars*sizeof(ushort));
    return !err;
}
