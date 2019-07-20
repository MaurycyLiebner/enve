#include "boxeslistanimationdockwidget.h"
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
#include "boxeslistkeysviewwidget.h"
#include "animationwidgetscrollbar.h"
#include "global.h"
#include "renderinstancesettings.h"
#include "document.h"
#include "layouthandler.h"
#include "memoryhandler.h"

BoxesListAnimationDockWidget::BoxesListAnimationDockWidget(
        Document& document, LayoutHandler * const layoutH,
        MainWindow *parent) :
    QWidget(parent), mDocument(document),
    mTimelineLayout(layoutH->timelineLayout()) {
    connect(&mDocument.fRenderHandler, &RenderHandler::previewFinished,
            this, &BoxesListAnimationDockWidget::previewFinished);
    connect(&mDocument.fRenderHandler, &RenderHandler::previewBeingPlayed,
            this, &BoxesListAnimationDockWidget::previewBeingPlayed);
    connect(&mDocument.fRenderHandler, &RenderHandler::previewBeingRendered,
            this, &BoxesListAnimationDockWidget::previewBeingRendered);
    connect(&mDocument.fRenderHandler, &RenderHandler::previewPaused,
            this, &BoxesListAnimationDockWidget::previewPaused);

    setFocusPolicy(Qt::NoFocus);

    mMainWindow = parent;
    setMinimumSize(10*MIN_WIDGET_DIM, 10*MIN_WIDGET_DIM);
    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);
    mMainLayout->setSpacing(0);
    mMainLayout->setMargin(0);

//    mAnimationDockWidget = new AnimationDockWidget(mBoxesListWidget,
//                                                   mKeysView);
//    mKeysView->setAnimationDockWidget(mAnimationDockWidget);

//    mGoToPreviousKeyButton = new QPushButton(
//                QIcon(":/icons/prev_key.png"), "", this);
//    mGoToPreviousKeyButton->setSizePolicy(QSizePolicy::Maximum,
//                                          QSizePolicy::Maximum);
//    mGoToNextKeyButton = new QPushButton(
//                QIcon(":/icons/next_key.png"), "", this);
//    mGoToNextKeyButton->setSizePolicy(QSizePolicy::Maximum,
//                                      QSizePolicy::Maximum);
    mResolutionComboBox = new QComboBox(this);
    mResolutionComboBox->addItem("100 %");
    mResolutionComboBox->addItem("75 %");
    mResolutionComboBox->addItem("50 %");
    mResolutionComboBox->addItem("25 %");
    mResolutionComboBox->setEditable(true);
    mResolutionComboBox->lineEdit()->setInputMask("D00 %");
    mResolutionComboBox->setCurrentText("100 %");
    mResolutionComboBox->setProperty("forceHandleEvent", QVariant(true));
    mResolutionComboBox->setInsertPolicy(QComboBox::NoInsert);
    mResolutionComboBox->setSizePolicy(QSizePolicy::Maximum,
                                       QSizePolicy::Maximum);
    connect(mResolutionComboBox, &QComboBox::currentTextChanged,
            this, &BoxesListAnimationDockWidget::setResolutionFractionText);

    mPlayButton = new ActionButton(
                ":/icons/renderPreviewButton.png",
                "render preview", this);
    mStopButton = new ActionButton(
                ":/icons/stopPreviewButton.png",
                "stop preview", this);
    connect(mStopButton, &ActionButton::pressed,
            this, &BoxesListAnimationDockWidget::interruptPreview);

    mLocalPivot = new ActionButton(
                ":/icons/globalPivot.png",
                "", this);
    mLocalPivot->setToolTip("P");
    mLocalPivot->setCheckable(":/icons/localPivot.png");
    mLocalPivot->setChecked(mDocument.fLocalPivot);
    connect(mLocalPivot, &ActionButton::toggled,
            this, &BoxesListAnimationDockWidget::setLocalPivot);

    mToolBar = new QToolBar(this);
    mToolBar->setMovable(false);

    mToolBar->setIconSize(QSize(5*MIN_WIDGET_DIM/4,
                                5*MIN_WIDGET_DIM/4));
    mToolBar->addSeparator();

//    mControlButtonsLayout->addWidget(mGoToPreviousKeyButton);
//    mGoToPreviousKeyButton->setFocusPolicy(Qt::NoFocus);
//    mControlButtonsLayout->addWidget(mGoToNextKeyButton);
//    mGoToNextKeyButton->setFocusPolicy(Qt::NoFocus);
    QAction *resA = mToolBar->addAction("Resolution:");
    mToolBar->widgetForAction(resA)->setObjectName("inactiveToolButton");

    mToolBar->addWidget(mResolutionComboBox);
    mToolBar->addSeparator();
    //mResolutionComboBox->setFocusPolicy(Qt::NoFocus);

    mToolBar->addWidget(mPlayButton);
    mToolBar->addWidget(mStopButton);

    mToolBar->addSeparator();
    mToolBar->addWidget(mLocalPivot);
    mLocalPivot->setFocusPolicy(Qt::NoFocus);
    mToolBar->addSeparator();

    QWidget * const spacerWidget = new QWidget(this);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding,
                                QSizePolicy::Minimum);
    spacerWidget->setStyleSheet("QWidget {"
                                    "background-color: rgba(0, 0, 0, 0)"
                                "}");
    mToolBar->addWidget(spacerWidget);

    mToolBar->addSeparator();

    mTimelineAction = mToolBar->addAction("Timeline", this,
                                          &BoxesListAnimationDockWidget::setTimelineMode);
    mTimelineAction->setObjectName("customToolButton");
    mTimelineAction->setCheckable(true);
    mTimelineAction->setChecked(true);
    mRenderAction = mToolBar->addAction("Render", this,
                                        &BoxesListAnimationDockWidget::setRenderMode);
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

    connect(mRenderWidget, &RenderWidget::renderFromSettings,
            this, [this](RenderInstanceSettings* const settings) {
        mDocument.fRenderHandler.renderFromSettings(settings);
    });

    mMainLayout->addWidget(mTimelineLayout);
    mMainLayout->addWidget(mRenderWidget);
    mRenderWidget->hide();

    previewFinished();
    //addNewBoxesListKeysViewWidget(1);
    //addNewBoxesListKeysViewWidget(0);

    connect(&mDocument, &Document::activeSceneSet,
            this, &BoxesListAnimationDockWidget::updateSettingsForCurrentCanvas);
}

