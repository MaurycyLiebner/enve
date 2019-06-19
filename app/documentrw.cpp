#include "document.h"
#include "basicreadwrite.h"

class FileFooter {
public:
    static bool sWrite(QIODevice * const target) {
        return target->write(rcConstChar(sEVFormat), sizeof(char[15])) &&
               target->write(rcConstChar(sAppName), sizeof(char[15])) &&
               target->write(rcConstChar(sAppVersion), sizeof(char[15]));
    }

    static bool sCompatible(QIODevice *target) {
        const qint64 savedPos = target->pos();
        const qint64 pos = target->size() -
                static_cast<qint64>(3*sizeof(char[15]));
        if(!target->seek(pos)) RuntimeThrow("Failed to seek to FileFooter");

        char format[15];
        target->read(rcChar(format), sizeof(char[15]));
        if(std::strcmp(format, sEVFormat)) return false;

//        char appVersion[15];
//        target->read(rcChar(appVersion), sizeof(char[15]));

//        char appName[15];
//        target->read(rcChar(appName), sizeof(char[15]));

        if(!target->seek(savedPos))
            RuntimeThrow("Could not restore current position for QIODevice.");
        return true;
    }
private:
    static char sEVFormat[15];
    static char sAppName[15];
    static char sAppVersion[15];
};

char FileFooter::sEVFormat[15] = "enve ev";
char FileFooter::sAppName[15] = "enve";
char FileFooter::sAppVersion[15] = "0.5";

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

    FileFooter::sWrite(dst);
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
    if(!FileFooter::sCompatible(src)) {
        RuntimeThrow("Incompatible or incomplete data");
    }

    readGradients(src);
    readScenes(src);
    clearGradientRWIds();
//    int currentCanvasId;
//    src->read(rcChar(&currentCanvasId), sizeof(int));
//    auto currentCanvas = BoundingBox::sGetBoxByReadId(currentCanvasId);
    //    setCurrentCanvas(GetAsPtr(currentCanvas, Canvas));
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
