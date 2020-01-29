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

#include "timelinedockwidget.h"
#include "mainwindow.h"
#include <QKeyEvent>
#include "canvaswindow.h"
#include "canvas.h"
#include "animationdockwidget.h"
#include <QScrollBar>
#include "GUI/BoxesList/boxscrollwidget.h"
#include "GUI/BoxesList/boxsinglewidget.h"
#include "widgetstack.h"
#include "actionbutton.h"
#include "GUI/RenderWidgets/renderwidget.h"
#include "timelinewidget.h"
#include "animationwidgetscrollbar.h"
#include "GUI/global.h"
#include "renderinstancesettings.h"
#include "Private/document.h"
#include "layouthandler.h"
#include "memoryhandler.h"
#include "switchbutton.h"

TimelineDockWidget::TimelineDockWidget(Document& document,
                                       LayoutHandler * const layoutH,
                                       MainWindow * const parent) :
    QWidget(parent), mDocument(document), mMainWindow(parent),
    mTimelineLayout(layoutH->timelineLayout()) {
    connect(RenderHandler::sInstance, &RenderHandler::previewFinished,
            this, &TimelineDockWidget::previewFinished);
    connect(RenderHandler::sInstance, &RenderHandler::previewBeingPlayed,
            this, &TimelineDockWidget::previewBeingPlayed);
    connect(RenderHandler::sInstance, &RenderHandler::previewBeingRendered,
            this, &TimelineDockWidget::previewBeingRendered);
    connect(RenderHandler::sInstance, &RenderHandler::previewPaused,
            this, &TimelineDockWidget::previewPaused);

    connect(Document::sInstance, &Document::canvasModeSet,
            this, &TimelineDockWidget::updateButtonsVisibility);

    setFocusPolicy(Qt::NoFocus);

    setMinimumSize(10*MIN_WIDGET_DIM, 12*MIN_WIDGET_DIM);
    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);
    mMainLayout->setSpacing(0);
    mMainLayout->setMargin(0);

    mResolutionComboBox = new QComboBox(this);
    mResolutionComboBox->addItem("100 %");
    mResolutionComboBox->addItem("75 %");
    mResolutionComboBox->addItem("50 %");
    mResolutionComboBox->addItem("25 %");
    mResolutionComboBox->setEditable(true);
    mResolutionComboBox->lineEdit()->setInputMask("D00 %");
    mResolutionComboBox->setCurrentText("100 %");
    MainWindow::sGetInstance()->installNumericFilter(mResolutionComboBox);
    mResolutionComboBox->setInsertPolicy(QComboBox::NoInsert);
    mResolutionComboBox->setSizePolicy(QSizePolicy::Maximum,
                                       QSizePolicy::Maximum);
    connect(mResolutionComboBox, &QComboBox::currentTextChanged,
            this, &TimelineDockWidget::setResolutionFractionText);

    const int iconSize = 5*MIN_WIDGET_DIM/4;
    const QString iconsDir = eSettings::sIconsDir() + "/toolbarButtons";

    mPlayFromBeginningButton = new ActionButton(
                iconsDir + "/preview.png",
                gSingleLineTooltip("Play From the Beginning"), this);
    connect(mPlayFromBeginningButton, &ActionButton::pressed,
            this, [this]() {
        const auto scene = mDocument.fActiveScene;
        if(!scene) return;
        scene->anim_setAbsFrame(scene->getFrameRange().fMin);
        renderPreview();
    });

    mPlayButton = SwitchButton::sCreate2Switch(iconsDir + "/play.png",
                                               iconsDir + "/pause.png",
                                               gSingleLineTooltip("Render Preview", "Space"), this);
    mStopButton = new ActionButton(iconsDir + "/stop.png",
                                   gSingleLineTooltip("Stop Preview", "Esc"), this);
    connect(mStopButton, &ActionButton::pressed,
            this, &TimelineDockWidget::interruptPreview);

    mLocalPivot = SwitchButton::sCreate2Switch(
                iconsDir + "/pivotGlobal.png", iconsDir + "/pivotLocal.png",
                gSingleLineTooltip("Pivot Global / Local", "P"), this);
    mLocalPivot->setState(mDocument.fLocalPivot);
    connect(mLocalPivot, &SwitchButton::toggled,
            this, &TimelineDockWidget::setLocalPivot);

    mNodeVisibility = new SwitchButton(
                gSingleLineTooltip("Node visibility", "N"), this);
    mNodeVisibility->addState(iconsDir + "/dissolvedAndNormalNodes.png");
    mNodeVisibility->addState(iconsDir + "/dissolvedNodesOnly.png");
    mNodeVisibility->addState(iconsDir + "/normalNodesOnly.png");
    connect(mNodeVisibility, &SwitchButton::toggled,
            this, [this](const int state) {
        mDocument.fNodeVisibility = static_cast<NodeVisiblity>(state);
        Document::sInstance->actionFinished();
    });

    mColorLabel = new TriggerLabel("");
    mColorLabel->setToolTip(gSingleLineTooltip("Current Color", "E"));
    connect(mColorLabel, &TriggerLabel::requestContextMenu,
            this, [this](const QPoint& pos) {
        QMenu menu(this);
        menu.addAction("Bookmark");
        const auto act = menu.exec(pos);
        if(act) {
            if(act->text() == "Bookmark") {
                const QColor& col = mDocument.fBrushColor;
                Document::sInstance->addBookmarkColor(col);
            }
        }
    });
    connect(mColorLabel, &TriggerLabel::triggered, this, [this]() {
        mMainWindow->toggleFillStrokeSettingsDockVisible();
    });

    connect(&mDocument, &Document::brushColorChanged,
            this, &TimelineDockWidget::setBrushColor);
    mBrushLabel = new TriggerLabel("");
    mBrushLabel->setToolTip(gSingleLineTooltip("Current Brush", "B"));
    connect(mBrushLabel, &TriggerLabel::requestContextMenu,
            this, [this](const QPoint& pos) {
        const auto brush = mDocument.fBrush;
        if(!brush) return;
        QMenu menu(this);
        menu.addAction("Bookmark");
        const auto act = menu.exec(pos);
        if(act) {
            if(act->text() == "Bookmark") {
                const auto ctxt = BrushSelectionWidget::sPaintContext;
                const auto wrapper = ctxt->brushWrapper(brush);
                if(wrapper) wrapper->bookmark();
            }
        }
    });
    connect(mBrushLabel, &TriggerLabel::triggered, this, [this]() {
        mMainWindow->togglePaintBrushDockVisible();
    });

    connect(&mDocument, &Document::brushChanged,
            this, &TimelineDockWidget::setBrush);

    mDecBrushSize = new ActionButton(
                iconsDir + "/brush-.png",
                gSingleLineTooltip("Decrease Brush Size", "Q"), this);
    connect(mDecBrushSize, &ActionButton::pressed,
            &mDocument, &Document::decBrushRadius);
    mBrushSizeLabel = new QLabel("0");
    mBrushSizeLabel->setAlignment(Qt::AlignCenter);
    connect(&mDocument, &Document::brushSizeChanged,
            this, [this](const float size) {
        mBrushSizeLabel->setText(QString("%1").arg(exp(qreal(size)), 0, 'f', 2));
    });
    mBrushSizeLabel->setFixedWidth(2*MIN_WIDGET_DIM);
    mIncBrushSize = new ActionButton(
                iconsDir + "/brush+.png",
                gSingleLineTooltip("Increase Brush Size", "W"), this);
    connect(mIncBrushSize, &ActionButton::pressed,
            &mDocument, &Document::incBrushRadius);

    mOnion = SwitchButton::sCreate2Switch(
                iconsDir + "/onionEnabled.png", iconsDir + "/onionDisabled.png",
                gSingleLineTooltip("Onion Skin"), this);
    connect(mOnion, &SwitchButton::toggled,
            &mDocument, &Document::setOnionDisabled);

    mToolBar = new QToolBar(this);
    mToolBar->setMovable(false);

    mToolBar->setIconSize(QSize(iconSize, iconSize));
    mToolBar->addSeparator();

