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

    static cUPtr sRead(QIODevice* const src) {
        const QString name = gReadString(src);

    }
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
        return addSceneLayout(std::move(newL));
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

    void write(QIODevice* const dst) const {
        const int nCustom = customCount();
        dst->write(rcConstChar(&nCustom), sizeof(int));
        for(const auto& lay : mLayouts)
            lay->write(dst);

        const int nScene = int(mSceneLayouts.size());
        dst->write(rcConstChar(&nScene), sizeof(int));
        for(const auto& lay : mSceneLayouts)
            lay->write(dst);
    }

    void read(QIODevice* const src) {
        int nCustom;
        src->read(rcChar(&nCustom), sizeof(int));
        for(int i = 0; i < nCustom; i++)
            addCustomLayout(BaseStackItem::sRead(src));

        int nScene;
        src->read(rcChar(&nScene), sizeof(int));
        for(int i = 0; i < nScene; i++)
            addSceneLayout(SceneOnlyBaseStackItem::sRead(src));
    }
private:
    int addSceneLayout(SceneOnlyBaseStackItem::cUPtr&& newL) {
        mSceneLayouts.insert(mSceneLayouts.begin(), std::move(newL));
        return int(mLayouts.size());
    }

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

    void write(QIODevice* const dst) const {
        mCollection.write(dst);

    }

    void read(QIODevice* const src) {
        mCollection.read(src);
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
