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

#include "oracreator.h"

#include "../zipfilesaver.h"
#include "XML/xmlexporthelpers.h"

#include <QtXml/QDomDocument>
#include <QPainter>

void writeMimeType(ZipFileSaver& fileSaver) {
    fileSaver.processText("mimetype", [](QTextStream& stream) {
        stream << "image/openraster";
    }, false);
}

void saveOraElementAttributes(const OraElement& ele,
                              QDomElement& xmlEle) {
    xmlEle.setAttribute("name", ele.fName);
    xmlEle.setAttribute("x", ele.fX);
    xmlEle.setAttribute("y", ele.fY);
    xmlEle.setAttribute("opacity", ele.fOpacity);
    xmlEle.setAttribute("visibility", ele.fVisible ? "visible" : "hidden");
    xmlEle.setAttribute("edit-locked", ele.fLocked ? "true" : "false");
    const QString compositeOp = XmlExportHelpers::blendModeToString(ele.fBlend);
    xmlEle.setAttribute("composite-op", compositeOp);
    xmlEle.setAttribute("selected", ele.fSelected ? "true" : "false");
}

void addText(const OraText& text, QDomElement& dst,
             QDomDocument& doc) {
    QDomElement textEle = doc.createElement("text");
    saveOraElementAttributes(text, textEle);
    textEle.appendChild(doc.createTextNode(text.fText));
    dst.appendChild(textEle);
}

void addLayer(const OraLayer& layer, QDomElement& dst,
              QDomDocument& doc) {
    QDomElement layerEle = doc.createElement("layer");
    saveOraElementAttributes(layer, layerEle);
    layerEle.setAttribute("src", layer.fSource);
    dst.appendChild(layerEle);
}

void addStack(const OraStack_Qt& stack, QDomElement& dst,
              QDomDocument& doc);

void addStackContent(const OraStack_Qt& stack, QDomElement& stackEle,
                     QDomDocument& doc) {
    for(const auto& child : stack.fChildren) {
        const auto type = child->fType;
        switch(type) {
        case OraElementType::stack:
            addStack(static_cast<const OraStack_Qt&>(*child), stackEle, doc);
            break;
        case OraElementType::text:
            addText(static_cast<const OraText&>(*child), stackEle, doc);
            break;
        case OraElementType::layerPNG:
        case OraElementType::layerSVG:
            addLayer(static_cast<const OraLayer&>(*child), stackEle, doc);
            break;
        default: break;
        }
    }
}

void addStack(const OraStack_Qt& stack, QDomElement& dst,
              QDomDocument& doc) {
    QDomElement stackEle = doc.createElement("stack");
    saveOraElementAttributes(stack, stackEle);
    addStackContent(stack, stackEle, doc);
    dst.appendChild(stackEle);
}

void saveLayerFilePNG(const OraLayerPNG_Qt &layer, ZipFileSaver& fileSaver) {
    fileSaver.process(layer.fSource, [&layer](QIODevice* const dst) {
        layer.fImage.save(dst, "PNG");
    });
}

void saveLayerFileSVG(const OraLayerSVG &layer, ZipFileSaver& fileSaver) {
    fileSaver.process(layer.fSource, [&layer](QIODevice* const dst) {
        dst->write(layer.fDocument);
    });
}

void saveStackSourceFiles(const OraStack_Qt &stack, ZipFileSaver& fileSaver) {
    for(const auto& child : stack.fChildren) {
        const auto type = child->fType;
        switch(type) {
        case OraElementType::stack:
            saveStackSourceFiles(static_cast<const OraStack_Qt&>(*child),
                                 fileSaver);
            break;
        case OraElementType::layerPNG:
            saveLayerFilePNG(static_cast<const OraLayerPNG_Qt&>(*child), fileSaver);
            break;
        case OraElementType::layerSVG:
            saveLayerFileSVG(static_cast<const OraLayerSVG&>(*child), fileSaver);
            break;
        default: break;
        }
    }
}

void drawStack(const OraStack_Qt &stack, QPainter& p) {
    const auto& children = stack.fChildren;
    for(int i = children.count() - 1; i >= 0; i--) {
        const auto& child = children.at(i);
        const auto type = child->fType;
        switch(type) {
        case OraElementType::stack:
            drawStack(static_cast<const OraStack_Qt&>(*child), p);
            break;
        case OraElementType::layerPNG: {
            const auto layer = static_cast<const OraLayerPNG_Qt&>(*child);
            //p.setCompositionMode();
            p.drawImage(layer.fX, layer.fY, layer.fImage);
        } break;
        case OraElementType::layerSVG:
            break;
        default: break;
        }
    }
}

QImage mergedImage(const OraImage_Qt &ora) {
    QImage img(ora.fWidth, ora.fHeight, QImage::Format_RGBA64_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p(&img);
    drawStack(ora, p);
    p.end();
    return img;
}

void save(ZipFileSaver &fileSaver, const OraImage_Qt &ora,
          const bool mergeImages) {
    writeMimeType(fileSaver);

    QDomDocument doc("stack.xml");

    QDomElement imageEle = doc.createElement("image");
    imageEle.setAttribute("w", ora.fWidth);
    imageEle.setAttribute("h", ora.fHeight);

    addStackContent(ora, imageEle, doc);

    doc.appendChild(imageEle);

    fileSaver.processText("stack.xml", [&doc](QTextStream& stream) {
        stream << doc.toString();
    });
    saveStackSourceFiles(ora, fileSaver);

    if(mergeImages) {
        const QImage img = mergedImage(ora);
        fileSaver.process("mergedimage.png", [&img](QIODevice* const dst) {
            img.save(dst, "PNG");
        });
        fileSaver.process("Thumbnails/thumbnail.png", [&img](QIODevice* const dst) {
            img.scaled(256, 256, Qt::KeepAspectRatio).save(dst, "PNG");
        });
    }
}

void CreatorOra::save(QIODevice* const dst, const OraImage_Qt &ora,
                      const bool mergeImages) {
    ZipFileSaver fileSaver;
    fileSaver.setIoDevice(dst);
    save(fileSaver, ora, mergeImages);
}

void CreatorOra::save(const QString &path, const OraImage_Qt &ora,
                      const bool mergeImages) {
    ZipFileSaver fileSaver;
    fileSaver.setZipPath(path);
    save(fileSaver, ora, mergeImages);
}