//    mControlButtonsLayout->addWidget(mGoToPreviousKeyButton);
//    mGoToPreviousKeyButton->setFocusPolicy(Qt::NoFocus);
//    mControlButtonsLayout->addWidget(mGoToNextKeyButton);
//    mGoToNextKeyButton->setFocusPolicy(Qt::NoFocus);
    QAction *resA = mToolBar->addAction("Resolution:");
    resA->setToolTip(gSingleLineTooltip("Preview resolution"));
    mToolBar->widgetForAction(resA)->setObjectName("inactiveToolButton");

    mToolBar->addWidget(mResolutionComboBox);
    mToolBar->addSeparator();
    //mResolutionComboBox->setFocusPolicy(Qt::NoFocus);

    mToolBar->addWidget(mPlayFromBeginningButton);
    mToolBar->addWidget(mPlayButton);
    mToolBar->addWidget(mStopButton);

    addSpaceToToolbar()->setText("     ");

    mLocalPivotAct = mToolBar->addWidget(mLocalPivot);
    mNodeVisibilityAct = mToolBar->addWidget(mNodeVisibility);

    const auto brushColorWidget = new QWidget(this);
    brushColorWidget->setContentsMargins(0, 0, 0, 0);
    brushColorWidget->setLayout(new QHBoxLayout);
    brushColorWidget->layout()->setSpacing(0);
    brushColorWidget->layout()->setContentsMargins(0, 0, 0, 0);
    brushColorWidget->layout()->addWidget(mBrushLabel);
    brushColorWidget->layout()->addWidget(mColorLabel);

    mBrushColorWidgetAct = mToolBar->addWidget(brushColorWidget);
    setupSculptBrushColorLabel();
    addSpaceToToolbar();
    mDecBrushSizeAct = mToolBar->addWidget(mDecBrushSize);
    mBrushSizeLabelAct = mToolBar->addWidget(mBrushSizeLabel);
    mIncBrushSizeAct = mToolBar->addWidget(mIncBrushSize);

    setupSculptBrushSizeButtons(iconsDir);

    addSpaceToToolbar();

    mOnionAct = mToolBar->addWidget(mOnion);

    setupSculptModeButtons(iconsDir);
    mSculptModeTargetSeperator = mToolBar->addSeparator();
    setupSculptTargetButtons(iconsDir);
    setupSculptValueSpins();

    mBrushLabel->setStyleSheet("QWidget {"
                                   "background: white;"
                                   "border: 1px solid black;"
                               "}");
    mColorLabel->setObjectName("colorLabel");

    QWidget * const spacerWidget = new QWidget(this);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding,
                                QSizePolicy::Minimum);
    spacerWidget->setStyleSheet("QWidget {"
                                    "background-color: rgba(0, 0, 0, 0)"
                                "}");
    mToolBar->addWidget(spacerWidget);

    mToolBar->addSeparator();

    mTimelineAction = mToolBar->addAction("Timeline", this,
                                          &TimelineDockWidget::setTimelineMode);
    mTimelineAction->setObjectName("customToolButton");
    mTimelineAction->setCheckable(true);
    mTimelineAction->setChecked(true);
    mRenderAction = mToolBar->addAction("Render", this,
                                        &TimelineDockWidget::setRenderMode);
    mRenderAction->setObjectName("customToolButton");
    mRenderAction->setCheckable(true);

    mToolBar->addSeparator();

    mMainLayout->addWidget(mToolBar);

    mPlayFromBeginningButton->setEnabled(false);
    mPlayButton->setEnabled(false);
    mStopButton->setEnabled(false);

    mRenderWidget = new RenderWidget(this);

    connect(&mDocument, &Document::activeSceneSet,
            this, [this](Canvas* const scene) {
        mPlayFromBeginningButton->setEnabled(scene);
        mPlayButton->setEnabled(scene);
        mStopButton->setEnabled(scene);
    });

    mMainLayout->addWidget(mTimelineLayout);
    mMainLayout->addWidget(mRenderWidget);
    mRenderWidget->hide();

    previewFinished();
    //addNewBoxesListKeysViewWidget(1);
    //addNewBoxesListKeysViewWidget(0);

    connect(&mDocument, &Document::activeSceneSet,
            this, &TimelineDockWidget::updateSettingsForCurrentCanvas);

    setBrush(nullptr);
    setBrushColor(Qt::black);
    setSculptBrushColor(Qt::black);

    connect(&mDocument, &Document::sculptModeChanged,
            this, &TimelineDockWidget::sculptModeChanged);
    sculptModeChanged();
    connect(&mDocument, &Document::sculptTargetChanged,
            this, &TimelineDockWidget::sculptTargetChanged);
    sculptTargetChanged();
}

