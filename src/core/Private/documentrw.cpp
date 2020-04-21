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

#include "Private/document.h"

#include "ReadWrite/basicreadwrite.h"
#include "ReadWrite/xevformat.h"
#include "XML/xmlexporthelpers.h"
#include "Animators/gradient.h"
#include "Paint/brushescontext.h"
#include "simpletask.h"
#include "canvas.h"

void Document::writeBookmarked(eWriteStream &dst) const {
    dst << fColors.count();
    for(const auto &col : fColors) {
        dst << col;
    }

    dst << fBrushes.count();
    for(const auto &brush : fBrushes) {
        dst << brush;
    }
}

void Document::writeScenes(eWriteStream &dst) const {
    const int nScenes = fScenes.count();
    dst.write(&nScenes, sizeof(int));
    for(const auto &scene : fScenes) {
        scene->writeBoundingBox(dst);
        dst.writeCheckpoint();
    }
}

void Document::write(eWriteStream& dst) const {
    writeBookmarked(dst);
    dst.writeCheckpoint();
    writeScenes(dst);

//        if(canvas.get() == mCurrentCanvas) {
//            currentCanvasId = mCurrentCanvas->getWriteId();
//        }
//    }
//    target->write(rcConstChar(&currentCanvasId), sizeof(int));
}

void Document::readBookmarked(eReadStream &src) {
    int nCol; src >> nCol;
    for(int i = 0; i < nCol; i++) {
        QColor col; src >> col;
        addBookmarkColor(col);
    }

    int nBrush; src >> nBrush;
    for(int i = 0; i < nBrush; i++) {
        SimpleBrushWrapper* brush; src >> brush;
        if(brush) addBookmarkBrush(brush);
    }
}

void Document::readGradients(eReadStream& src) {
    int nGrads; src >> nGrads;
    for(int i = 0; i < nGrads; i++) {
        enve::make_shared<SceneBoundGradient>(nullptr)->read(src);
    }
}

void Document::readScenes(eReadStream& src) {
    int nScenes;
    src.read(&nScenes, sizeof(int));
    for(int i = 0; i < nScenes; i++) {
        const auto scene = createNewScene();
        const auto block = scene->blockUndoRedo();
        scene->readBoundingBox(src);
        src.readCheckpoint("Error reading scene");
    }
}

void Document::read(eReadStream& src) {
    if(src.evFileVersion() > 1) {
        readBookmarked(src);
        src.readCheckpoint("Error reading bookmarks");
    }
    if(src.evFileVersion() <= 5) {
        readGradients(src);
        src.readCheckpoint("Error reading gradients");
    }
    readScenes(src);
    SimpleTask::sProcessAll();
}

void Document::writeDoxumentXEV(QDomDocument& doc) const {
    auto document = doc.createElement("Document");
    document.setAttribute("format-version", XevFormat::version);

    auto bColors = doc.createElement("ColorBookmarks");
    for(const auto &col : fColors) {
        auto color = doc.createElement("Color");
        color.setAttribute("name", col.name());
        bColors.appendChild(color);
    }
    document.appendChild(bColors);

    auto bBrushes = doc.createElement("BrushBookmarks");
    for(const auto &b : fBrushes) {
        auto brush = doc.createElement("Brush");
        brush.setAttribute("collection", b->getCollectionName());
        brush.setAttribute("name", b->getBrushName());
        bBrushes.appendChild(brush);
    }
    document.appendChild(bBrushes);

    auto scenes = doc.createElement("Scenes");
    for(const auto &s : fScenes) {
        auto scene = doc.createElement("Scene");
        scene.setAttribute("name", s->prp_getName());
        scene.setAttribute("frame", s->getCurrentFrame());
        scene.setAttribute("width", s->getCanvasWidth());
        scene.setAttribute("height", s->getCanvasHeight());
        scene.setAttribute("fps", s->getFps());

        scenes.appendChild(scene);
    }
    document.appendChild(scenes);

    doc.appendChild(document);
}

