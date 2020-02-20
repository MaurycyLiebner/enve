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

#include <quazip/quazipfile.h>
#include <QtXml/QDomDocument>
#include "skia/skiaincludes.h"
#include "svgimporter.h"
#include "Boxes/textbox.h"
#include "Boxes/paintbox.h"

#include "exceptions.h"

enum class OraElementType {
    image, stack, layer, text
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
    OraLayer() : OraElement("Layer", OraElementType::layer) {}

    QString fSource;
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

SkBlendMode compositeOpToBlendMode(const QString& compOpStr) {
    if(compOpStr.isEmpty()) return SkBlendMode::kSrcOver;
    if(compOpStr == "svg:src-over") {
        return SkBlendMode::kSrcOver;
    } else if(compOpStr == "svg:multiply") {
        return SkBlendMode::kMultiply;
    } else if(compOpStr == "svg:screen") {
        return SkBlendMode::kScreen;
    } else if(compOpStr == "svg:overlay") {
        return SkBlendMode::kOverlay;
    } else if(compOpStr == "svg:darken") {
        return SkBlendMode::kDarken;
    } else if(compOpStr == "svg:lighten") {
        return SkBlendMode::kLighten;
    } else if(compOpStr == "svg:color-dodge") {
        return SkBlendMode::kColorDodge;
    } else if(compOpStr == "svg:color-burn") {
        return SkBlendMode::kColorBurn;
    } else if(compOpStr == "svg:hard-light") {
        return SkBlendMode::kHardLight;
    } else if(compOpStr == "svg:soft-light") {
        return SkBlendMode::kSoftLight;
    } else if(compOpStr == "svg:difference") {
        return SkBlendMode::kDifference;
    } else if(compOpStr == "svg:color") {
        return SkBlendMode::kColor;
    } else if(compOpStr == "svg:luminosity") {
        return SkBlendMode::kLuminosity;
    } else if(compOpStr == "svg:hue") {
        return SkBlendMode::kHue;
    } else if(compOpStr == "svg:saturation") {
        return SkBlendMode::kSaturation;
    } else if(compOpStr == "svg:plus") {
        return SkBlendMode::kPlus;
    } else if(compOpStr == "svg:dst-in") {
        return SkBlendMode::kDstIn;
    } else if(compOpStr == "svg:dst-out") {
        return SkBlendMode::kDstOut;
    } else if(compOpStr == "svg:src-atop") {
        return SkBlendMode::kSrcATop;
    } else if(compOpStr == "svg:dst-atop") {
        return SkBlendMode::kDstATop;
    } else return SkBlendMode::kSrcOver;
}

void parseOraElementAttributes(OraElement& oraEle,
                               const QDomElement &domEle) {
    const QString name = domEle.attribute("name");
    if(!name.isEmpty()) oraEle.fName = name;
    const QString xStr = domEle.attribute("x");
    if(!xStr.isEmpty()) {
        bool ok;
        const int x = xStr.toInt(&ok);
        if(ok) oraEle.fX = x;
    }
    const QString yStr = domEle.attribute("y");
    if(!yStr.isEmpty()) {
        bool ok;
        const int y = yStr.toInt(&ok);
        if(ok) oraEle.fY = y;
    }
    const QString opaStr = domEle.attribute("opacity");
    if(!opaStr.isEmpty()) {
        bool ok;
        const qreal opa = opaStr.toDouble(&ok);
        if(ok) oraEle.fOpacity = opa;
    }
    const QString visStr = domEle.attribute("visibility");
    if(visStr == "visible" || visStr == "hidden") {
        oraEle.fVisible = visStr == "visible";
    }
    const QString compOpStr = domEle.attribute("composite-op");
    oraEle.fBlend = compositeOpToBlendMode(compOpStr);
}

std::shared_ptr<OraStack> parseStack(const QDomElement &element);

std::shared_ptr<OraLayer> parseLayer(const QDomElement &element) {
    auto result = std::make_unique<OraLayer>();
    parseOraElementAttributes(*result, element);
    result->fSource = element.attribute("src");
    return result;
}

std::shared_ptr<OraText> parseText(const QDomElement &element) {
    auto result = std::make_unique<OraText>();
    parseOraElementAttributes(*result, element);
    result->fText = element.text();
    return result;
}

void parseStackContent(OraStack& stack, const QDomElement &element) {
    const auto childNodes = element.childNodes();
    for(int i = 0; i < childNodes.count(); i++) {
        const QDomNode iNode = childNodes.at(i);
        if(!iNode.isElement()) continue;
        const QDomElement iElement = iNode.toElement();
        const QString tagName = iElement.tagName();
        std::shared_ptr<OraElement> child;
        if(tagName == "stack") {
            child = parseStack(iElement);
        } else if(tagName == "layer") {
            child = parseLayer(iElement);
        } else if(tagName == "text") {
            child = parseText(iElement);
        } else RuntimeThrow("Unrecognized type " + tagName);
        stack.fChildren.push_back(std::move(child));
    }
}

std::shared_ptr<OraStack> parseStack(const QDomElement &element) {
    auto result = std::make_unique<OraStack>();
    parseOraElementAttributes(*result, element);
    parseStackContent(*result, element);
    return result;
}

std::shared_ptr<OraImage> parseStackXml(QIODevice* const src) {
    QDomDocument document;
    if(!document.setContent(src)) RuntimeThrow("Could not parse content");
    const QDomElement rootElement = document.firstChildElement("image");
    if(rootElement.isNull()) RuntimeThrow("Missing root (image) element");
    auto result = std::make_unique<OraImage>();
    const QString wStr = rootElement.attribute("w");
    if(!wStr.isEmpty()) {
        bool ok;
        const int w = wStr.toInt(&ok);
        if(ok) result->fWidth = w;
    }
    const QString hStr = rootElement.attribute("h");
    if(!hStr.isEmpty()) {
        bool ok;
        const int h = hStr.toInt(&ok);
        if(ok) result->fHeight = h;
    }
    parseStackContent(*result, rootElement);
    return result;
}

class OraFileProcessor {
public:
    OraFileProcessor(QuaZip& zip, QuaZipFile& file) :
        mZip(zip), mFile(file) {}

