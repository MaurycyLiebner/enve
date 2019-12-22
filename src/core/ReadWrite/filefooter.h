#ifndef FILEFOOTER_H
#define FILEFOOTER_H
#include <QIODevice>
#include <cstring>

#include "exceptions.h"

class FileFooter {
public:
    static bool sWrite(QIODevice * const target);

    static qint64 sSize(const int evVersion);

    static int sReadEvFileVersion(QIODevice * const src);

    static const int sNewestEvRW;
private:
    static char sEVFormat[15];
    static char sAppName[15];
    static char sAppVersion[15];
};

#endif // FILEFOOTER_H