void BoxesListAnimationDockWidget::setResolutionFractionText(QString text) {
    text = text.remove(" %");
    const qreal res = clamp(text.toDouble(), 1, 200)/100;
    mMainWindow->setResolutionFractionValue(res);
}

void BoxesListAnimationDockWidget::clearAll() {

}

RenderWidget *BoxesListAnimationDockWidget::getRenderWidget() {
    return mRenderWidget;
}

bool BoxesListAnimationDockWidget::processKeyEvent(QKeyEvent *event) {
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
    } else {
        return false;
    }
    return true;
}

void BoxesListAnimationDockWidget::previewFinished() {
    //setPlaying(false);
    mStopButton->setDisabled(true);
    mPlayButton->setIcon(":/icons/renderPreviewButton.png");
    mPlayButton->setToolTip("render preview");
    disconnect(mPlayButton, nullptr, this, nullptr);
    connect(mPlayButton, &ActionButton::pressed,
            this, &BoxesListAnimationDockWidget::renderPreview);
}

void BoxesListAnimationDockWidget::previewBeingPlayed() {
    mStopButton->setDisabled(false);
    mPlayButton->setIcon(":/icons/pausePreviewButton.png");
    mPlayButton->setToolTip("pause preview");
    disconnect(mPlayButton, nullptr, this, nullptr);
    connect(mPlayButton, &ActionButton::pressed,
            this, &BoxesListAnimationDockWidget::pausePreview);
}

void BoxesListAnimationDockWidget::previewBeingRendered() {
    mStopButton->setDisabled(false);
    mPlayButton->setIcon(":/icons/playPreviewButton.png");
    mPlayButton->setToolTip("play preview");
    disconnect(mPlayButton, nullptr, this, nullptr);
    connect(mPlayButton, &ActionButton::pressed,
            this, &BoxesListAnimationDockWidget::playPreview);
}

void BoxesListAnimationDockWidget::previewPaused() {
    mStopButton->setDisabled(false);
    mPlayButton->setIcon(":/icons/playPreviewButton.png");
    mPlayButton->setToolTip("resume preview");
    disconnect(mPlayButton, nullptr, this, nullptr);
    connect(mPlayButton, &ActionButton::pressed,
            this, &BoxesListAnimationDockWidget::resumePreview);
}

void BoxesListAnimationDockWidget::resumePreview() {
    mDocument.fRenderHandler.resumePreview();
}

void BoxesListAnimationDockWidget::pausePreview() {
    mDocument.fRenderHandler.pausePreview();
}

void BoxesListAnimationDockWidget::playPreview() {
    mDocument.fRenderHandler.playPreview();
}

void BoxesListAnimationDockWidget::renderPreview() {
    mDocument.fRenderHandler.renderPreview();
}

void BoxesListAnimationDockWidget::interruptPreview() {
    mDocument.fRenderHandler.interruptPreview();
}

void BoxesListAnimationDockWidget::setLocalPivot(const bool bT) {
    mDocument.fLocalPivot = bT;
    for(const auto& scene : mDocument.fScenes)
        scene->updatePivot();
    Document::sInstance->actionFinished();
}

void BoxesListAnimationDockWidget::setTimelineMode() {
    mTimelineAction->setDisabled(true);
    mRenderAction->setDisabled(false);

    mRenderAction->setChecked(false);
    mTimelineLayout->show();
    mRenderWidget->hide();
}

void BoxesListAnimationDockWidget::setRenderMode() {
    mTimelineAction->setDisabled(false);
    mRenderAction->setDisabled(true);

    mTimelineAction->setChecked(false);
    mRenderWidget->show();
    mTimelineLayout->hide();
}

void BoxesListAnimationDockWidget::updateSettingsForCurrentCanvas(
        Canvas* const canvas) {
    if(canvas) {
        disconnect(mResolutionComboBox, &QComboBox::currentTextChanged,
                   this, &BoxesListAnimationDockWidget::setResolutionFractionText);
        mResolutionComboBox->setCurrentText(
                    QString::number(canvas->getResolutionFraction()*100) + " %");
        connect(mResolutionComboBox, &QComboBox::currentTextChanged,
                this, &BoxesListAnimationDockWidget::setResolutionFractionText);
    }
}
