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
        if(scene) setName(scene->prp_getName());
        cwwItem->setScene(scene);
        setChild(std::move(cwwItem));
    }

    template <typename WidgetT>
    void readSceneBaseStackItem(QIODevice* const src) {
        readBaseStackItem<WidgetT>(src);
    }
public:
    void setScene(Canvas* const scene) {
        mScene = scene;
    }

    Canvas* getScene() const { return mScene; }
private:
    Canvas* mScene = nullptr;
};

struct TSceneBaseStackItem : public SceneBaseStackItem {
    TSceneBaseStackItem(Canvas* const scene = nullptr) :
        SceneBaseStackItem(std::make_unique<TWidgetStackLayoutItem>(), scene) {}

    void read(QIODevice* const src) {
        readSceneBaseStackItem<TWidgetStackLayoutItem>(src);
    }
};

struct CWSceneBaseStackItem : public SceneBaseStackItem {
    CWSceneBaseStackItem(Canvas* const scene = nullptr) :
        SceneBaseStackItem(std::make_unique<CWWidgetStackLayoutItem>(), scene) {}

    void read(QIODevice* const src) {
        readSceneBaseStackItem<CWWidgetStackLayoutItem>(src);
    }
};

#endif // LAYOUTCOLLECTION_H