QAction* TimelineDockWidget::addSpaceToToolbar() {
    const auto spaceAct = mToolBar->addAction("");
    const auto spaceWidget = mToolBar->widgetForAction(spaceAct);
    spaceWidget->setObjectName("emptyToolButton");
    return spaceAct;
}

void TimelineDockWidget::setupSculptBrushColorLabel() {
    mSculptColorLabel = new TriggerLabel("");
    mSculptColorLabel->setObjectName("colorLabel");
    mSculptColorLabel->setToolTip(gSingleLineTooltip("Current Color", "E"));
    connect(mSculptColorLabel, &TriggerLabel::requestContextMenu,
            this, [this](const QPoint& pos) {
        QMenu menu(this);
        menu.addAction("Bookmark");
        const auto act = menu.exec(pos);
        if(act) {
            if(act->text() == "Bookmark") {
                const QColor& col = mDocument.fBrushColor;
                Document::sInstance->addBookmarkColor(col);
            }
        }
    });
    connect(mSculptColorLabel, &TriggerLabel::triggered, this, [this]() {
        mMainWindow->toggleFillStrokeSettingsDockVisible();
    });

    connect(&mDocument, &Document::brushColorChanged,
            this, &TimelineDockWidget::setSculptBrushColor);

    mSculptColorLabelAct = mToolBar->addWidget(mSculptColorLabel);
}

