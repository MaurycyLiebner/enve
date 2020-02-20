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

#ifndef ORAPARSER_H
#define ORAPARSER_H

#include <QString>
#include <quazip/quazipfile.h>
#include <QtXml/QDomDocument>

#include "skia/skiaincludes.h"

enum class OraElementType {
    image,
    stack,
    layerPNG,
    layerSVG,
    text
};

struct OraElement {
    OraElement(const QString& name,
               const OraElementType type) :
        fName(name), fType(type) {}

    QString fName;
    int fX = 0;
    int fY = 0;
    qreal fOpacity = 1;
    bool fVisible = true;
    SkBlendMode fBlend = SkBlendMode::kSrcOver;
    const OraElementType fType;
};

struct OraLayer : public OraElement {
    OraLayer(const OraElementType type) :
        OraElement("Layer", type) {}

    QString fSource;
};

struct OraLayerPNG : public OraLayer {
    OraLayerPNG() : OraLayer(OraElementType::layerPNG) {}

    sk_sp<SkImage> fImage;
};

struct OraLayerSVG : public OraLayer {
    OraLayerSVG() : OraLayer(OraElementType::layerSVG) {}

    QDomDocument fDocument;
};

struct OraText : public OraElement {
    OraText() : OraElement("Text", OraElementType::text) {}

    QString fText;
};

struct OraStack : public OraElement {
    OraStack(const QString& name, const OraElementType type) :
        OraElement(name, type) {}
    OraStack() : OraStack("Layer", OraElementType::stack) {}

    QList<std::shared_ptr<OraElement>> fChildren;
};

struct OraImage : public OraStack {
    OraImage() : OraStack("Image", OraElementType::image) {}

    int fWidth;
    int fHeight;
};

namespace ImportORA {
    std::shared_ptr<OraImage> readOraFile(const QString &filename);
}

#endif // ORAPARSER_H
