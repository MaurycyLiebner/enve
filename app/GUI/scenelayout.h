#ifndef SCENELAYOUT_H
#define SCENELAYOUT_H
#include <QMainWindow>
#include "canvaswindowwrapper.h"

class LayoutCollection {
public:
    const BaseStackItem* getAt(const int id) const {
        if(id < 0) return nullptr;
        if(id < mLayouts.size()) mLayouts.at(id).get();
        else if(id - mLayouts.size() < mConstLayouts.size()) {
            mConstLayouts.at(id - mLayouts.size()).get();
        }
        return nullptr;
    }

    void replaceCustomLayout(const int id, BaseStackItem::UPtr&& layout) {
        if(id >= 0 && id < mLayouts.size()) {
            mLayouts[id] = std::move(layout);
        }
    }

    int addCustomLayout(BaseStackItem::UPtr&& layout) {
        mLayouts.insert(mLayouts.begin(), std::move(layout));
        return 0;
    }

    int addConstLayout(BaseStackItem::cUPtr&& layout) {
        mConstLayouts.insert(mConstLayouts.begin(), std::move(layout));
        return mLayouts.size();
    }

    void removeLayout(const int id) {
        if(id < 0) return;
        if(id < mLayouts.size()) mLayouts.erase(mLayouts.begin() + id);
        else if(id - mLayouts.size() < mConstLayouts.size()) {
            mConstLayouts.erase(mConstLayouts.begin() + id - mLayouts.size());
        }
    }

    bool isCustom(const int id) {
        return id < customCount();
    }

    int customCount() const { return static_cast<int>(mLayouts.size()); }
private:
    std::vector<BaseStackItem::UPtr> mLayouts;
    std::vector<BaseStackItem::cUPtr> mConstLayouts;
};

class SceneLayout {
public:
    SceneLayout(Document& document, QMainWindow* const window);

    void duplicateLayout(const int id);
private:
    BaseStackItem::UPtr apply(const int id);
    BaseStackItem::UPtr reset(CanvasWindowWrapper ** const cwwP = nullptr);

    int mCurrentId = -1;
    LayoutCollection mCollection;

    Document& mDocument;
    QMainWindow* const mWindow;
    BaseStackItem::UPtr mBaseStack;

};

#endif // SCENELAYOUT_H
