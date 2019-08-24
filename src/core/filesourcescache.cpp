// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
