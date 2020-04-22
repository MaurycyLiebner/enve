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

#include "xevexporter.h"

#include "../zipfilesaver.h"

XevExporter::XevExporter(QDomDocument& doc,
                         ZipFileSaver& fileSaver,
                         const QString& path,
                         const QString& assetsPath) :
    mDoc(doc), mFileSaver(fileSaver),
    mPath(path), mAssetsPath(assetsPath) {}

XevExporter XevExporter::withAssetsPath(const QString& path) const {
    return XevExporter(mDoc, mFileSaver, mPath, mAssetsPath + path);
}

QDomElement XevExporter::createElement(const QString& tagName) const {
    return mDoc.createElement(tagName);
}

QDomText XevExporter::createTextNode(const QString& data) const {
    return mDoc.createTextNode(data);
}

void XevExporter::processAsset(const QString& file, const Processor& func) const {
    mFileSaver.process(mPath + "assets/" + mAssetsPath + file, func);
}
