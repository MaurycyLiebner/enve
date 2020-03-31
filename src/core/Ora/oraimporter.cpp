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

#include "Ora/oraimporter.h"

#include "svgimporter.h"
#include "Boxes/textbox.h"
#include "Boxes/paintbox.h"

#include "exceptions.h"
#include "oraparser.h"

#include <QSvgRenderer>

void applyAttributesToBox(OraElement& ele, BoundingBox& box) {
    box.prp_setName(ele.fName);
    box.setRelativePos(QPointF(ele.fX, ele.fY));
    box.setOpacity(100*ele.fOpacity);
    box.setVisible(ele.fVisible);
    box.setLocked(ele.fLocked);
    box.setBlendModeSk(ele.fBlend);
}

qsptr<TextBox> textToBox(OraText& text) {
    const auto result = enve::make_shared<TextBox>();
    result->setCurrentValue(text.fText);
    if(result) applyAttributesToBox(text, *result);
    return result;
}

qsptr<BoundingBox> layerPNGToBox(OraLayerPNG_Qt& layer) {
    const auto result = enve::make_shared<PaintBox>();
    const auto surf = result->getSurface()->getCurrentSurface();
    if(layer.fImage.isNull()) RuntimeThrow("Decoding png layer failed");
    surf->loadPixmap(layer.fImage);
    if(result) applyAttributesToBox(layer, *result);
    return result;
}

qsptr<BoundingBox> layerSVGToBox(OraLayerSVG& layer,
                                 const GradientCreator& gradientCreator) {
    const auto result = ImportSVG::loadSVGFile(layer.fDocument, gradientCreator);
    if(result) applyAttributesToBox(layer, *result);
    return result;
}

qsptr<ContainerBox> stackToBox(OraStack_Qt& stack,
                               const GradientCreator& gradientCreator) {
    const auto result = enve::make_shared<ContainerBox>(eBoxType::layer);
    applyAttributesToBox(stack, *result);
    for(const auto& child : stack.fChildren) {
        qsptr<BoundingBox> childBox;
        const auto type = child->fType;
        switch(type) {
        case OraElementType::stack:
            childBox = stackToBox(static_cast<OraStack_Qt&>(*child),
                                  gradientCreator);
            break;
        case OraElementType::text:
            childBox = textToBox(static_cast<OraText&>(*child));
            break;
        case OraElementType::layerPNG:
            childBox = layerPNGToBox(static_cast<OraLayerPNG_Qt&>(*child));
            break;
        case OraElementType::layerSVG:
            childBox = layerSVGToBox(static_cast<OraLayerSVG&>(*child),
                                     gradientCreator);
            break;
        default: break;
        }
        if(childBox) {
            const int insertId = result->getContainedBoxesCount();
            result->insertContained(insertId, childBox);
        }
    }
    return result;
}

qsptr<ContainerBox> ImportORA::loadORAFile(
        const QString &filename, const GradientCreator& gradientCreator) {
    const auto oraImg = ImportORA::readOraFileQImage(filename);
    return stackToBox(*oraImg, gradientCreator);
}

void setupPaint(OraElement& element, SkPaint& paint) {
    paint.setBlendMode(element.fBlend);
    paint.setAlphaf(element.fOpacity);
}

void drawLayerPNG(OraLayerPNG_Sk& layer, SkCanvas& canvas) {
    SkPaint paint;
    setupPaint(layer, paint);
    canvas.drawImage(layer.fImage, layer.fX, layer.fY, &paint);
}

void drawLayerSVG(OraLayerSVG& layer, SkCanvas& canvas) {
    SkPaint paint;
    setupPaint(layer, paint);

    // render svg
    QSvgRenderer renderer(layer.fDocument);
    QImage qImg(renderer.defaultSize(), QImage::Format_RGBA8888_Premultiplied);
    QPainter p(&qImg);
    renderer.render(&p);
    p.end();

    canvas.drawImage(toSkImage(qImg), layer.fX, layer.fY, &paint);
}

void drawStack(OraStack_Sk& stack, SkCanvas& canvas) {
    for(int i = stack.fChildren.count() - 1; i >= 0; i--) {
        const auto& child = stack.fChildren.at(i);
        if(!child->fVisible) continue;
        switch(child->fType) {
        case OraElementType::stack:
            drawStack(static_cast<OraStack_Sk&>(*child), canvas);
            break;
        case OraElementType::layerPNG:
            drawLayerPNG(static_cast<OraLayerPNG_Sk&>(*child), canvas);
            break;
        case OraElementType::layerSVG:
            drawLayerSVG(static_cast<OraLayerSVG&>(*child), canvas);
            break;
        default: break;
        }
    }
}

sk_sp<SkImage> ImportORA::loadMergedORAFile(const QString &filename,
                                            const bool useContained) {
    if(useContained) {
        try {
            const auto result = loadContainedMerged(filename);
            return result;
        } catch(...) {}
    }
    const auto oraImg = ImportORA::readOraFileSkImage(filename);
    const int width = oraImg->fWidth;
    const int height = oraImg->fHeight;
    const auto info = SkiaHelpers::getPremulRGBAInfo(width, height);

    SkBitmap bitmap;
    bitmap.allocPixels(info);
    bitmap.eraseColor(SK_ColorTRANSPARENT);
    SkCanvas canvas(bitmap);
    drawStack(*oraImg, canvas);
    return SkiaHelpers::transferDataToSkImage(bitmap);
}
