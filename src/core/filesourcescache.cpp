#include "filesourcescache.h"
#include <QStringList>

const QStringList videoExts{"avi", "mp4", "mov",
                            "mkv", "m4v", "webm",
                            "flv", "vob", "wmv",
                            "3gp", "rmvb", "gif",
                            "mpeg", "mpg", "mp2",
                            "mpv"};
bool isVideoExt(const QString &extension) {
    return videoExts.contains(extension.toLower());
}

const QStringList audioExts{"mp3", "wav", "aiff",
                            "flac", "m4a", "oga",
                            "gsm"};
bool isSoundExt(const QString &extension) {
    return audioExts.contains(extension.toLower());
}

bool isVectorExt(const QString &extension) {
    return extension.toLower() == "svg";
}

const QStringList imageExts{"png", "jpg", "tiff",
                            "tif", "jpeg", "bmp",
                            "gsm"};
bool isImageExt(const QString &extension) {
    return imageExts.contains(extension.toLower());
}

bool isEvExt(const QString &extension) {
    return extension.toLower() == "ev";
}

bool hasVideoExt(const QString &filename) {
    return isVideoExt(filename.split(".").last());
}

bool hasSoundExt(const QString &filename) {
    return isSoundExt(filename.split(".").last());
}

bool hasVectorExt(const QString &filename) {
    return isVectorExt(filename.split(".").last());
}

bool hasImageExt(const QString &filename) {
    return isImageExt(filename.split(".").last());
}

bool hasAvExt(const QString &filename) {
    return isEvExt(filename.split(".").last());
}
