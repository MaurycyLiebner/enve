// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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

#ifndef FILESOURCESCACHE_H
#define FILESOURCESCACHE_H
#include <QStringList>

bool hasVideoExt(const QString &filename);
bool hasSoundExt(const QString &filename);
bool hasVectorExt(const QString &filename);
bool hasImageExt(const QString &filename);
bool hasEvExt(const QString &filename);

bool isVideoExt(const QString &extension);
bool isSoundExt(const QString &extension);
bool isVectorExt(const QString &extension);
bool isImageExt(const QString &extension);
bool isEvExt(const QString &extension);

bool isLayersExt(const QString& ext);

namespace FileExtensions {
    extern QStringList image;
    extern QStringList sound;
    extern QStringList video;
    extern QStringList layers;

    QString filters(const QStringList& exts);

    QString imageFilters();
    QString soundFilters();
    QString videoFilters();
    QString layersFilters();
};

#endif // FILESOURCESCACHE_H
