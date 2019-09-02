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

#include "timelinewidget.h"
#include "animationwidgetscrollbar.h"
#include "mainwindow.h"
#include "timelinedockwidget.h"
#include "GUI/BoxesList/boxsinglewidget.h"
#include "singlewidgetabstraction.h"
#include "GUI/BoxesList/boxscrollwidget.h"
#include "keysview.h"
#include "GUI/BoxesList/boxscrollarea.h"
#include "GUI/BoxesList/boxscrollwidgetvisiblepart.h"
#include "canvaswindow.h"
#include "animationdockwidget.h"
#include "GUI/global.h"
#include "canvas.h"
#include "scenechooser.h"
#include "changewidthwidget.h"
#include <QToolButton>

TimelineWidget::TimelineWidget(Document &document,
                               QMenuBar * const menu,
                               QWidget *parent) :
    QWidget(parent), mDocument(document) {
    mMainLayout = new QGridLayout(this);
    mMainLayout->setSpacing(0);
    mMainLayout->setMargin(0);

    mMenuLayout = new QHBoxLayout();
    mMenuLayout->setSpacing(0);
    mMenuLayout->setMargin(0);
    mBoxesListMenuBar = new QMenuBar(this);
    mBoxesListMenuBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    ((QToolButton*)mBoxesListMenuBar->children()[0])->setStyleSheet(
                "QToolButton {"
                    "padding: 0px 0px;"
                    "background: transparent;"
                    "border-radius: 4px;"
                    "margin-top: 0;"
                    "border-bottom-right-radius: 0px;"
                    "border-bottom-left-radius: 0px;"
                    "padding-bottom: 0;"
                    "margin-bottom: 0;"
                "}"
                "QToolButton:hover {"
                    "background-color: rgba(0, 0, 0, 30);"
                "}"

                "QToolButton:pressed {"
                    "background-color: rgba(0, 0, 0, 50);"
                "}"

                "QToolButton:checked {"
                    "background-color: rgb(60, 60, 60);"
                    "color: white;"
                "}");
    mBoxesListMenuBar->addSeparator();
    mSceneChooser = new SceneChooser(mDocument, true,
                                     mBoxesListMenuBar);
    mBoxesListMenuBar->addMenu(mSceneChooser);

    mCornerMenuBar = new QMenuBar(this);
    mCornerMenuBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    mCornerMenuBar->setStyleSheet("QMenuBar::item { padding: 1px 0px; }");

    const auto iconsDir = eSettings::sIconsDir();

    QMenu * const settingsMenu = mCornerMenuBar->addMenu(
                QIcon(iconsDir + "/settings_dots.png"), "Settings");
    QMenu * const objectsMenu = settingsMenu->addMenu("State");
    objectsMenu->addAction("All", this,
                           &TimelineWidget::setRuleNone);
    objectsMenu->addAction("Selected", this,
                           &TimelineWidget::setRuleSelected);
    objectsMenu->addAction("Animated", this,
                           &TimelineWidget::setRuleAnimated);
    objectsMenu->addAction("Not Animated", this,
                           &TimelineWidget::setRuleNotAnimated);
    objectsMenu->addAction("Visible", this,
                           &TimelineWidget::setRuleVisible);
    objectsMenu->addAction("Hidden", this,
                           &TimelineWidget::setRuleHidden);
    objectsMenu->addAction("Unlocked", this,
                           &TimelineWidget::setRuleUnloced);
    objectsMenu->addAction("Locked", this,
                           &TimelineWidget::setRuleLocked);

    QMenu * const targetMenu = settingsMenu->addMenu("Target");
//    targetMenu->addAction("All", this,
//                          &BoxesListKeysViewWidget::setTargetAll);
    targetMenu->addAction("Current Scene", this,
                          &TimelineWidget::setTargetCurrentCanvas);
    targetMenu->addAction("Current Group", this,
                          &TimelineWidget::setTargetCurrentGroup);
    QMenu * const typeMenu = settingsMenu->addMenu("Type");
    typeMenu->addAction("All", this, &TimelineWidget::setTypeAll);
    typeMenu->addAction("Graphics", this,
                        &TimelineWidget::setTypeGraphics);
    typeMenu->addAction("Sound", this,
                        &TimelineWidget::setTypeSound);

    //QMenu *viewMenu = mBoxesListMenuBar->addMenu("View");
    mGraphAct = mCornerMenuBar->addAction("Graph");
    mGraphAct->setIcon(QIcon(iconsDir + "/graphDisabled.png"));
    mGraphAct->setCheckable(true);
    connect(mGraphAct, &QAction::toggled,
            this, &TimelineWidget::setGraphEnabled);

    mCornerMenuBar->setCornerWidget(menu);
//    mCornerMenuBar->addSeparator();
//    mCornerMenuBar->addAction(" + ", this,
//                              &TimelineWidget::addNewBelowThis);
//    mCornerMenuBar->addAction(" - ", this,
//                              &TimelineWidget::removeThis);
//    mCornerMenuBar->addSeparator();

    mSearchLine = new QLineEdit("", mBoxesListMenuBar);
    mSearchLine->setMinimumHeight(0);
    mSearchLine->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    MainWindow::sGetInstance()->installLineFilter(mSearchLine);
    mSearchLine->setStyleSheet("background-color: white;"
                               "color: black;"
                               "border-radius: 0;"
                               "border: 0;"
                               "border-right: 1px solid black;"
                               "border-left: 1px solid black;"
                               "border-bottom: 1px solid black;"
                               "margin: 0;");
    connect(mSearchLine, &QLineEdit::textChanged,
            this, &TimelineWidget::setSearchText);
    mSearchLine->setFocusPolicy(Qt::ClickFocus);

    mMenuLayout->addWidget(mBoxesListMenuBar);
    mMenuLayout->addWidget(mSearchLine);
    mMenuLayout->addWidget(mCornerMenuBar);

    mMenuWidget = new QWidget(this);
    mMenuWidget->setLayout(mMenuLayout);

    mBoxesListScrollArea = new ScrollArea(this);

    mBoxesListWidget = new BoxScrollWidget(mDocument, mBoxesListScrollArea);
    mBoxesListVisible = mBoxesListWidget->getVisiblePartWidget();
    mBoxesListVisible->setCurrentRule(SWT_BR_ALL);
    mBoxesListVisible->setCurrentTarget(nullptr, SWT_TARGET_CURRENT_CANVAS);

    mBoxesListScrollArea->setWidget(mBoxesListWidget);
    mMainLayout->addWidget(mMenuWidget, 0, 0);
    mMainLayout->addWidget(mBoxesListScrollArea, 1, 0);

    mKeysViewLayout = new QVBoxLayout();
    mKeysView = new KeysView(mBoxesListVisible, this);
    mKeysViewLayout->addWidget(mKeysView);
    mAnimationDockWidget = new AnimationDockWidget(this, mKeysView);
    mAnimationDockWidget->hide();
    mMainLayout->addLayout(mKeysViewLayout, 1, 1);

    const auto keysViewScrollbarLayout = new QHBoxLayout();
    const auto layoutT = new QVBoxLayout();
    layoutT->setAlignment(Qt::AlignBottom);
    layoutT->addWidget(mAnimationDockWidget);
    keysViewScrollbarLayout->addLayout(layoutT);
    mKeysView->setLayout(keysViewScrollbarLayout);
    keysViewScrollbarLayout->setAlignment(Qt::AlignRight);
    keysViewScrollbarLayout->addWidget(
                mBoxesListScrollArea->verticalScrollBar());
    mBoxesListScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    keysViewScrollbarLayout->setContentsMargins(0, 0, 0, 0);

    connect(mBoxesListScrollArea->verticalScrollBar(),
            &QScrollBar::valueChanged,
            mBoxesListWidget, &BoxScrollWidget::changeVisibleTop);
    connect(mBoxesListScrollArea, &ScrollArea::heightChanged,
            mBoxesListWidget, &BoxScrollWidget::changeVisibleHeight);
    connect(mBoxesListScrollArea, &ScrollArea::widthChanged,
            mBoxesListWidget, &BoxScrollWidget::setWidth);

    connect(mBoxesListScrollArea->verticalScrollBar(),
            &QScrollBar::valueChanged,
            this, &TimelineWidget::moveSlider);
    connect(mKeysView, &KeysView::wheelEventSignal,
            mBoxesListScrollArea, &ScrollArea::callWheelEvent);
    connect(mKeysView, &KeysView::changedViewedFrames,
             this, &TimelineWidget::setViewedFrameRange);

    connect(mSceneChooser, &SceneChooser::currentChanged,
            this, &TimelineWidget::setCurrentScene);

    mBoxesListScrollArea->setFixedWidth(20*MIN_WIDGET_DIM);

    setLayout(mMainLayout);

    mFrameScrollBar = new FrameScrollBar(1, 1, 0, false, false, this);
    mFrameScrollBar->setSizePolicy(QSizePolicy::Minimum,
                                   QSizePolicy::Preferred);
//    connect(MemoryHandler::sGetInstance(), &MemoryHandler::memoryFreed,
//            frameScrollBar,
//            qOverload<>(&FrameScrollBar::update));
    connect(mFrameScrollBar, &FrameScrollBar::triggeredFrameRangeChange,
            this, [this](const FrameRange& range){
        const auto scene = mSceneChooser->getCurrentScene();
        if(scene) scene->anim_setAbsFrame(range.fMin);
        Document::sInstance->actionFinished();
    });
    mMainLayout->addWidget(mFrameScrollBar, 0, 1);

    mFrameRangeScrollBar = new FrameScrollBar(20, 200, MIN_WIDGET_DIM*2/3,
                                              true, true, this);

    connect(mFrameRangeScrollBar, &FrameScrollBar::triggeredFrameRangeChange,
            this, &TimelineWidget::setViewedFrameRange);
    mKeysViewLayout->addWidget(mFrameRangeScrollBar);
    mSceneChooser->setCurrentScene(mDocument.fActiveScene);


    const auto chww = new ChangeWidthWidget(this);
    chww->show();
    chww->updatePos();
    chww->raise();
    connect(chww, &ChangeWidthWidget::widthSet,
            this, &TimelineWidget::setBoxesListWidth);
    setBoxesListWidth(chww->getCurrentWidth());
}