void TimelineDockWidget::setupSculptBrushSizeButtons(const QString& iconsDir) {
    mDecSculptBrushSize = new ActionButton(
                iconsDir + "/brush-.png",
                gSingleLineTooltip("Decrease Brush Size", "Q"), this);
    connect(mDecSculptBrushSize, &ActionButton::pressed,
            &mDocument, &Document::decSculptBrushRadius);
    mDecSculptBrushSizeAct = mToolBar->addWidget(mDecSculptBrushSize);

    mSculptBrushSizeLabel = new QLabel("0");
    mSculptBrushSizeLabel->setAlignment(Qt::AlignCenter);
    connect(&mDocument, &Document::sculptBrushSizeChanged,
            this, [this](const qreal size) {
        mSculptBrushSizeLabel->setText(QString("%1").arg(size, 0, 'f', 2));
    });
    mDocument.incSculptBrushRadius();
    mSculptBrushSizeLabel->setFixedWidth(2*MIN_WIDGET_DIM);
    mSculptBrushSizeLabelAct = mToolBar->addWidget(mSculptBrushSizeLabel);

    mIncSculptBrushSize = new ActionButton(
                iconsDir + "/brush+.png",
                gSingleLineTooltip("Increase Brush Size", "W"), this);
    connect(mIncSculptBrushSize, &ActionButton::pressed,
            &mDocument, &Document::incSculptBrushRadius);
    mIncSculptBrushSizeAct = mToolBar->addWidget(mIncSculptBrushSize);
}

void TimelineDockWidget::setupSculptModeButtons(const QString& iconsDir) {
    mDragMode = SwitchButton::sCreate2Switch(
                iconsDir + "/sculpt<-Unchecked.png",
                iconsDir + "/sculpt<-Checked.png",
                gSingleLineTooltip("Drag Shape"), this);
    connect(mDragMode, &SwitchButton::toggled,
            &mDocument, [this] {
        mDocument.setSculptMode(SculptMode::drag);
    });
    mDragModeAct = mToolBar->addWidget(mDragMode);

    mAddMode = SwitchButton::sCreate2Switch(
                iconsDir + "/sculpt+Unchecked.png",
                iconsDir + "/sculpt+Checked.png",
                gSingleLineTooltip("Add Value"), this);
    connect(mAddMode, &SwitchButton::toggled,
            &mDocument, [this] {
        mDocument.setSculptMode(SculptMode::add);
    });
    mAddModeAct = mToolBar->addWidget(mAddMode);

    mReplaceMode = SwitchButton::sCreate2Switch(
                iconsDir + "/sculpt=Unchecked.png",
                iconsDir + "/sculpt=Checked.png",
                gSingleLineTooltip("Replace Value"), this);
    connect(mReplaceMode, &SwitchButton::toggled,
            &mDocument, [this] {
        mDocument.setSculptMode(SculptMode::replace);
    });
    mReplaceModeAct = mToolBar->addWidget(mReplaceMode);

    mSubtractMode = SwitchButton::sCreate2Switch(
                iconsDir + "/sculpt-Unchecked.png",
                iconsDir + "/sculpt-Checked.png",
                gSingleLineTooltip("Subtract Value"), this);
    connect(mSubtractMode, &SwitchButton::toggled,
            &mDocument, [this] {
        mDocument.setSculptMode(SculptMode::subtract);
    });
    mSubtractModeAct = mToolBar->addWidget(mSubtractMode);
}

