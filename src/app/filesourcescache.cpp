#include "filesourcescache.h"
#include <QStringList>

bool isVideoExt(const QString &extension) {
    return extension == "avi" ||
           extension == "mp4" ||
           extension == "mov" ||
           extension == "mkv" ||
           extension == "m4v";
}

bool isSoundExt(const QString &extension) {
    return extension == "mp3" ||
           extension == "wav";
}

bool isVectorExt(const QString &extension) {
    return extension == "svg";
}

bool isImageExt(const QString &extension) {
    return extension == "png" ||
           extension == "jpg";
}

bool isEvExt(const QString &extension) {
    return extension == "ev";
}

bool hasVideoExt(const QString &filename) {
    const auto& extension = filename.split(".").last();
    return isVideoExt(extension);
}

bool hasSoundExt(const QString &filename) {
    const auto& extension = filename.split(".").last();
    return isSoundExt(extension);
}

bool hasVectorExt(const QString &filename) {
    const auto& extension = filename.split(".").last();
    return isVectorExt(extension);
}

bool hasImageExt(const QString &filename) {
    const auto& extension = filename.split(".").last();
    return isImageExt(extension);
}

bool hasAvExt(const QString &filename) {
    const auto& extension = filename.split(".").last();
    return isEvExt(extension);
}
