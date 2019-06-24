#ifndef LAYOUTCOLLECTION_H
#define LAYOUTCOLLECTION_H
#include "canvas.h"
#include "canvaswindowwrapper.h"

struct SceneBaseStackItem : public BaseStackItem {
    typedef std::unique_ptr<const SceneBaseStackItem> cUPtr;
    SceneBaseStackItem(Canvas* const scene = nullptr) :
        SceneBaseStackItem(new CWWidgetStackLayoutItem, scene) {}

    SceneBaseStackItem(CWWidgetStackLayoutItem* const cwwItem,
                       Canvas* const scene = nullptr) : mScene(scene) {
        if(scene) setName(scene->getName());
        cwwItem->setScene(scene);
        setChild(std::unique_ptr<WidgetStackLayoutItem>(cwwItem));
    }

    void setScene(Canvas* const scene) {
        mScene = scene;
    }

    Canvas* getScene() const { return mScene; }
private:
    using BaseStackItem::write;
    Canvas* mScene = nullptr;
};

class LayoutCollection {
public:
    const BaseStackItem* getAt(const int id) const;

    int idForScene(Canvas* const scene);

    void replaceCustomLayout(const int id, BaseStackItem::UPtr&& layout);

    int addCustomLayout(BaseStackItem::UPtr&& layout);
    int addSceneLayout(Canvas * const scene);

    int removeSceneLayout(Canvas * const scene);
    bool removeCustomLayout(const int id);

    bool resetSceneLayout(const int id, Canvas * const scene);
    bool resetSceneLayout(Canvas * const scene);
    bool resetSceneLayout(const int id);

    bool isCustom(const int id) const;

    QStringList getCustomNames() const;
    QStringList getSceneNames() const;

    int customCount() const;
private:
    int getSceneLayoutRelId(const Canvas* const scene);

    int addSceneLayout(SceneBaseStackItem::cUPtr&& newL);

    std::vector<BaseStackItem::UPtr> mLayouts;
    std::vector<SceneBaseStackItem::cUPtr> mSceneLayouts;
};

#endif // LAYOUTCOLLECTION_H