void TimelineWidget::setCurrentScene(Canvas * const scene) {
    if(scene == mCurrentScene) return;
    if(mCurrentScene) {
        disconnect(mCurrentScene, nullptr, mFrameScrollBar, nullptr);
        disconnect(mCurrentScene, nullptr, this, nullptr);
    }

    mCurrentScene = scene;
    mSceneChooser->setCurrentScene(scene);
    mFrameScrollBar->setCurrentCanvas(scene);
    mBoxesListWidget->setCurrentScene(scene);
    mKeysView->setCurrentScene(scene);
    if(scene) {
        setCanvasFrameRange(scene->getFrameRange());
        mFrameScrollBar->setFirstViewedFrame(scene->getCurrentFrame());
        mFrameRangeScrollBar->setFirstViewedFrame(scene->getCurrentFrame());
        setViewedFrameRange(mFrameRangeScrollBar->getViewedRange());

        connect(scene, &Canvas::currentFrameChanged,
                mFrameScrollBar, &FrameScrollBar::setFirstViewedFrame);
        connect(scene, &Canvas::newFrameRange,
                this, &TimelineWidget::setCanvasFrameRange);


        const auto rules = mBoxesListVisible->getCurrentRulesCollection();
        if(rules.fTarget == SWT_TARGET_CURRENT_CANVAS) {
            mBoxesListVisible->scheduleContentUpdateIfIsCurrentTarget(
                        scene, SWT_TARGET_CURRENT_CANVAS);
        } else if(rules.fTarget == SWT_TARGET_CURRENT_GROUP) {
            mBoxesListVisible->scheduleContentUpdateIfIsCurrentTarget(
                        scene->getCurrentGroup(), SWT_TARGET_CURRENT_GROUP);
        }
        connect(scene, &Canvas::currentContainerSet, this,
                [this](ContainerBox* const container) {
            mBoxesListVisible->scheduleContentUpdateIfIsCurrentTarget(
                        container, SWT_TARGET_CURRENT_GROUP);
        });
        connect(scene, &Canvas::requestUpdate, this, [this]() {
            mFrameScrollBar->update();
            mBoxesListVisible->update();
            update();
        });
    }
}

