#include "document.h"
#include "FileCacheHandlers/filecachehandler.h"
#include "canvas.h"
Document* Document::sInstance = nullptr;

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

bool Document::removeScene(const qsptr<Canvas>& scene) {
    const int id = fScenes.indexOf(scene);
    return removeScene(id);
}

bool Document::removeScene(const int id) {
    if(id < 0 || id >= fScenes.count()) return false;
    const auto scene = fScenes.takeAt(id);
    SWT_removeChild(scene.data());
    emit sceneRemoved(scene.data());
    emit sceneRemoved(id);
    return true;
}

void Document::setActiveScene(Canvas * const scene) {
    if(scene == fLastActiveScene) return;
    fLastActiveScene = scene;
    SWT_scheduleContentUpdate(scene ? scene->getCurrentGroup() : nullptr,
                              SWT_TARGET_CURRENT_GROUP);
    SWT_scheduleContentUpdate(scene, SWT_TARGET_CURRENT_CANVAS);
    emit activeSceneChanged(scene);
}

void Document::clear() {
    for(const auto& scene : fScenes)
        SWT_removeChild(scene.data());
    fScenes.clear();
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
