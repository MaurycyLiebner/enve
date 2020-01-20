// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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
#include "GUI/BoxesList/boxscroller.h"
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

    const auto ruleActionAdder = [this, objectsMenu](
            const SWT_BoxRule rule, const QString& text) {
        const auto slot = [this, rule]() { setBoxRule(rule); };
        const auto action = objectsMenu->addAction(text, this, slot);
        action->setCheckable(true);
        connect(this, &TimelineWidget::boxRuleChanged,
                action, [action, rule](const SWT_BoxRule setRule) {
            action->setChecked(rule == setRule);
        });
        return action;
    };

    ruleActionAdder(SWT_BoxRule::all, "All")->setChecked(true);
    ruleActionAdder(SWT_BoxRule::selected, "Selected");
    ruleActionAdder(SWT_BoxRule::animated, "Animated");
    ruleActionAdder(SWT_BoxRule::notAnimated, "Not Animated");
    ruleActionAdder(SWT_BoxRule::visible, "Visible");
    ruleActionAdder(SWT_BoxRule::hidden, "Hidden");
    ruleActionAdder(SWT_BoxRule::unlocked, "Unlocked");
    ruleActionAdder(SWT_BoxRule::locked, "Locked");

    QMenu * const targetMenu = settingsMenu->addMenu("Target");

    const auto targetActionAdder = [this, targetMenu](
            const SWT_Target target, const QString& text) {
        const auto slot = [this, target]() { setTarget(target); };
        const auto action = targetMenu->addAction(text, this, slot);
        action->setCheckable(true);
        connect(this, &TimelineWidget::targetChanged,
                action, [action, target](const SWT_Target setTarget) {
            action->setChecked(target == setTarget);
        });
        return action;
    };

    //targetActionAdder(SWT_Target::all, "All");
    targetActionAdder(SWT_Target::canvas, "Current Scene")->setChecked(true);
    targetActionAdder(SWT_Target::group, "Current Group");

    QMenu * const typeMenu = settingsMenu->addMenu("Type");

    const auto typeActionAdder = [this, typeMenu](
            const SWT_Type type, const QString& text) {
        const auto slot = [this, type]() { setType(type); };
        const auto action = typeMenu->addAction(text, this, slot);
        action->setCheckable(true);
        connect(this, &TimelineWidget::typeChanged,
                action, [action, type](const SWT_Type setType) {
            action->setChecked(type == setType);
        });
        return action;
    };

    typeActionAdder(SWT_Type::all, "All")->setChecked(true);
    typeActionAdder(SWT_Type::sound, "Sound");
    typeActionAdder(SWT_Type::graphics, "Graphics");

    settingsMenu->addSeparator();

    {
        const auto op = [this]() {
            setBoxRule(SWT_BoxRule::all);
            setTarget(SWT_Target::canvas);
            setType(SWT_Type::all);
        };
        const auto act = settingsMenu->addAction("Reset", this, op);
        const auto can = [this]() {
            const auto rules = mBoxesListVisible->getCurrentRulesCollection();
            return rules.fRule != SWT_BoxRule::all ||
                   rules.fTarget != SWT_Target::canvas ||
                   rules.fType != SWT_Type::all;
        };
        const auto setEnabled = [act, can]() { act->setEnabled(can()); };
        connect(this, &TimelineWidget::typeChanged, act, setEnabled);
        connect(this, &TimelineWidget::targetChanged, act, setEnabled);
        connect(this, &TimelineWidget::boxRuleChanged, act, setEnabled);
    }

    //QMenu *viewMenu = mBoxesListMenuBar->addMenu("View");
    mGraphAct = mCornerMenuBar->addAction("Graph");
    mGraphAct->setIcon(QIcon(iconsDir + "/graphDisabled.png"));
    mGraphAct->setCheckable(true);
    connect(mGraphAct, &QAction::toggled,
            this, &TimelineWidget::setGraphEnabled);

    mCornerMenuBar->setCornerWidget(menu);

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
    mBoxesListVisible->setCurrentRule(SWT_BoxRule::all);
    mBoxesListVisible->setCurrentTarget(nullptr, SWT_Target::canvas);

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
        if(rules.fTarget == SWT_Target::canvas) {
            mBoxesListVisible->scheduleContentUpdateIfIsCurrentTarget(
                        scene, SWT_Target::canvas);
        } else if(rules.fTarget == SWT_Target::group) {
            mBoxesListVisible->scheduleContentUpdateIfIsCurrentTarget(
                        scene->getCurrentGroup(), SWT_Target::group);
        }
        connect(scene, &Canvas::currentContainerSet, this,
                [this](ContainerBox* const container) {
            mBoxesListVisible->scheduleContentUpdateIfIsCurrentTarget(
                        container, SWT_Target::group);
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

    const auto rules = mBoxesListVisible->getCurrentRulesCollection();
    dst.write(&rules.fRule, sizeof(SWT_BoxRule));
    dst.write(&rules.fType, sizeof(SWT_Type));
    dst.write(&rules.fTarget, sizeof(SWT_Target));
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

    if(src.evFileVersion() > 6) {
        SWT_BoxRule boxRule;
        SWT_Type type;
        SWT_Target target;
        src.read(&boxRule, sizeof(SWT_BoxRule));
        src.read(&type, sizeof(SWT_Type));
        src.read(&target, sizeof(SWT_Target));
        setBoxRule(boxRule);
        setType(type);
        setTarget(target);
    }

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

    mKeysView->setFramesRange({minViewedFrame, maxViewedFrame});
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
    mBoxesListVisible->setCurrentRule(rule);
    emit boxRuleChanged(rule);
    Document::sInstance->updateScenes();
}

void TimelineWidget::setTarget(const SWT_Target target) {
    switch(target) {
    case SWT_Target::all:
        mBoxesListVisible->setCurrentTarget(&mDocument, SWT_Target::all);
        break;
    case SWT_Target::canvas:
        mBoxesListVisible->setCurrentTarget(mCurrentScene, SWT_Target::canvas);
        break;
    case SWT_Target::group:
        const auto group = mCurrentScene ? mCurrentScene->getCurrentGroup() :
                                           nullptr;
        mBoxesListVisible->setCurrentTarget(group, SWT_Target::group);
        break;
    }

    emit targetChanged(target);
    Document::sInstance->updateScenes();
}

void TimelineWidget::setType(const SWT_Type type) {
    mBoxesListVisible->setCurrentType(type);
    emit typeChanged(type);
    Document::sInstance->updateScenes();
}

void TimelineWidget::setSearchText(const QString &text) {
    mBoxesListVisible->setCurrentSearchText(text);
    Document::sInstance->actionFinished();
}

void TimelineWidget::setViewedFrameRange(const FrameRange& range) {
    mFrameRangeScrollBar->setViewedFrameRange(range);
    mFrameScrollBar->setDisplayedFrameRange(range);
    mKeysView->setFramesRange(range);
}

void TimelineWidget::setCanvasFrameRange(const FrameRange& range) {
    mFrameRangeScrollBar->setDisplayedFrameRange(range);
    setViewedFrameRange(mFrameRangeScrollBar->getViewedRange());
    mFrameRangeScrollBar->setCanvasFrameRange(range);
    mFrameScrollBar->setCanvasFrameRange(range);
}