void TimelineWidget::setGraphEnabled(const bool enabled) {
    mKeysView->setGraphViewed(enabled);
    mAnimationDockWidget->setVisible(enabled);
    const auto iconsDir = eSettings::sIconsDir();
    if(enabled) mGraphAct->setIcon(QIcon(iconsDir + "/graphEnabled.png"));
    else mGraphAct->setIcon(QIcon(iconsDir + "/graphDisabled.png"));
}

void TimelineWidget::writeState(eWriteStream &dst) const {
    const int id = mBoxesListVisible->getId();
    for(const auto& scene : mDocument.fScenes) {
        const auto abs = scene->SWT_getAbstractionForWidget(id);
        abs->writeAll(dst);
    }

    if(mCurrentScene) {
        dst << mCurrentScene->getWriteId();
        dst << mCurrentScene->getDocumentId();
    } else {
        dst << -1;
        dst << -1;
    }

    dst << mSearchLine->text();
    dst << mBoxesListScrollArea->verticalScrollBar()->sliderPosition();

    dst << mFrameScrollBar->getFirstViewedFrame();
    dst << mFrameRangeScrollBar->getFirstViewedFrame();
    dst << mFrameRangeScrollBar->getLastViewedFrame();
}

void TimelineWidget::readState(eReadStream &src) {
    const int id = mBoxesListVisible->getId();
    BoundingBox::sForEveryReadBox([id, &src](BoundingBox* const box) {
        if(!box->SWT_isCanvas()) return;
        const auto scene = static_cast<Canvas*>(box);
        const auto abs = scene->SWT_getAbstractionForWidget(id);
        if(abs) abs->readAll(src);
    });

    int sceneReadId; src >> sceneReadId;
    int sceneDocumentId; src >> sceneDocumentId;

    QString search; src >> search;
    int sliderPos; src >> sliderPos;

    int frame; src >> frame;
    int minViewedFrame; src >> minViewedFrame;
    int maxViewedFrame; src >> maxViewedFrame;

    BoundingBox* sceneBox = nullptr;;
    if(sceneReadId != -1)
        sceneBox = BoundingBox::sGetBoxByReadId(sceneReadId);
    if(!sceneBox && sceneDocumentId != -1)
        sceneBox = BoundingBox::sGetBoxByDocumentId(sceneDocumentId);

    setCurrentScene(qobject_cast<Canvas*>(sceneBox));

    mSearchLine->setText(search);

    mBoxesListScrollArea->verticalScrollBar()->setSliderPosition(sliderPos);
    mKeysView->setViewedVerticalRange(sliderPos, sliderPos + mBoxesListScrollArea->height());

    mFrameScrollBar->setFirstViewedFrame(frame);
    mFrameScrollBar->setDisplayedFrameRange({minViewedFrame, maxViewedFrame});
    mFrameRangeScrollBar->setViewedFrameRange({minViewedFrame, maxViewedFrame});
}