void Document::writeScenesXEV(ZipFileSaver& fileSaver) const {
    int id = 0;
    for(const auto &s : fScenes) {
        const QString path = "scenes/" + QString::number(id++) + "/";
        s->writeBoxOrSoundXEV(fileSaver, path);
    }
}

void Document::writeXEV(ZipFileSaver& fileSaver) const {
    fileSaver.processText("document.xml", [&](QTextStream& stream) {
        QDomDocument document;
        writeDoxumentXEV(document);
        stream << document.toString();
    });
    writeScenesXEV(fileSaver);
}

void Document::readDoxumentXEV(const QDomDocument& doc,
                               QList<SceneSettingsXEV>& sceneSetts) {
    const auto document = XmlExportHelpers::getOnlyElement(doc, "Document");
    const QString versionStr = document.attribute("format-version", "");
    if(versionStr.isEmpty()) RuntimeThrow("No format version specified");
//    const int version = XmlExportHelpers::stringToInt(versionStr);

    auto bColors = XmlExportHelpers::getOnlyElement(document, "ColorBookmarks");
    const auto colors = bColors.elementsByTagName("Color");
    const int nColors = colors.count();
    for(int i = 0; i < nColors; i++) {
        const auto color = colors.at(i);
        if(!color.isElement()) continue;
        const auto colorEle = color.toElement();
        const QString name = colorEle.attribute("name");
        if(name.isEmpty()) continue;
        fColors << QColor(name);
    }

    auto bBrushes = XmlExportHelpers::getOnlyElement(document, "BrushBookmarks");
    const auto brushes = bBrushes.elementsByTagName("Brush");
    const int nBrushes = brushes.count();
    for(int i = 0; i < nBrushes; i++) {
        const auto brush = brushes.at(i);
        if(!brush.isElement()) continue;
        const auto brushEle = brush.toElement();
        const QString coll = brushEle.attribute("collection");
        const QString name = brushEle.attribute("name");
        const auto brushPtr = BrushCollectionData::sGetBrush(coll, name);
        if(brushPtr) fBrushes.append(brushPtr);
    }

    auto scenesE = XmlExportHelpers::getOnlyElement(document, "Scenes");
    const auto scenes = scenesE.elementsByTagName("Scene");
    const int nScenes = scenes.count();
    for(int i = 0; i < nScenes; i++) {
        const auto scene = scenes.at(i);
        if(!scene.isElement()) continue;
        const auto sceneEle = scene.toElement();
        SceneSettingsXEV sett;
        sett.fName = sceneEle.attribute("name");
        sett.fFrame = XmlExportHelpers::stringToInt(sceneEle.attribute("frame"));
        sett.fWidth = XmlExportHelpers::stringToInt(sceneEle.attribute("width"));
        sett.fHeight = XmlExportHelpers::stringToInt(sceneEle.attribute("height"));
        sett.fFps = XmlExportHelpers::stringToDouble(sceneEle.attribute("fps"));

        sceneSetts << sett;
    }
}

void Document::readScenesXEV(ZipFileLoader& fileLoader,
                             const QList<SceneSettingsXEV>& sceneSetts) {
    int id = 0;
    for(const auto& sett : sceneSetts) {
        const auto newScene = createNewScene();
        newScene->prp_setName(sett.fName);
        newScene->anim_setAbsFrame(sett.fFrame);
        newScene->setCanvasSize(sett.fWidth, sett.fHeight);
        newScene->setFps(sett.fFps);
        const QString path = "scenes/" + QString::number(id++) + "/";
        newScene->readAllContainedXEV(fileLoader, path);
    }
}

void Document::readXEV(ZipFileLoader& fileLoader) {
    QList<SceneSettingsXEV> sceneSetts;
    fileLoader.process("document.xml", [&](QIODevice* const src) {
        QDomDocument document;
        document.setContent(src);
        readDoxumentXEV(document, sceneSetts);
    });
    readScenesXEV(fileLoader, sceneSetts);
}