void TimelineDockWidget::setupSculptTargetButtons(const QString& iconsDir) {
    mPositionTarget = SwitchButton::sCreate2Switch(
                iconsDir + "/positionUnchecked.png",
                iconsDir + "/positionChecked.png",
                gSingleLineTooltip("Position Target"), this);
    connect(mPositionTarget, &SwitchButton::toggled, &mDocument, [this] {
        mDocument.setSculptTarget(SculptTarget::position);
    });
    mPositionTargetAct = mToolBar->addWidget(mPositionTarget);

    mWidthTarget = SwitchButton::sCreate2Switch(
                iconsDir + "/widthUnchecked.png",
                iconsDir + "/widthChecked.png",
                gSingleLineTooltip("Width Target"), this);
    connect(mWidthTarget, &SwitchButton::toggled, &mDocument, [this] {
        mDocument.setSculptTarget(SculptTarget::width);
    });
    mWidthTargetAct = mToolBar->addWidget(mWidthTarget);

    mPressureTarget = SwitchButton::sCreate2Switch(
                iconsDir + "/pressureUnchecked.png",
                iconsDir + "/pressureChecked.png",
                gSingleLineTooltip("Pressure Target"), this);
    connect(mPressureTarget, &SwitchButton::toggled, &mDocument, [this] {
        mDocument.setSculptTarget(SculptTarget::pressure);
    });
    mPressureTargetAct = mToolBar->addWidget(mPressureTarget);

    mSpacingTarget = SwitchButton::sCreate2Switch(
                iconsDir + "/spacingUnchecked.png",
                iconsDir + "/spacingChecked.png",
                gSingleLineTooltip("Spacing Target"), this);
    connect(mSpacingTarget, &SwitchButton::toggled, &mDocument, [this] {
        mDocument.setSculptTarget(SculptTarget::spacing);
    });
    mSpacingTargetAct = mToolBar->addWidget(mSpacingTarget);

    mTimeTarget = SwitchButton::sCreate2Switch(
                iconsDir + "/timeUnchecked.png",
                iconsDir + "/timeChecked.png",
                gSingleLineTooltip("Time Target"), this);
    connect(mTimeTarget, &SwitchButton::toggled, &mDocument, [this] {
        mDocument.setSculptTarget(SculptTarget::time);
    });
    mTimeTargetAct = mToolBar->addWidget(mTimeTarget);

    mColorTarget = SwitchButton::sCreate2Switch(
                iconsDir + "/colorUnchecked.png",
                iconsDir + "/colorChecked.png",
                gSingleLineTooltip("Color Target"), this);
    connect(mColorTarget, &SwitchButton::toggled, &mDocument, [this] {
        mDocument.setSculptTarget(SculptTarget::color);
    });
    mColorTargetAct = mToolBar->addWidget(mColorTarget);
}