void TimelineWidget::moveSlider(int val) {
    int diff = val%MIN_WIDGET_DIM;
    if(diff != 0) {
        val -= diff;
        mBoxesListScrollArea->verticalScrollBar()->setSliderPosition(val);
    }
    mKeysView->setViewedVerticalRange(val, val + mBoxesListScrollArea->height());
}

void TimelineWidget::setBoxesListWidth(const int width) {
    mMenuWidget->setFixedWidth(width);
    mBoxesListScrollArea->setFixedWidth(width);
}

void TimelineWidget::setBoxRule(const SWT_BoxRule rule) {
    mBoxesListWidget->getVisiblePartWidget()->setCurrentRule(rule);
    Document::sInstance->actionFinished();
}

void TimelineWidget::setRuleNone() {
    setBoxRule(SWT_BR_ALL);
}

void TimelineWidget::setRuleSelected() {
    setBoxRule(SWT_BR_SELECTED);
}

void TimelineWidget::setRuleAnimated() {
    setBoxRule(SWT_BR_ANIMATED);
}

void TimelineWidget::setRuleNotAnimated() {
    setBoxRule(SWT_BR_NOT_ANIMATED);
}

void TimelineWidget::setRuleVisible() {
    setBoxRule(SWT_BR_VISIBLE);
}

void TimelineWidget::setRuleHidden() {
    setBoxRule(SWT_BR_HIDDEN);
}

void TimelineWidget::setRuleUnloced() {
    setBoxRule(SWT_BR_UNLOCKED);
}

void TimelineWidget::setRuleLocked() {
    setBoxRule(SWT_BR_LOCKED);
}

void TimelineWidget::setTargetAll() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(&mDocument, SWT_TARGET_ALL);
    Document::sInstance->actionFinished();
}

void TimelineWidget::setTargetCurrentCanvas() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(
                mCurrentScene,
                SWT_TARGET_CURRENT_CANVAS);
    Document::sInstance->actionFinished();
}

void TimelineWidget::setTargetCurrentGroup() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(
                mCurrentScene->getCurrentGroup(),
                SWT_TARGET_CURRENT_GROUP);
    Document::sInstance->actionFinished();
}

void TimelineWidget::setCurrentType(const SWT_Type type) {
    mBoxesListWidget->getVisiblePartWidget()->setCurrentType(type);
    Document::sInstance->actionFinished();
}

void TimelineWidget::setTypeAll() {
    setCurrentType(SWT_TYPE_ALL);
}

void TimelineWidget::setTypeSound() {
    setCurrentType(SWT_TYPE_SOUND);
}

void TimelineWidget::setTypeGraphics() {
    setCurrentType(SWT_TYPE_GRAPHICS);
}

void TimelineWidget::setSearchText(const QString &text) {
    mBoxesListWidget->getVisiblePartWidget()->setCurrentSearchText(text);
    Document::sInstance->actionFinished();
}

void TimelineWidget::setViewedFrameRange(const FrameRange& range) {
    mFrameRangeScrollBar->setViewedFrameRange(range);
    mFrameScrollBar->setDisplayedFrameRange(range);
    mKeysView->setFramesRange(range);
}

void TimelineWidget::setCanvasFrameRange(
        const FrameRange& range) {
    mFrameRangeScrollBar->setDisplayedFrameRange(range);
    setViewedFrameRange(mFrameRangeScrollBar->getViewedRange());
    mFrameRangeScrollBar->setCanvasFrameRange(range);
    mFrameScrollBar->setCanvasFrameRange(range);
}
