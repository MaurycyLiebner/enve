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

#include "document.h"
#include "basicreadwrite.h"

#include "Animators/gradient.h"
#include "canvas.h"

void Document::writeGradients(eWriteStream &dst) const {
    const int nGrads = fGradients.count();
    dst.write(&nGrads, sizeof(int));
    int id = 0;
    for(const auto &grad : fGradients)
        grad->write(id++, dst);
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
    writeGradients(dst);
    dst.writeCheckpoint();
    writeScenes(dst);
    clearGradientRWIds();

//        if(canvas.get() == mCurrentCanvas) {
//            currentCanvasId = mCurrentCanvas->getWriteId();
//        }
//    }
//    target->write(rcConstChar(&currentCanvasId), sizeof(int));
}

void Document::readGradients(eReadStream& src) {
    int nGrads;
    src.read(&nGrads, sizeof(int));
    for(int i = 0; i < nGrads; i++) {
        createNewGradient()->read(src);
    }
}

void Document::readScenes(eReadStream& src) {
    int nScenes;
    src.read(&nScenes, sizeof(int));
    for(int i = 0; i < nScenes; i++) {
        const auto canvas = createNewScene();
        canvas->readBoundingBox(src);
        src.readCheckpoint("Error reading scene");
    }
}

void Document::read(eReadStream& src) {
    readGradients(src);
    src.readCheckpoint("Error reading gradients");
    readScenes(src);
    clearGradientRWIds();
}

Gradient *Document::getGradientWithRWId(const int rwId) {
    for(const auto &grad : fGradients) {
        if(grad->getReadWriteId() == rwId) return grad.get();
    }
    return nullptr;
}

void Document::clearGradientRWIds() const {
    for(const auto &grad : fGradients)
        grad->clearReadWriteId();
}
