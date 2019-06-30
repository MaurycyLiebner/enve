#ifndef LAYOUTHANDLER_H
#define LAYOUTHANDLER_H
#include <QComboBox>
#include "scenelayout.h"
#include "timelinelayout.h"

struct LayoutData {
    LayoutData(Document& document, AudioHandler& audioHandler,
               const QString& name) : fName(name) {
        reset(document, audioHandler);
    }

    LayoutData(Document& document, AudioHandler& audioHandler,
               Canvas* const scene) :
        fName(scene->getName()), fScene(scene) {
        reset(document, audioHandler);
    }

    void reset(Document& document, AudioHandler& audioHandler) {
        fCanvas = std::make_unique<CWSceneBaseStackItem>(document, audioHandler, fScene);
        fTimeline = std::make_unique<TSceneBaseStackItem>(document, fScene);
    }

    QString fName;
    SceneBaseStackItem::cUPtr fCanvas;
    SceneBaseStackItem::cUPtr fTimeline;
    Canvas* fScene = nullptr;
};

class LayoutHandler : public QObject {
public:
    LayoutHandler(Document &document, AudioHandler &audioHandler);

    QWidget* comboWidget() {
        return mComboWidget;
    }

    SceneLayout* sceneLayout() {
        return mSceneLayout;
    }

    TimelineLayout* timelineLayout() {
        return mTimelineLayout;
    }

    void clear() {}
private:
    void rename(const int id, const QString& newName) {
        mLayouts[uint(id)].fName = newName;
        mComboBox->setItemText(id, newName);
    }

    void renameCurrent(const QString& newName) {
        if(mCurrentId == -1) return;
        rename(mCurrentId, newName);
    }

    void removeCurrentLayout() {
        removeAt(mCurrentId);
    }

    void resetCurrentScene() {
        mLayouts[uint(mCurrentId)].reset(mDocument, mAudioHandler);
        const int idTmp = mCurrentId;
        mCurrentId = -1;
        setCurrent(idTmp);
    }

    void removeCurrent() {
        if(mCurrentId == -1) return;
        if(mCurrentId < mNumberLayouts)
            removeCurrentLayout();
        else resetCurrentScene();
    }

    void saveCurrent();

    void setCurrent(const int id) {
        if(id == mCurrentId) return;
        saveCurrent();

        mCurrentId = id;
        const auto& current = mLayouts.at(uint(id));

        mComboBox->setCurrentIndex(id);
        mSceneLayout->setCurrent(current.fCanvas.get());
        mTimelineLayout->setCurrent(current.fTimeline.get());
    }

    void newLayout() {
        const QString name = "Layout " + QString::number(mNumberLayouts);
        mLayouts.insert(mLayouts.begin(),
                        LayoutData(mDocument, mAudioHandler, name));
        mComboBox->insertItem(0, name);
        if(mCurrentId != -1) mCurrentId++;
        mNumberLayouts++;
        setCurrent(0);
    }

    void newForScene(Canvas* const scene) {
        mLayouts.insert(mLayouts.begin() + mNumberLayouts,
                        LayoutData(mDocument, mAudioHandler, scene));
        mComboBox->insertItem(mNumberLayouts, scene->getName());
        if(mCurrentId >= mNumberLayouts) mCurrentId++;
        setCurrent(mNumberLayouts);
        connect(scene, &Canvas::nameChanged, this, [this, scene]() {
            sceneRenamed(scene);
        });
    }

    int sceneId(const Canvas* const scene) const {
        int id = -1;
        for(uint i = 0; i < mLayouts.size(); i++) {
            const auto& lay = mLayouts.at(i);
            if(lay.fScene == scene) {
                id = int(i);
                break;
            }
        }
        return id;
    }

    void sceneRenamed(const Canvas* const scene) {
        rename(sceneId(scene), scene->getName());
    }

    void removeForScene(const Canvas* const scene) {
        removeAt(sceneId(scene));
        disconnect(scene, nullptr, this, nullptr);
    }

    void removeAt(const int id) {
        if(id < mNumberLayouts) mNumberLayouts--;
        mLayouts.erase(mLayouts.begin() + mCurrentId);
        mComboBox->removeItem(id);
        const int newId = qMin(mCurrentId, int(mLayouts.size()) - 1);
        if(mCurrentId == id) mCurrentId = -1;
        else if(mCurrentId > id) mCurrentId--;
        setCurrent(newId);
    }

    Document& mDocument;
    AudioHandler& mAudioHandler;

    int mCurrentId = -1;
    int mNumberLayouts = 0;
    std::vector<LayoutData> mLayouts;

    QWidget* mComboWidget;
    QComboBox* mComboBox;
    SceneLayout* mSceneLayout;
    TimelineLayout* mTimelineLayout;
};

#endif // LAYOUTHANDLER_H
