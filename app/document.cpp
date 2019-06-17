#include "document.h"
#include "FileCacheHandlers/filecachehandler.h"
#include "canvas.h"

bool Document::FileCompare::operator()(const FileHandler &f1,
                                       const FileHandler &f2) {
    return f1->getFilePath() < f2->getFilePath();
}

Canvas *Document::createNewScene() {
    const auto newScene = SPtrCreate(Canvas)(*this);
    fScenes.append(newScene);
    SWT_addChild(newScene.get());
    emit sceneCreated(newScene.get());
    return newScene.get();
}

void Document::removeScene(const int id) {
    const auto scene = fScenes.takeAt(id);
    SWT_removeChild(scene.data());
    emit sceneRemoved(scene.data());
    emit sceneRemoved(id);
}

void Document::setActiveScene(Canvas * const scene) {
    if(scene == fLastActiveScene) return;
    fLastActiveScene = scene;
    SWT_scheduleContentUpdate(scene ? scene->getCurrentGroup() : nullptr,
                              SWT_TARGET_CURRENT_GROUP);
    SWT_scheduleContentUpdate(scene, SWT_TARGET_CURRENT_CANVAS);
}

void Document::clear() {
    for(const auto& scene : fScenes)
        SWT_removeChild(scene.data());
    fScenes.clear();
}

void Document::write(QIODevice * const dst) const {
    const int nScenes = fScenes.count();
    dst->write(rcConstChar(&nScenes), sizeof(int));
    for(const auto &scene : fScenes)
        scene->writeBoundingBox(dst);
//        if(canvas.get() == mCurrentCanvas) {
//            currentCanvasId = mCurrentCanvas->getWriteId();
//        }
//    }
//    target->write(rcConstChar(&currentCanvasId), sizeof(int));
}

void Document::read(QIODevice * const src) {
    int nScenes;
    src->read(rcChar(&nScenes), sizeof(int));
    for(int i = 0; i < nScenes; i++) {
        const auto canvas = createNewScene();
        canvas->readBoundingBox(src);
    }
//    int currentCanvasId;
//    src->read(rcChar(&currentCanvasId), sizeof(int));
//    auto currentCanvas = BoundingBox::sGetBoxByReadId(currentCanvasId);
//    setCurrentCanvas(GetAsPtr(currentCanvas, Canvas));
}

void Document::SWT_setupAbstraction(SWT_Abstraction * const abstraction,
                                    const UpdateFuncs &updateFuncs,
                                    const int visiblePartWidgetId) {
    for(const auto& scene : fScenes) {
        auto abs = scene->SWT_abstractionForWidget(updateFuncs,
                                                   visiblePartWidgetId);
        abstraction->addChildAbstraction(abs);
    }
}
