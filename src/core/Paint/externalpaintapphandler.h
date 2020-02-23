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

#ifndef EXTERNALPAINTAPPHANDLER_H
#define EXTERNALPAINTAPPHANDLER_H

#include <QFileSystemWatcher>
#include <QObject>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QSharedPointer>

#include "smartPointers/stdpointer.h"

class AnimatedSurface;
class DrawableAutoTiledSurface;

class ExternalPaintAppHandler : public QObject {
public:
    enum App {
        gimp, mypaint, krita
    };

    ExternalPaintAppHandler(const App app);

    bool checkAppValid() const;
    void setupFor(AnimatedSurface* const surface, const QString &name);

    QString appName() const;
private:
    static void sCreateFileWatcher();
    static void sCreateTmpDir();

    static QSharedPointer<QFileSystemWatcher> sFileWatcher;
    static QSharedPointer<QTemporaryDir> sTmpDir;

    void setupSuccess();

    QString mLayerName;
    QString mPivotName;

    QPoint mInitial00Pos;
    bool mFileExists = false;
    QString mEditFilePath;
    const App mApp;
    QString mExec;
    QSharedPointer<QTemporaryFile> mEditFile;
    AnimatedSurface* mASurface = nullptr;
    stdptr<DrawableAutoTiledSurface> mEditSurface;

};

#endif // EXTERNALPAINTAPPHANDLER_H