void TimelineDockWidget::setupSculptValueSpins() {
    mSculptSpace0 = addSpaceToToolbar();

    const auto valueWidget = new QWidget;
    valueWidget->setObjectName("transparentWidget");
    const auto valueLayout = new QHBoxLayout;
    valueWidget->setLayout(valueLayout);
    const auto valueLabel = new QLabel("value: ");
    mValue = new QDoubleSlider(0, 1, 0.1, this);
    mValue->setDisplayedValue(mDocument.fSculptBrush.value());
    connect(mValue, &QDoubleSlider::valueChanged,
            this, [this](const qreal value) {
        mDocument.fSculptBrush.setValue(value);
    });
    valueLayout->addWidget(valueLabel);
    valueLayout->addWidget(mValue);
    mValueAct = mToolBar->addWidget(valueWidget);

    mSculptSpace1 = addSpaceToToolbar();

    const auto hardnessWidget = new QWidget;
    hardnessWidget->setObjectName("transparentWidget");
    const auto hardnessLayout = new QHBoxLayout;
    hardnessWidget->setLayout(hardnessLayout);
    const auto hardnessLabel = new QLabel("hardness: ");
    mHardness = new QDoubleSlider(0, 1, 0.1, this);
    mHardness->setDisplayedValue(mDocument.fSculptBrush.hardness());
    connect(mHardness, &QDoubleSlider::valueChanged,
            this, [this](const qreal value) {
        mDocument.fSculptBrush.setHardness(value);
    });
    hardnessLayout->addWidget(hardnessLabel);
    hardnessLayout->addWidget(mHardness);
    mHardnessAct = mToolBar->addWidget(hardnessWidget);

    mSculptSpace2 = addSpaceToToolbar();

    const auto opacityWidget = new QWidget;
    opacityWidget->setObjectName("transparentWidget");
    const auto opacityLayout = new QHBoxLayout;
    opacityWidget->setLayout(opacityLayout);
    const auto opacityLabel = new QLabel("opacity: ");
    mOpacity = new QDoubleSlider(0, 1, 0.1, this);
    mOpacity->setDisplayedValue(mDocument.fSculptBrush.opacity());
    connect(mOpacity, &QDoubleSlider::valueChanged,
            this, [this](const qreal value) {
        mDocument.fSculptBrush.setOpacity(value);
    });
    opacityLayout->addWidget(opacityLabel);
    opacityLayout->addWidget(mOpacity);
    mOpacityAct = mToolBar->addWidget(opacityWidget);
}

void TimelineDockWidget::updateSculptPositionEnabled() {
    const SculptMode mode = mDocument.fSculptMode;
    const SculptTarget target = mDocument.fSculptTarget;
    mPositionTarget->setEnabled(target != SculptTarget::position ||
                                (mode == SculptMode::drag ||
                                 mode == SculptMode::replace ||
                                 mode == SculptMode::add));
}

void TimelineDockWidget::sculptTargetChanged() {
    const SculptTarget target = mDocument.fSculptTarget;
    mPositionTarget->setState(target == SculptTarget::position);
    mWidthTarget->setState(target == SculptTarget::width);
    mPressureTarget->setState(target == SculptTarget::pressure);
    mSpacingTarget->setState(target == SculptTarget::spacing);
    mTimeTarget->setState(target == SculptTarget::time);
    mColorTarget->setState(target == SculptTarget::color);

    updateSculptPositionEnabled();
}

void TimelineDockWidget::sculptModeChanged() {
    const SculptMode mode = mDocument.fSculptMode;
    mDragMode->setState(mode == SculptMode::drag);
    mAddMode->setState(mode == SculptMode::add);
    mReplaceMode->setState(mode == SculptMode::replace);
    mSubtractMode->setState(mode == SculptMode::subtract);

    updateSculptPositionEnabled();
}

void TimelineDockWidget::setResolutionFractionText(QString text) {
    text = text.remove(" %");
    const qreal res = clamp(text.toDouble(), 1, 200)/100;
    mMainWindow->setResolutionFractionValue(res);
}

void TimelineDockWidget::clearAll() {
    mRenderWidget->clearRenderQueue();
}

RenderWidget *TimelineDockWidget::getRenderWidget() {
    return mRenderWidget;
}

