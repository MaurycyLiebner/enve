#ifndef BASICREADWRITE_H
#define BASICREADWRITE_H
#include <QString>
#include <QIODevice>
#include "castmacros.h"
struct qCubicSegment1D;
class SmartPath;

extern bool gRead(QIODevice* src, QString& targetStr);
extern bool gWrite(QIODevice* dst, const QString& strToWrite);

extern bool gRead(QIODevice* src, bool& value);
extern bool gWrite(QIODevice* dst, const bool& value);

extern bool gRead(QIODevice* src, qCubicSegment1D& value);
extern bool gWrite(QIODevice* dst, const qCubicSegment1D& value);

extern bool gRead(QIODevice* src, SmartPath& value);
extern bool gWrite(QIODevice* dst, const SmartPath& value);
#endif // BASICREADWRITE_H
