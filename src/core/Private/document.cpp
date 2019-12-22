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

#include "Private/document.h"
#include "FileCacheHandlers/filecachehandler.h"
#include "canvas.h"
#include "simpletask.h"

Document* Document::sInstance = nullptr;

bool Document::FileCompare::operator()(const FileHandler &f1,
                                       const FileHandler &f2) {
    return f1->getFilePath() < f2->getFilePath();
}

Document::Document(TaskScheduler& taskScheduler) {
    Q_ASSERT(!sInstance);
    sInstance = this;
    connect(&taskScheduler, &TaskScheduler::finishedAllQuedTasks,
            this, &Document::actionFinished);
}

void Document::actionFinished() {
    SimpleTask::sProcessAll();
    TaskScheduler::sInstance->queTasks();

    for(const auto& scene : fVisibleScenes) {
        if(scene.first->newUndoRedoSet())
            emit documentChanged();
        emit scene.first->requestUpdate();
    }
}

void Document::replaceClipboard(const stdsptr<Clipboard> &container) {
    fClipboardContainer = container;
}

Clipboard *Document::getClipboard(const ClipboardType type) const {
    if(!fClipboardContainer) return nullptr;
    if(type == fClipboardContainer->getType())
        return fClipboardContainer.get();
    return nullptr;
}

DynamicPropsClipboard* Document::getDynamicPropsClipboard() const {
    auto contT = getClipboard(ClipboardType::dynamicProperties);
    return static_cast<DynamicPropsClipboard*>(contT);
}

PropertyClipboard* Document::getPropertyClipboard() const {
    auto contT = getClipboard(ClipboardType::property);
    return static_cast<PropertyClipboard*>(contT);
}

KeysClipboard* Document::getKeysClipboard() const {
    auto contT = getClipboard(ClipboardType::keys);
    return static_cast<KeysClipboard*>(contT);
}

BoxesClipboard* Document::getBoxesClipboard() const {
    auto contT = getClipboard(ClipboardType::boxes);
    return static_cast<BoxesClipboard*>(contT);
}

SmartPathClipboard* Document::getSmartPathClipboard() const {
    auto contT = getClipboard(ClipboardType::smartPath);
    return static_cast<SmartPathClipboard*>(contT);
}

void Document::setCanvasMode(const CanvasMode mode) {
    fCanvasMode = mode;
    emit canvasModeSet(mode);
    actionFinished();
}

Canvas *Document::createNewScene() {
    const auto newScene = enve::make_shared<Canvas>(*this);
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

void Document::addVisibleScene(Canvas * const scene) {
    fVisibleScenes[scene]++;
}

bool Document::removeVisibleScene(Canvas * const scene) {
    const auto it = fVisibleScenes.find(scene);
    if(it == fVisibleScenes.end()) return false;
    if(it->second == 1) fVisibleScenes.erase(it);
    else it->second--;
    return true;
}

void Document::setActiveScene(Canvas * const scene) {
    if(scene == fActiveScene) return;
    if(fActiveScene) {
        disconnect(fActiveScene, nullptr, this, nullptr);
    }
    fActiveScene = scene;
    if(fActiveScene) {
        connect(fActiveScene, &Canvas::currentBoxChanged,
                this, &Document::currentBoxChanged);
        connect(fActiveScene, &Canvas::selectedPaintSettingsChanged,
                this, &Document::selectedPaintSettingsChanged);
        connect(fActiveScene, &Canvas::destroyed,
                this, &Document::clearActiveScene);
        emit currentBoxChanged(fActiveScene->getCurrentBox());
        emit selectedPaintSettingsChanged();
    }
    emit activeSceneSet(scene);
}

void Document::clearActiveScene() {
    setActiveScene(nullptr);
}

int Document::getActiveSceneFrame() const {
    if(!fActiveScene) return 0;
    return fActiveScene->anim_getCurrentAbsFrame();
}

void Document::setActiveSceneFrame(const int frame) {
    if(!fActiveScene) return;
    if(fActiveScene->anim_getCurrentRelFrame() == frame) return;
    fActiveScene->anim_setAbsFrame(frame);
    emit activeSceneFrameSet(frame);
}

void Document::incActiveSceneFrame() {
    setActiveSceneFrame(getActiveSceneFrame() + 1);
}

void Document::decActiveSceneFrame() {
    setActiveSceneFrame(getActiveSceneFrame() - 1);
}

Gradient *Document::createNewGradient() {
    const auto grad = enve::make_shared<Gradient>();
    fGradients.append(grad);
    emit gradientCreated(grad.get());
    return grad.get();
}

Gradient *Document::duplicateGradient(const int id) {
    if(id < 0 || id >= fGradients.count()) return nullptr;
    const auto from = fGradients.at(id).get();
    const auto newGrad = createNewGradient();
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    eWriteStream writeStream(&buffer);
    from->write(-1, writeStream);
    if(buffer.reset()) {
        eReadStream readStream(&buffer);
        newGrad->read(readStream);
    }
    buffer.close();
    return newGrad;
}

bool Document::removeGradient(const qsptr<Gradient> &gradient) {
    const int id = fGradients.indexOf(gradient);
    return removeGradient(id);
}

bool Document::removeGradient(const int id) {
    if(id < 0 || id >= fGradients.count()) return false;
    const auto grad = fGradients.takeAt(id);
    emit gradientRemoved(grad.data());
    return true;
}

void Document::clear() {
    setPath("");
    const int nScenes = fScenes.count();
    for(int i = 0; i < nScenes; i++) removeScene(0);
    replaceClipboard(nullptr);
    fGradients.clear();
    fBrushes.clear();
    fColors.clear();
}

void Document::SWT_setupAbstraction(SWT_Abstraction * const abstraction,
                                    const UpdateFuncs &updateFuncs,
                                    const int visiblePartWidgetId) {
    for(const auto& scene : fScenes) {
        auto abs = scene->SWT_abstractionForWidget(updateFuncs,
                                                   visiblePartWidgetId);
        abstraction->addChildAbstraction(abs->ref<SWT_Abstraction>());
    }
}
