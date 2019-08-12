#ifndef LAYOUTHANDLER_H
#define LAYOUTHANDLER_H
#include <QComboBox>
#include "scenelayout.h"

struct LayoutData {
    LayoutData(const QString& name) : fName(name) {
        reset();
    }

    LayoutData(Canvas* const scene) :
        fName(scene->prp_getName()), fScene(scene) {
        reset();
    }

    void reset() {
        fCanvas = std::make_unique<CWSceneBaseStackItem>(fScene);
        fTimeline = std::make_unique<TSceneBaseStackItem>(fScene);


    }

    void write(QIODevice* const dst) const {
        gWrite(dst, fName);
        fCanvas->write(dst);
        fTimeline->write(dst);
    }

    void read(QIODevice* const src) {
        fName = gReadString(src);
        fCanvas->read(src);
        fTimeline->read(src);
    }

    QString fName;
    std::unique_ptr<CWSceneBaseStackItem> fCanvas;
    std::unique_ptr<TSceneBaseStackItem> fTimeline;
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

    SceneLayout* timelineLayout() {
        return mTimelineLayout;
    }

    void clear();

    void write(QIODevice* const dst) const {
        dst->write(rcConstChar(&mNumberLayouts), sizeof(int));
        for(int i = mNumberLayouts - 1; i >= 0; i--) {
            mLayouts.at(uint(i)).write(dst);
        }
        const int nScenes = int(mLayouts.size()) - mNumberLayouts;
        dst->write(rcConstChar(&nScenes), sizeof(int));
        for(int i = 0; i < nScenes; i++) {
            mLayouts.at(uint(i + mNumberLayouts)).write(dst);
        }
        dst->write(rcConstChar(&mCurrentId), sizeof(int));
    }

    void read(QIODevice* const src) {
        setCurrent(-1);

        int nLays;
        src->read(rcChar(&nLays), sizeof(int));
        for(int i = 0; i < nLays; i++) {
            newLayout().read(src);
        }
        int nScenes;
        src->read(rcChar(&nScenes), sizeof(int));
        for(int i = 0; i < nScenes; i++) {
            mLayouts.at(uint(i + mNumberLayouts)).read(src);
        }
        int relCurrentId;
        src->read(rcChar(&relCurrentId), sizeof(int));
        if(relCurrentId == -1) return;
        const int absId = relCurrentId < nLays ? relCurrentId :
                                                 mNumberLayouts - nLays + relCurrentId;
        setCurrent(absId);
    }
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
        mLayouts[uint(mCurrentId)].reset();
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
        if(id == -1) {
            mSceneLayout->setCurrent(nullptr);
            mTimelineLayout->setCurrent(nullptr);
            return;
        }
        const auto& current = mLayouts.at(uint(id));

        mComboBox->setCurrentIndex(id);
        mSceneLayout->setCurrent(current.fCanvas.get());
        mTimelineLayout->setCurrent(current.fTimeline.get());
    }

    LayoutData& newLayout() {
        const QString name = "Layout " + QString::number(mNumberLayouts);
        const auto it = mLayouts.insert(mLayouts.begin(),
                        LayoutData(name));
        mComboBox->insertItem(0, name);
        if(mCurrentId != -1) mCurrentId++;
        mNumberLayouts++;
        return *it;
    }

    void newLayoutAction() {
        newLayout();
        setCurrent(0);
    }

    void newForScene(Canvas* const scene) {
        mLayouts.insert(mLayouts.begin() + mNumberLayouts,
                        LayoutData(scene));
        mComboBox->insertItem(mNumberLayouts, scene->prp_getName());
        if(mCurrentId >= mNumberLayouts) mCurrentId++;
        connect(scene, &Canvas::prp_nameChanged, this, [this, scene]() {
            sceneRenamed(scene);
        });
    }

    void newForSceneAction(Canvas* const scene) {
        newForScene(scene);
        setCurrent(mNumberLayouts);
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
        rename(sceneId(scene), scene->prp_getName());
    }

    void removeForScene(const Canvas* const scene) {
        removeAt(sceneId(scene));
        disconnect(scene, nullptr, this, nullptr);
    }

    void removeAt(const int id) {
        if(id < mNumberLayouts) mNumberLayouts--;
        mLayouts.erase(mLayouts.begin() + id);
        mComboBox->removeItem(id);
        if(mCurrentId == id) mCurrentId = -1;
        else if(mCurrentId > id) mCurrentId--;
        const int newId = qMin(mCurrentId, int(mLayouts.size()) - 1);
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
    SceneLayout* mTimelineLayout;
};

#endif // LAYOUTHANDLER_H