    using Processor = std::function<void(QIODevice* const src)>;
    void process(const QString& file, const bool text, const Processor& func) {
        if(!mZip.setCurrentFile(file))
            RuntimeThrow("No stack.xml found in " + mZip.getZipName());
        QIODevice::OpenMode openMode = QIODevice::ReadOnly;
        if(text) openMode |= QIODevice::Text;
        if(!mFile.open(openMode))
            RuntimeThrow("Could not open stack.xml from " + mZip.getZipName());
        try {
            func(&mFile);
        } catch(...) {
            mFile.close();
            RuntimeThrow("Could not parse stack.xml from " + mZip.getZipName());
        }
        mFile.close();
    }
private:
    QuaZip& mZip;
    QuaZipFile& mFile;
};

std::shared_ptr<OraImage> readStackXml(OraFileProcessor& fileProcessor) {
    std::shared_ptr<OraImage> result;
    fileProcessor.process("stack.xml", true, [&result](QIODevice* const src) {
        result = parseStackXml(src);
    });
    return result;
}

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

qsptr<BoundingBox> layerToBox(OraLayer& layer, OraFileProcessor& fileProcessor,
                              const GradientCreator& gradientCreator) {
    const auto ext = layer.fSource.right(3);
    qsptr<BoundingBox> result;
    if(ext == "svg") {
        fileProcessor.process(layer.fSource, true, [&](QIODevice* const src) {
            result = loadSVGFile(src, gradientCreator);
        });
    } else if(ext == "png") {
        fileProcessor.process(layer.fSource, false, [&result](QIODevice* const src) {
            const auto qData = src->readAll();
            const sk_sp<SkData> data = SkData::MakeWithoutCopy(qData.data(), qData.size());
            const auto image = SkImage::DecodeToRaster(data);
            SkPixmap pixmap;
            if(!image || !image->peekPixels(&pixmap))
                RuntimeThrow("Decoding png layer failed");
            const auto paintBox = enve::make_shared<PaintBox>();
            const auto surf = paintBox->getSurface()->getCurrentSurface();
            surf->loadPixmap(pixmap);
            result = paintBox;
        });
    }
    if(result) applyAttributesToBox(layer, *result);
    return result;
}

qsptr<ContainerBox> stackToBox(OraStack& stack, OraFileProcessor& fileProcessor,
                               const GradientCreator& gradientCreator) {
    const auto result = enve::make_shared<ContainerBox>(eBoxType::layer);
    applyAttributesToBox(stack, *result);
    for(const auto& child : stack.fChildren) {
        qsptr<BoundingBox> childBox;
        const auto type = child->fType;
        switch(type) {
        case OraElementType::stack:
            childBox = stackToBox(*static_cast<OraStack*>(child.get()),
                                  fileProcessor, gradientCreator);
            break;
        case OraElementType::text:
            childBox = textToBox(*static_cast<OraText*>(child.get()));
            break;
        case OraElementType::layer:
            childBox = layerToBox(*static_cast<OraLayer*>(child.get()),
                                  fileProcessor, gradientCreator);
            break;
        default: break;
        }
        if(childBox) result->insertContained(0, childBox);
    }
    return result;
}

qsptr<ContainerBox> loadORAFile(const QString &filename,
                                const GradientCreator& gradientCreator) {
    QuaZip zip(filename);
    if(!zip.open(QuaZip::mdUnzip))
        RuntimeThrow("Could not open " + filename);
    QuaZipFile file(&zip);

    OraFileProcessor fileProcessor(zip, file);
    const auto oraImg = readStackXml(fileProcessor);
    return stackToBox(*oraImg, fileProcessor, gradientCreator);
}
