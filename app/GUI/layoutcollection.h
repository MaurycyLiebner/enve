#ifndef LAYOUTCOLLECTION_H
#define LAYOUTCOLLECTION_H
#include "canvas.h"
#include "canvaswindowwrapper.h"
#include "timelinewrapper.h"

struct SceneBaseStackItem : public BaseStackItem {
    typedef std::unique_ptr<SceneBaseStackItem> cUPtr;
protected:
    SceneBaseStackItem(std::unique_ptr<SceneWidgetStackLayoutItem>&& cwwItem,
                       Canvas* const scene = nullptr) : mScene(scene) {
        if(scene) setName(scene->getName());
        cwwItem->setScene(scene);
        setChild(std::move(cwwItem));
    }
public:

    void setScene(Canvas* const scene) {
        mScene = scene;
    }

    Canvas* getScene() const { return mScene; }
private:
    using BaseStackItem::write;
    Canvas* mScene = nullptr;
};

struct TSceneBaseStackItem : public SceneBaseStackItem {
    TSceneBaseStackItem(Canvas* const scene = nullptr) :
        SceneBaseStackItem(std::make_unique<TWidgetStackLayoutItem>(), scene) {}
};

struct CWSceneBaseStackItem : public SceneBaseStackItem {
    CWSceneBaseStackItem(Canvas* const scene = nullptr) :
        SceneBaseStackItem(std::make_unique<CWWidgetStackLayoutItem>(), scene) {}
};

#endif // LAYOUTCOLLECTION_H
