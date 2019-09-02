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
#include "timelinebasewrappernode.h"
#include "canvasbasewrappernode.h"
#include "canvas.h"
#include "audiohandler.h"
#include <QStackedWidget>

struct LayoutData {
    LayoutData(const QString& name) : fName(name), fScene(nullptr),
        fSceneLayout(new CanvasBaseWrapperNode),
        fTimelineLayout(new TimelineBaseWrapperNode) {
        reset();
    }

    LayoutData(Canvas* const scene) :
        fName(scene->prp_getName()), fScene(scene),
        fSceneLayout(new CanvasBaseWrapperNode),
        fTimelineLayout(new TimelineBaseWrapperNode) {
        reset();
    }

    void reset() {
        fSceneLayout->reset(fScene);
        fTimelineLayout->reset(fScene);
    }

    void write(eWriteStream& dst) const {
        dst << fName;
        fSceneLayout->writeData(dst);
        fTimelineLayout->writeData(dst);
    }

    void read(eReadStream& src) {
        src >> fName;
        fSceneLayout->readData(src);
        fTimelineLayout->readData(src);
    }

    QString fName;
    Canvas* const fScene;

    CanvasBaseWrapperNode* const fSceneLayout;
    TimelineBaseWrapperNode* const fTimelineLayout;
};

class LayoutHandler : public QObject {
public:
    LayoutHandler(Document &document, AudioHandler &audioHandler);

    QWidget* comboWidget() {
        return mComboWidget;
    }

    QStackedWidget* sceneLayout() {
        return mSceneLayout;
    }

    QStackedWidget* timelineLayout() {
        return mTimelineLayout;
    }

    void clear();

    void write(eWriteStream& dst) const {
        dst << mNumberLayouts;
        for(int i = mNumberLayouts - 1; i >= 0; i--) {
            mLayouts.at(uint(i))->write(dst);
        }
        const int nScenes = int(mLayouts.size()) - mNumberLayouts;
        dst << nScenes;
        for(int i = 0; i < nScenes; i++) {
            mLayouts.at(uint(i + mNumberLayouts))->write(dst);
        }
        dst << mCurrentId;
    }

    void read(eReadStream& src) {
        setCurrent(-1);

        int nLays; src >> nLays;
        for(int i = 0; i < nLays; i++) {
            newLayout()->read(src);
        }
        int nScenes; src >> nScenes;
        for(int i = 0; i < nScenes; i++) {
            mLayouts.at(uint(i + mNumberLayouts))->read(src);
        }
        int relCurrentId; src >> relCurrentId;
        if(relCurrentId == -1) return;
        const int absId = relCurrentId < nLays ? relCurrentId :
                                                 mNumberLayouts - nLays + relCurrentId;
        setCurrent(absId);
    }
private:
    void rename(const int id, const QString& newName) {
        mLayouts[uint(id)]->fName = newName;
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

    void setCurrent(const int id) {
        if(id == mCurrentId) return;

        mCurrentId = id;
        mComboBox->setCurrentIndex(id);
        mSceneLayout->setCurrentIndex(id);
        mTimelineLayout->setCurrentIndex(id);
    }

    LayoutData* newLayout() {
        const QString name = "Layout " + QString::number(mNumberLayouts);
        const auto lay = std::make_shared<LayoutData>(name);
        mSceneLayout->insertWidget(0, lay->fSceneLayout);
        mTimelineLayout->insertWidget(0, lay->fTimelineLayout);
        const auto it = mLayouts.insert(mLayouts.begin(), lay);
        mComboBox->insertItem(0, name);
        if(mCurrentId != -1) mCurrentId++;
        mNumberLayouts++;
        return it->get();
    }

    void newLayoutAction() {
        newLayout();
        setCurrent(0);
    }

    void newForScene(Canvas* const scene) {
        const auto lay = std::make_shared<LayoutData>(scene);
        mLayouts.insert(mLayouts.begin() + mNumberLayouts, lay);
        mSceneLayout->insertWidget(mNumberLayouts, lay->fSceneLayout);
        mTimelineLayout->insertWidget(mNumberLayouts, lay->fTimelineLayout);
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
            if(lay->fScene == scene) {
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
        const auto canvasWidget = mSceneLayout->widget(id);
        const auto timelineWidget = mTimelineLayout->widget(id);
        mSceneLayout->removeWidget(canvasWidget);
        mTimelineLayout->removeWidget(timelineWidget);
        delete canvasWidget;
        delete timelineWidget;
        if(mCurrentId == id) {
            const int newId = qMin(qMax(0, mCurrentId - 1), int(mLayouts.size()) - 1);
            mCurrentId = -1;
            setCurrent(newId);
        } else if(mCurrentId > id) mCurrentId--;
    }

    Document& mDocument;
    AudioHandler& mAudioHandler;

    int mCurrentId = -1;
    int mNumberLayouts = 0;
    std::vector<stdsptr<LayoutData>> mLayouts;

    QWidget* mComboWidget;
    QComboBox* mComboBox;
    QStackedWidget* mSceneLayout;
    QStackedWidget* mTimelineLayout;
};

#endif // LAYOUTHANDLER_H
