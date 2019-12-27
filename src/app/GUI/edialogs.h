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

#ifndef EDIALOGS_H
#define EDIALOGS_H
#include <QString>

namespace eDialogs {
    QString openFile(const QString& title,
                     const QString& path,
                     const QString& filter);
    QStringList openFiles(const QString& title,
                          const QString& path,
                          const QString& filter);
    QString openDir(const QString& title,
                    const QString& path);
    QString saveFile(const QString& title,
                     const QString& path,
                     const QString& filter);
};

#endif // EDIALOGS_H
