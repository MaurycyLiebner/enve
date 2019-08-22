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
    dst.write(rcConstChar(&nScenes), sizeof(int));
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
//    int currentCanvasId;
//    src->read(rcChar(&currentCanvasId), sizeof(int));
//    auto currentCanvas = BoundingBox::sGetBoxByReadId(currentCanvasId);
    //    setCurrentCanvas(static_cast<Canvas*>(currentCanvas));
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
