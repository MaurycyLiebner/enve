#ifndef FILESOURCESCACHE_H
#define FILESOURCESCACHE_H
#include <QString>

bool hasVideoExt(const QString &filename);
bool hasSoundExt(const QString &filename);
bool hasVectorExt(const QString &filename);
bool hasImageExt(const QString &filename);
bool hasAvExt(const QString &filename);

bool isVideoExt(const QString &extension);
bool isSoundExt(const QString &extension);
bool isVectorExt(const QString &extension);
bool isImageExt(const QString &extension);
bool isEvExt(const QString &extension);

#endif // FILESOURCESCACHE_H