bool TimelineDockWidget::processKeyPress(QKeyEvent *event) {
    const CanvasMode mode = mDocument.fCanvasMode;
    const int key = event->key();
    const auto mods = event->modifiers();
    if(key == Qt::Key_Escape) {
        const auto state = RenderHandler::sInstance->currentPreviewState();
        if(state == PreviewSate::stopped) return false;
        interruptPreview();
    } else if(key == Qt::Key_Space) {
        const auto state = RenderHandler::sInstance->currentPreviewState();
        switch(state) {
            case PreviewSate::stopped: renderPreview(); break;
            case PreviewSate::rendering: playPreview(); break;
            case PreviewSate::playing: pausePreview(); break;
            case PreviewSate::paused: resumePreview(); break;
        }
    } else if(key == Qt::Key_Right && !(mods & Qt::ControlModifier)) {
        mDocument.incActiveSceneFrame();
    } else if(key == Qt::Key_Left && !(mods & Qt::ControlModifier)) {
        mDocument.decActiveSceneFrame();
    } else if(key == Qt::Key_Down && !(mods & Qt::ControlModifier)) {
        const auto scene = mDocument.fActiveScene;
        if(!scene) return false;
        int targetFrame;
        if(scene->anim_prevRelFrameWithKey(mDocument.getActiveSceneFrame(), targetFrame)) {
            mDocument.setActiveSceneFrame(targetFrame);
        }
    } else if(key == Qt::Key_Up && !(mods & Qt::ControlModifier)) {
        const auto scene = mDocument.fActiveScene;
        if(!scene) return false;
        int targetFrame;
        if(scene->anim_nextRelFrameWithKey(mDocument.getActiveSceneFrame(), targetFrame)) {
            mDocument.setActiveSceneFrame(targetFrame);
        }
    } else if(key == Qt::Key_P &&
              !(mods & Qt::ControlModifier) && !(mods & Qt::AltModifier)) {
        mLocalPivot->toggle();
    } else if(mode == CanvasMode::pointTransform && key == Qt::Key_N &&
              !(mods & Qt::ControlModifier) && !(mods & Qt::AltModifier)) {
        mNodeVisibility->toggle();
    } else {
        return false;
    }
    return true;
}

void TimelineDockWidget::previewFinished() {
    //setPlaying(false);
    mPlayFromBeginningButton->setDisabled(false);
    mStopButton->setDisabled(true);
    mPlayButton->setState(0);
    mPlayButton->setToolTip(gSingleLineTooltip("Render Preview", "Space"));
    disconnect(mPlayButton, nullptr, this, nullptr);
    connect(mPlayButton, &ActionButton::pressed,
            this, &TimelineDockWidget::renderPreview);
}

void TimelineDockWidget::previewBeingPlayed() {
    mPlayFromBeginningButton->setDisabled(true);
    mStopButton->setDisabled(false);
    mPlayButton->setState(1);
    mPlayButton->setToolTip(gSingleLineTooltip("Pause Preview", "Space"));
    disconnect(mPlayButton, nullptr, this, nullptr);
    connect(mPlayButton, &ActionButton::pressed,
            this, &TimelineDockWidget::pausePreview);
}

void TimelineDockWidget::previewBeingRendered() {
    mPlayFromBeginningButton->setDisabled(true);
    mStopButton->setDisabled(false);
    mPlayButton->setState(0);
    mPlayButton->setToolTip(gSingleLineTooltip("Play Preview", "Space"));
    disconnect(mPlayButton, nullptr, this, nullptr);
    connect(mPlayButton, &ActionButton::pressed,
            this, &TimelineDockWidget::playPreview);
}

void TimelineDockWidget::previewPaused() {
    mPlayFromBeginningButton->setDisabled(true);
    mStopButton->setDisabled(false);
    mPlayButton->setState(0);
    mPlayButton->setToolTip(gSingleLineTooltip("Resume Preview", "Space"));
    disconnect(mPlayButton, nullptr, this, nullptr);
    connect(mPlayButton, &ActionButton::pressed,
            this, &TimelineDockWidget::resumePreview);
}

void TimelineDockWidget::resumePreview() {
    RenderHandler::sInstance->resumePreview();
}

