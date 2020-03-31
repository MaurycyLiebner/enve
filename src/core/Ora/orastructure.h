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

#ifndef ORASTRUCTURE_H
#define ORASTRUCTURE_H

#include <QString>
#include <QImage>

#include "../skia/skiaincludes.h"
#include "../core_global.h"

enum class OraElementType {
    image,
    stack,
    layerPNG,
    layerSVG,
    text
};

struct CORE_EXPORT OraElement {
    OraElement(const QString& name,
               const OraElementType type) :
        fName(name), fType(type) {}

    QString fName;
    int fX = 0;
    int fY = 0;
    qreal fOpacity = 1;
    bool fLocked = false;
    bool fVisible = true;
    bool fSelected = false;
    SkBlendMode fBlend = SkBlendMode::kSrcOver;
    const OraElementType fType;
};

struct CORE_EXPORT OraLayer : public OraElement {
    OraLayer(const OraElementType type) :
        OraElement("Layer", type) {}

    QString fSource;
};

//#define OraLayerPNG_SkImage

template <typename T>
struct OraLayerPNG : public OraLayer {
    OraLayerPNG() : OraLayer(OraElementType::layerPNG) {}
    T fImage;
};

using OraLayerPNG_Qt = OraLayerPNG<QImage>;
using OraLayerPNG_Sk = OraLayerPNG<sk_sp<SkImage>>;

struct CORE_EXPORT OraLayerSVG : public OraLayer {
    OraLayerSVG() : OraLayer(OraElementType::layerSVG) {}

    QByteArray fDocument;
};

struct CORE_EXPORT OraText : public OraElement {
    OraText() : OraElement("Text", OraElementType::text) {}

    QString fText;
};

template <typename OraLayerPNG_XX>
struct OraStack : public OraElement {
    OraStack(const QString& name, const OraElementType type) :
        OraElement(name, type) {}
    OraStack() : OraStack("Layer", OraElementType::stack) {}

    using Finder = std::function<bool(OraElement&)>;
    //! @brief Break after finder returns true
    void findChildren(const Finder& finder) {
        for(const auto& child : fChildren) {
            if(finder(*child)) return;
        }
    }

    QList<std::shared_ptr<OraElement>> fChildren;
};

using OraStack_Qt = OraStack<OraLayerPNG_Qt>;
using OraStack_Sk = OraStack<OraLayerPNG_Sk>;

template <typename OraLayerPNG_XX>
struct OraImage : public OraStack<OraLayerPNG_XX> {
    OraImage() : OraStack<OraLayerPNG_XX>("Image", OraElementType::image) {}

    int fWidth;
    int fHeight;
};

using OraImage_Qt = OraImage<OraLayerPNG_Qt>;
using OraImage_Sk = OraImage<OraLayerPNG_Sk>;

#endif // ORASTRUCTURE_H
