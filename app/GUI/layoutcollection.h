#ifndef LAYOUTCOLLECTION_H
#define LAYOUTCOLLECTION_H
#include "canvas.h"
#include "canvaswindowwrapper.h"
#include "timelinewrapper.h"

struct SceneBaseStackItem : public BaseStackItem {
    typedef std::unique_ptr<const SceneBaseStackItem> cUPtr;
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

class LayoutCollection {
    typedef std::function<SceneBaseStackItem::cUPtr(Canvas*)> Creator;
public:
    LayoutCollection(const Creator& creator);

    template <typename T>
    static std::function<SceneBaseStackItem::cUPtr(Canvas*)> sCreator() {
        return [](Canvas* const scene) {
            return std::make_unique<T>(scene);
        };
    }

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

    const Creator mCreator;
    std::vector<BaseStackItem::UPtr> mLayouts;
    std::vector<SceneBaseStackItem::cUPtr> mSceneLayouts;
};

#endif // LAYOUTCOLLECTION_H