void TimelineDockWidget::updateButtonsVisibility(const CanvasMode mode) {
    mLocalPivotAct->setVisible(mode == CanvasMode::pointTransform ||
                               mode == CanvasMode::boxTransform);
    mNodeVisibilityAct->setVisible(mode == CanvasMode::pointTransform);

    const bool paintMode = mode == CanvasMode::paint;
    mBrushColorWidgetAct->setVisible(paintMode);
    mDecBrushSizeAct->setVisible(paintMode);
    mBrushSizeLabelAct->setVisible(paintMode);
    mIncBrushSizeAct->setVisible(paintMode);
    mOnionAct->setVisible(paintMode);

    const bool sculptMode = mode == CanvasMode::sculptPath;
    mSculptColorLabelAct->setVisible(sculptMode);

    mDecSculptBrushSizeAct->setVisible(sculptMode);
    mSculptBrushSizeLabelAct->setVisible(sculptMode);
    mIncSculptBrushSizeAct->setVisible(sculptMode);

    mDragModeAct->setVisible(sculptMode);
    mAddModeAct->setVisible(sculptMode);
    mReplaceModeAct->setVisible(sculptMode);
    mSubtractModeAct->setVisible(sculptMode);

    mSculptModeTargetSeperator->setVisible(sculptMode);

    mPositionTargetAct->setVisible(sculptMode);
    mWidthTargetAct->setVisible(sculptMode);
    mPressureTargetAct->setVisible(sculptMode);
    mSpacingTargetAct->setVisible(sculptMode);
    mTimeTargetAct->setVisible(sculptMode);
    mColorTargetAct->setVisible(sculptMode);

    mSculptSpace0->setVisible(sculptMode);

    mValueAct->setVisible(sculptMode);
    mSculptSpace1->setVisible(sculptMode);
    mHardnessAct->setVisible(sculptMode);
    mSculptSpace2->setVisible(sculptMode);
    mOpacityAct->setVisible(sculptMode);
}

void setSomeBrushColor(const int height,
                       TriggerLabel* const label,
                       const QColor &color) {
    const int dim = height - 2;
    QPixmap pix(dim, dim);
    if(color.alpha() == 255) {
        pix.fill(color);
    } else {
        QPainter p(&pix);
        p.drawTiledPixmap(0, 0, dim, dim, *ALPHA_MESH_PIX);
        p.fillRect(0, 0, dim, dim, color);
        p.end();
    }

    label->setPixmap(pix);
}

void TimelineDockWidget::setBrushColor(const QColor &color) {
    setSomeBrushColor(mToolBar->height(), mColorLabel, color);
}

void TimelineDockWidget::setSculptBrushColor(const QColor &color) {
    setSomeBrushColor(mToolBar->height(), mSculptColorLabel, color);
}

void TimelineDockWidget::setBrush(BrushContexedWrapper* const brush) {
    const int dim = mToolBar->height() - 2;
    if(!brush) {
        QPixmap pix(dim, dim);
        pix.fill(Qt::white);
        mBrushLabel->setPixmap(pix);
        return;
    }
    const auto& icon = brush->getBrushData().fIcon;
    const auto img = icon.scaled(dim, dim, Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation);
    mBrushLabel->setPixmap(QPixmap::fromImage(img));
}

void TimelineDockWidget::pausePreview() {
    RenderHandler::sInstance->pausePreview();
}

void TimelineDockWidget::playPreview() {
    RenderHandler::sInstance->playPreview();
}

void TimelineDockWidget::renderPreview() {
    RenderHandler::sInstance->renderPreview();
}

void TimelineDockWidget::interruptPreview() {
    RenderHandler::sInstance->interruptPreview();
}

void TimelineDockWidget::setLocalPivot(const bool local) {
    mDocument.fLocalPivot = local;
    for(const auto& scene : mDocument.fScenes)
        scene->updatePivot();
    Document::sInstance->actionFinished();
}

void TimelineDockWidget::setTimelineMode() {
    mTimelineAction->setDisabled(true);
    mRenderAction->setDisabled(false);

    mRenderAction->setChecked(false);
    mRenderWidget->hide();
    mTimelineLayout->show();
}

void TimelineDockWidget::setRenderMode() {
    mTimelineAction->setDisabled(false);
    mRenderAction->setDisabled(true);

    mTimelineAction->setChecked(false);
    mTimelineLayout->hide();
    mRenderWidget->show();
}

void TimelineDockWidget::updateSettingsForCurrentCanvas(Canvas* const canvas) {
    if(canvas) {
        disconnect(mResolutionComboBox, &QComboBox::currentTextChanged,
                   this, &TimelineDockWidget::setResolutionFractionText);
        mResolutionComboBox->setCurrentText(
                    QString::number(canvas->getResolutionFraction()*100) + " %");
        connect(mResolutionComboBox, &QComboBox::currentTextChanged,
                this, &TimelineDockWidget::setResolutionFractionText);
    }
}
