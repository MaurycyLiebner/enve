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

#include "canvaswindowwrapper.h"

#include <QComboBox>
#include <QLabel>

#include "GUI/newcanvasdialog.h"
#include "canvaswindow.h"
#include "document.h"
#include "scenechooser.h"
#include "audiohandler.h"

class CanvasWrapperMenuBar : public StackWrapperMenu {
    friend struct CWWidgetStackLayoutItem;
    friend class CanvasWindowWrapper;
public:
    CanvasWrapperMenuBar(Document& document, CanvasWindow * const window) :
        mDocument(document), mWindow(window) {
        mSceneMenu = new SceneChooser(mDocument, false, this);
        addMenu(mSceneMenu);
        connect(mSceneMenu, &SceneChooser::currentChanged,
                this, &CanvasWrapperMenuBar::setCurrentScene);
    }
private:
    void setCurrentScene(Canvas * const scene) {
        mWindow->setCurrentCanvas(scene);
        mSceneMenu->setCurrentScene(scene);
    }

    Canvas* getCurrentScene() const { return mCurrentScene; }

    Document& mDocument;
    CanvasWindow* const mWindow;
    SceneChooser * mSceneMenu;
    Canvas * mCurrentScene = nullptr;
    std::map<Canvas*, QAction*> mSceneToAct;
};

CanvasWindowWrapper::CanvasWindowWrapper(Document * const document,
                                         CWWidgetStackLayoutItem* const layItem,
                                         QWidget * const parent) :
    StackWidgetWrapper(
        layItem,
        []() {
            const auto rPtr = new CWWidgetStackLayoutItem();
            return std::unique_ptr<WidgetStackLayoutItem>(rPtr);
        },
        [document](WidgetStackLayoutItem* const layItem,
                                 QWidget * const parent) {
            const auto cLayItem = static_cast<CWWidgetStackLayoutItem*>(layItem);
            const auto derived = new CanvasWindowWrapper(document, cLayItem, parent);
            return static_cast<StackWidgetWrapper*>(derived);
        },
        [document](StackWidgetWrapper * toSetup) {
            const auto window = new CanvasWindow(*document, toSetup);
            toSetup->setCentralWidget(window);
            toSetup->setMenuBar(new CanvasWrapperMenuBar(*document, window));
        }, parent) {}

void CanvasWindowWrapper::setScene(Canvas * const scene) {
    const auto menu = static_cast<CanvasWrapperMenuBar*>(getMenuBar());
    menu->setCurrentScene(scene);
}

Canvas* CanvasWindowWrapper::getScene() const {
    const auto menu = static_cast<CanvasWrapperMenuBar*>(getMenuBar());
    return menu->getCurrentScene();
}

void CanvasWindowWrapper::saveDataToLayout() const {
    const auto lItem = static_cast<CWWidgetStackLayoutItem*>(getLayoutItem());
    const auto sceneWidget = getSceneWidget();
    lItem->setTransform(sceneWidget->getViewTransform());
    lItem->setScene(sceneWidget->getCurrentCanvas());
}

CanvasWindow* CanvasWindowWrapper::getSceneWidget() const {
    return static_cast<CanvasWindow*>(getCentralWidget());
}

void CWWidgetStackLayoutItem::clear() {
    SceneWidgetStackLayoutItem::clear();
    mTransform.reset();
}

QWidget* CWWidgetStackLayoutItem::create(Document& document,
                                         QWidget* const parent,
                                         QLayout* const layout) {
    const auto cwWrapper = new CanvasWindowWrapper(&document, this, parent);
    if(layout) {
        cwWrapper->disableClose();
        layout->addWidget(cwWrapper);
    }
    cwWrapper->setScene(mScene);
    const auto cw = cwWrapper->getSceneWidget();
    if(mTransformSet) cw->setViewTransform(mTransform);
    else cw->requestFitCanvasToSize();
    return cwWrapper;
}

void CWWidgetStackLayoutItem::write(eWriteStream &dst) const {
    SceneWidgetStackLayoutItem::write(dst);
    dst.write(rcConstChar(&mTransform), sizeof(QMatrix));
}

void CWWidgetStackLayoutItem::read(eReadStream& src) {
    SceneWidgetStackLayoutItem::read(src);
    QMatrix transform;
    src.read(rcChar(&transform), sizeof(QMatrix));
    setTransform(transform);
}

void CWWidgetStackLayoutItem::setTransform(const QMatrix& transform) {
    mTransform = transform;
    mTransformSet = true;
}

void SceneWidgetStackLayoutItem::clear() {
    setScene(nullptr);
}

void SceneWidgetStackLayoutItem::write(eWriteStream& dst) const {
    const int sceneId = mScene ? mScene->getWriteId() : -1;
    dst << sceneId;
}

void SceneWidgetStackLayoutItem::read(eReadStream &src) {
    int sceneId; src >> sceneId;
    const auto sceneBox = BoundingBox::sGetBoxByReadId(sceneId);
    const auto scene = dynamic_cast<Canvas*>(sceneBox);
    setScene(scene);
}

void SceneWidgetStackLayoutItem::setScene(Canvas * const scene) {
    mScene = scene;
}
