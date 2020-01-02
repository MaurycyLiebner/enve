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

#ifndef FILECACHEHANDLER_H
#define FILECACHEHANDLER_H
#include "filedatacachehandler.h"
#include "smartPointers/ememory.h"

class FileCacheHandler : public SelfRef {
    friend class FilesHandler;
    Q_OBJECT
protected:
    FileCacheHandler();

    virtual void afterPathSet(const QString& path) = 0;
    virtual void reload() = 0;
public:
    virtual void replace() = 0;

    void reloadAction() {
        reload();
        emit reloaded();
    }

    const QString& path() const { return mPath; }
    bool fileMissing() const { return mFileMissing; }
signals:
    void pathChanged(const QString& newPath);
    void reloaded();
protected:
    void setPath(const QString& path) {
        if(mPath == path) return;
        mPath = path;
        afterPathSet(path);
        emit pathChanged(path);
    }

    bool mFileMissing = false;
    QString mPath; // filename / dirname
};


#endif // FILECACHEHANDLER_H
