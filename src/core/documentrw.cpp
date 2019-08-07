#include "document.h"
#include "basicreadwrite.h"

#include "Animators/gradient.h"
#include "canvas.h"

void Document::writeGradients(QIODevice * const dst) const {
    const int nGrads = fGradients.count();
    dst->write(rcConstChar(&nGrads), sizeof(int));
    int id = 0;
    for(const auto &grad : fGradients)
        grad->write(id++, dst);
}

void Document::writeScenes(QIODevice * const dst) const {
    const int nScenes = fScenes.count();
    dst->write(rcConstChar(&nScenes), sizeof(int));
    for(const auto &scene : fScenes)
        scene->writeBoundingBox(dst);
}

void Document::write(QIODevice * const dst) const {
    writeGradients(dst);
    writeScenes(dst);
    clearGradientRWIds();

//        if(canvas.get() == mCurrentCanvas) {
//            currentCanvasId = mCurrentCanvas->getWriteId();
//        }
//    }
//    target->write(rcConstChar(&currentCanvasId), sizeof(int));
}

void Document::readGradients(QIODevice * const src) {
    int nGrads;
    src->read(rcChar(&nGrads), sizeof(int));
    for(const auto &grad : fGradients)
        grad->read(src);
}

void Document::readScenes(QIODevice * const src) {
    int nScenes;
    src->read(rcChar(&nScenes), sizeof(int));
    for(int i = 0; i < nScenes; i++) {
        const auto canvas = createNewScene();
        canvas->readBoundingBox(src);
    }
}

void Document::read(QIODevice * const src) {
    readGradients(src);
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
