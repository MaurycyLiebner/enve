// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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

    void write(eWriteStream& dst) const {
        dst << fName;
        fCanvas->write(dst);
        fTimeline->write(dst);
    }

    void read(eReadStream& src) {
        src >> fName;
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

    void write(eWriteStream& dst) const {
        dst << mNumberLayouts;
        for(int i = mNumberLayouts - 1; i >= 0; i--) {
            mLayouts.at(uint(i)).write(dst);
        }
        const int nScenes = int(mLayouts.size()) - mNumberLayouts;
        dst << nScenes;
        for(int i = 0; i < nScenes; i++) {
            mLayouts.at(uint(i + mNumberLayouts)).write(dst);
        }
        dst << mCurrentId;
    }

    void read(eReadStream& src) {
        setCurrent(-1);

        int nLays; src >> nLays;
        for(int i = 0; i < nLays; i++) {
            newLayout().read(src);
        }
        int nScenes; src >> nScenes;
        for(int i = 0; i < nScenes; i++) {
            mLayouts.at(uint(i + mNumberLayouts)).read(src);
        }
        int relCurrentId; src >> relCurrentId;
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
        const int newId = qBound(0, mCurrentId, int(mLayouts.size()) - 1);
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
