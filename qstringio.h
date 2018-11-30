#ifndef QSTRINGIO_H
#define QSTRINGIO_H
#include <QString>
#include <QIODevice>

extern bool readQString(QIODevice* src,
                        QString& targetStr);

extern bool writeQString(QIODevice* dst,
                         const QString& strToWrite);

#endif // QSTRINGIO_H
