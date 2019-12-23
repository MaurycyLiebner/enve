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

    mPlayButton = SwitchButton::sCreate2Switch(iconsDir + "/play.png",
                                               iconsDir + "/pause.png",
                                               "Render Preview", this);
    mStopButton = new ActionButton(iconsDir + "/stop.png", "Stop Preview", this);
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

    mToolBar->addWidget(mPlayButton);
    mToolBar->addWidget(mStopButton);

    mToolBar->widgetForAction(mToolBar->addAction("     "))->
            setObjectName("emptyToolButton");

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
    mToolBar->widgetForAction(mToolBar->addAction(""))->
            setObjectName("emptyToolButton");
    mDecBrushSizeAct = mToolBar->addWidget(mDecBrushSize);
    mBrushSizeLabelAct = mToolBar->addWidget(mBrushSizeLabel);
    mIncBrushSizeAct = mToolBar->addWidget(mIncBrushSize);
    mBrushLabel->setStyleSheet("QWidget {"
                                   "background: white;"
                                   "border: 1px solid black;"
                               "}");
    mColorLabel->setStyleSheet("QWidget {"
                                   "background: black;"
                                   "border: 1px solid black;"
                                   "padding-left: -1px;"
                               "}");
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

    mPlayButton->setEnabled(false);
    mStopButton->setEnabled(false);

    mRenderWidget = new RenderWidget(this);

    connect(&mDocument, &Document::activeSceneSet,
            this, [this](Canvas* const scene) {
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
}

void TimelineDockWidget::setResolutionFractionText(QString text) {
    text = text.remove(" %");
    const qreal res = clamp(text.toDouble(), 1, 200)/100;
    mMainWindow->setResolutionFractionValue(res);
}

void TimelineDockWidget::clearAll() {

}

RenderWidget *TimelineDockWidget::getRenderWidget() {
    return mRenderWidget;
}

bool TimelineDockWidget::processKeyPress(QKeyEvent *event) {
    const int key = event->key();
    const auto mods = event->modifiers();
    if(key == Qt::Key_Right && !(mods & Qt::ControlModifier)) {
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
    } else if(key == Qt::Key_N &&
              !(mods & Qt::ControlModifier) && !(mods & Qt::AltModifier)) {
        mNodeVisibility->toggle();
    } else {
        return false;
    }
    return true;
}

void TimelineDockWidget::previewFinished() {
    //setPlaying(false);
    mStopButton->setDisabled(true);
    mPlayButton->setState(0);
    mPlayButton->setToolTip("Render Preview");
    disconnect(mPlayButton, nullptr, this, nullptr);
    connect(mPlayButton, &ActionButton::pressed,
            this, &TimelineDockWidget::renderPreview);
}

void TimelineDockWidget::previewBeingPlayed() {
    mStopButton->setDisabled(false);
    mPlayButton->setState(1);
    mPlayButton->setToolTip("Pause Preview");
    disconnect(mPlayButton, nullptr, this, nullptr);
    connect(mPlayButton, &ActionButton::pressed,
            this, &TimelineDockWidget::pausePreview);
}

void TimelineDockWidget::previewBeingRendered() {
    mStopButton->setDisabled(false);
    mPlayButton->setState(0);
    mPlayButton->setToolTip("Play Preview");
    disconnect(mPlayButton, nullptr, this, nullptr);
    connect(mPlayButton, &ActionButton::pressed,
            this, &TimelineDockWidget::playPreview);
}

void TimelineDockWidget::previewPaused() {
    mStopButton->setDisabled(false);
    mPlayButton->setState(0);
    mPlayButton->setToolTip("Resume Preview");
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

    mBrushColorWidgetAct->setVisible(mode == CanvasMode::paint);
    mDecBrushSizeAct->setVisible(mode == CanvasMode::paint);
    mBrushSizeLabelAct->setVisible(mode == CanvasMode::paint);
    mIncBrushSizeAct->setVisible(mode == CanvasMode::paint);

}

void TimelineDockWidget::setBrushColor(const QColor &color) {
    const int dim = mToolBar->height() - 2;
    QPixmap pix(dim, dim);
    if(color.alpha() == 255) {
        pix.fill(color);
    } else {
        QPainter p(&pix);
        p.drawTiledPixmap(0, 0, dim, dim, *ALPHA_MESH_PIX);
        p.fillRect(0, 0, dim, dim, color);
        p.end();
    }

    mColorLabel->setPixmap(pix);
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
