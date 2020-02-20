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

#include "oraimporter.h"

#include "svgimporter.h"
#include "Boxes/textbox.h"
#include "Boxes/paintbox.h"

#include "exceptions.h"
#include "oraparser.h"

void applyAttributesToBox(OraElement& ele, BoundingBox& box) {
    box.prp_setName(ele.fName);
    box.setRelativePos(QPointF(ele.fX, ele.fY));
    box.setOpacity(100*ele.fOpacity);
    box.setVisible(ele.fVisible);
    box.setBlendModeSk(ele.fBlend);
}

qsptr<TextBox> textToBox(OraText& text) {
    const auto result = enve::make_shared<TextBox>();
    result->setCurrentValue(text.fText);
    if(result) applyAttributesToBox(text, *result);
    return result;
}

qsptr<BoundingBox> layerPNGToBox(OraLayerPNG& layer) {
    SkPixmap pixmap;
    if(!layer.fImage || !layer.fImage->peekPixels(&pixmap))
        RuntimeThrow("Decoding png layer failed");
    const auto result = enve::make_shared<PaintBox>();
    const auto surf = result->getSurface()->getCurrentSurface();
    surf->loadPixmap(pixmap);
    if(result) applyAttributesToBox(layer, *result);
    return result;
}

qsptr<BoundingBox> layerSVGToBox(OraLayerSVG& layer,
                                 const GradientCreator& gradientCreator) {
    const auto result = ImportSVG::loadSVGFile(layer.fDocument, gradientCreator);
    if(result) applyAttributesToBox(layer, *result);
    return result;
}

qsptr<ContainerBox> stackToBox(OraStack& stack,
                               const GradientCreator& gradientCreator) {
    const auto result = enve::make_shared<ContainerBox>(eBoxType::layer);
    applyAttributesToBox(stack, *result);
    for(const auto& child : stack.fChildren) {
        qsptr<BoundingBox> childBox;
        const auto type = child->fType;
        switch(type) {
        case OraElementType::stack:
            childBox = stackToBox(static_cast<OraStack&>(*child),
                                  gradientCreator);
            break;
        case OraElementType::text:
            childBox = textToBox(static_cast<OraText&>(*child));
            break;
        case OraElementType::layerPNG:
            childBox = layerPNGToBox(static_cast<OraLayerPNG&>(*child));
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
    const auto oraImg = ImportORA::readOraFile(filename);
    return stackToBox(*oraImg, gradientCreator);
}
