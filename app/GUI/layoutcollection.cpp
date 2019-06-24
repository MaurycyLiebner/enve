#include "layoutcollection.h"
template<typename TO, typename FROM>
std::unique_ptr<TO> static_unique_pointer_cast(std::unique_ptr<FROM>&& old){
    return std::unique_ptr<TO>{static_cast<TO*>(old.release())};
}

const BaseStackItem *LayoutCollection::getAt(const int id) const {
    if(id < 0) return nullptr;
    if(id < int(mLayouts.size())) return mLayouts.at(uint(id)).get();
    else if(uint(id) - mLayouts.size() < mSceneLayouts.size()) {
        return mSceneLayouts.at(uint(id) - mLayouts.size()).get();
    }
    return nullptr;
}

int LayoutCollection::idForScene(Canvas * const scene) {
    for(uint i = 0; i < mSceneLayouts.size(); i++) {
        if(mSceneLayouts[i]->getScene() == scene)
            return int(i + mLayouts.size());
    }
    return -1;
}

auto sceneOnlyBaseStackItemCast(BaseStackItem::UPtr&& old) {
    return static_unique_pointer_cast<SceneBaseStackItem>(std::move(old));
}

void LayoutCollection::replaceCustomLayout(const int id, BaseStackItem::UPtr &&layout) {
    if(id >= 0 && id < int(mLayouts.size())) {
        mLayouts[uint(id)] = std::move(layout);
    } else if(id >= 0 && id < int(mSceneLayouts.size() + mLayouts.size())) {
        const int relId = id - int(mLayouts.size());
        mSceneLayouts[uint(relId)] = sceneOnlyBaseStackItemCast(std::move(layout));
    }
}

int LayoutCollection::addCustomLayout(BaseStackItem::UPtr &&layout) {
    mLayouts.insert(mLayouts.begin(), std::move(layout));
    return 0;
}

int LayoutCollection::addSceneLayout(Canvas * const scene) {
    auto newL = std::make_unique<SceneBaseStackItem>(scene);
    return addSceneLayout(std::move(newL));
}

int LayoutCollection::removeSceneLayout(Canvas * const scene) {
    const int id = getSceneLayoutRelId(scene);
    mSceneLayouts.erase(mSceneLayouts.begin() + id);
    return int(mLayouts.size()) + id;
}

bool LayoutCollection::removeCustomLayout(const int id) {
    if(id < 0 || id >= int(mLayouts.size())) return false;
    mLayouts.erase(mLayouts.begin() + id);
    return true;
}

bool LayoutCollection::resetSceneLayout(const int id, Canvas * const scene) {
    if(id == -1 || !scene) return false;
    mSceneLayouts[id] = std::make_unique<SceneBaseStackItem>(scene);
    return true;
}

bool LayoutCollection::resetSceneLayout(Canvas * const scene) {
    const int id = getSceneLayoutRelId(scene);
    return resetSceneLayout(id, scene);
}

bool LayoutCollection::resetSceneLayout(const int id) {
    const auto scene = mSceneLayouts[id]->getScene();
    return resetSceneLayout(id, scene);
}

bool LayoutCollection::isCustom(const int id) const {
    return id < customCount();
}

QStringList LayoutCollection::getCustomNames() const {
    QStringList result;
    for(const auto& lay : mLayouts)
        result << lay->getName();
    return result;
}

QStringList LayoutCollection::getSceneNames() const {
    QStringList result;
    for(const auto& lay : mSceneLayouts)
        result << lay->getName();
    return result;
}

int LayoutCollection::customCount() const { return int(mLayouts.size()); }

int LayoutCollection::getSceneLayoutRelId(const Canvas * const scene) {
    for(uint i = 0; i < mSceneLayouts.size(); i++) {
        if(mSceneLayouts[i]->getScene() == scene) {
            return int(i);
        }
    }
    return -1;
}

int LayoutCollection::addSceneLayout(SceneBaseStackItem::cUPtr &&newL) {
    mSceneLayouts.insert(mSceneLayouts.begin(), std::move(newL));
    return int(mLayouts.size());
}
