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

#ifndef XEVEXPORTER_H
#define XEVEXPORTER_H

#include "xmlexporthelpers.h"

class ZipFileSaver;

class XevExporter {
public:
    XevExporter(QDomDocument& doc,
                ZipFileSaver& fileSaver,
                const QString& path,
                const QString& assetsPath = "");

    QDomDocument& doc() const { return mDoc; }

    XevExporter withAssetsPath(const QString& path) const;

    QDomElement createElement(const QString& tagName) const;
    QDomText createTextNode(const QString& data) const;

    using Processor = std::function<void(QIODevice* const dst)>;
    void processAsset(const QString& file, const Processor& func) const;
private:
    QDomDocument& mDoc;
    ZipFileSaver& mFileSaver;
    const QString mPath;
    const QString mAssetsPath;
};

#endif // XEVEXPORTER_H
