#ifndef SCENELAYOUT_H
#define SCENELAYOUT_H
#include <QMainWindow>
#include "canvaswindowwrapper.h"
#include "canvas.h"

struct SceneOnlyBaseStackItem : public BaseStackItem {
    typedef std::unique_ptr<const SceneOnlyBaseStackItem> cUPtr;
    SceneOnlyBaseStackItem(Canvas* const scene) : mScene(scene) {
        setName(scene->getName());
        auto cwwItem = new CWWidgetStackLayoutItem;
        cwwItem->setScene(scene);
        setChild(std::unique_ptr<WidgetStackLayoutItem>(cwwItem));
    }

    Canvas* getScene() const { return mScene; }
private:
    Canvas* const mScene;
};

class LayoutCollection {
public:
    const BaseStackItem* getAt(const int id) const {
        if(id < 0) return nullptr;
        if(id < int(mLayouts.size())) return mLayouts.at(uint(id)).get();
        else if(uint(id) - mLayouts.size() < mSceneLayouts.size()) {
            return mSceneLayouts.at(uint(id) - mLayouts.size()).get();
        }
        return nullptr;
    }

    int idForScene(Canvas* const scene) {
        for(uint i = 0; i < mSceneLayouts.size(); i++) {
            if(mSceneLayouts[i]->getScene() == scene)
                return int(i + mLayouts.size());
        }
        return -1;
    }

    void replaceCustomLayout(const int id, BaseStackItem::UPtr&& layout) {
        if(id >= 0 && id < int(mLayouts.size())) {
            mLayouts[uint(id)] = std::move(layout);
        }
    }

    int addCustomLayout(BaseStackItem::UPtr&& layout) {
        mLayouts.insert(mLayouts.begin(), std::move(layout));
        return 0;
    }

    int addSceneLayout(Canvas * const scene) {
        auto newL = std::make_unique<SceneOnlyBaseStackItem>(scene);
        mSceneLayouts.insert(mSceneLayouts.begin(), std::move(newL));
        return int(mLayouts.size());
    }

    int removeSceneLayout(Canvas * const scene) {
        int id = -1;
        for(uint i = 0; i < mSceneLayouts.size(); i++) {
            if(mSceneLayouts[i]->getScene() == scene) {
                id = int(i);
                mSceneLayouts.erase(mSceneLayouts.begin() + i);
                break;
            }
        }
        return int(mLayouts.size()) + id;
    }

    void removeCustomLayout(const int id) {
        if(id >= 0 && id < int(mLayouts.size()))
            mLayouts.erase(mLayouts.begin() + id);
    }

    bool isCustom(const int id) const {
        return id < customCount();
    }

    QStringList getCustomNames() const {
        QStringList result;
        for(const auto& lay : mLayouts)
            result << lay->getName();
        return result;
    }

    QStringList getSceneNames() const {
        QStringList result;
        for(const auto& lay : mSceneLayouts)
            result << lay->getName();
        return result;
    }

    int customCount() const { return int(mLayouts.size()); }
private:
    std::vector<BaseStackItem::UPtr> mLayouts;
    std::vector<SceneOnlyBaseStackItem::cUPtr> mSceneLayouts;
};

class SceneLayout : public QObject {
    Q_OBJECT
public:
    SceneLayout(Document& document, QMainWindow* const window);

    void setCurrent(const int id);
    void remove(const int id);
    void removeCurrent();
    QString duplicate();
    QString newEmpty();
    void setCurrentName(const QString& name);
    QStringList getCustomNames() const {
        return mCollection.getCustomNames();
    }

    QStringList getSceneNames() const {
        return mCollection.getSceneNames();
    }

    bool isCurrentCustom() const {
        return mCollection.isCustom(mCurrentId);
    }
signals:
    void removed(int);
    void created(int, QString);
    void renamed(int, QString);
private:
    void setName(const int id, const QString& name);

    void sceneNameChanged(Canvas* const scene);
    void newForScene(Canvas* const scene);
    void removeForScene(Canvas* const scene);
    void reset(CanvasWindowWrapper ** const cwwP = nullptr,
               Canvas * const scene = nullptr);

    int mCurrentId = -1;
    LayoutCollection mCollection;

    Document& mDocument;
    QMainWindow* const mWindow;
    BaseStackItem::UPtr mBaseStack;

};

#endif // SCENELAYOUT_H
