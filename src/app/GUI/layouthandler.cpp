#include "layouthandler.h"

#include <QPushButton>

LayoutHandler::LayoutHandler(Document& document,
                             AudioHandler& audioHandler) :
    mDocument(document), mAudioHandler(audioHandler) {
    const auto canvasComboLayout = new QHBoxLayout;
    canvasComboLayout->setSpacing(0);
    canvasComboLayout->setMargin(0);

    mComboWidget = new QWidget;
    mComboWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    mComboWidget->setLayout(canvasComboLayout);
    mComboWidget->setObjectName("invisWid");
    mComboWidget->setStyleSheet("QWidget#invisWid { background: transparent; }");

    mComboBox = new QComboBox;
    mComboBox->setMinimumContentsLength(20);
    mComboBox->setObjectName("currentLayoutComboBox");
    mComboBox->setLayoutDirection(Qt::RightToLeft);
    mComboBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    const auto newLayPush = new QPushButton("+", mComboWidget);
    newLayPush->setObjectName("addCanvasButton");
    newLayPush->setFixedWidth(MIN_WIDGET_DIM);
    newLayPush->setMinimumHeight(0);
    newLayPush->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    const auto removeLayPush = new QPushButton("x", mComboWidget);
    removeLayPush->setObjectName("removeCanvasButton");
    removeLayPush->setFixedWidth(MIN_WIDGET_DIM);
    removeLayPush->setMinimumHeight(0);
    removeLayPush->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    canvasComboLayout->addWidget(mComboBox);
    canvasComboLayout->addWidget(newLayPush);
    canvasComboLayout->addWidget(removeLayPush);

    mSceneLayout = new SceneLayout(mDocument);
    mTimelineLayout = new SceneLayout(mDocument);

    connect(mComboBox, qOverload<int>(&QComboBox::activated),
            this, &LayoutHandler::setCurrent);
    connect(mComboBox, &QComboBox::editTextChanged,
            this, &LayoutHandler::renameCurrent);

    connect(newLayPush, &QPushButton::pressed,
            this, &LayoutHandler::newLayoutAction);
    connect(removeLayPush, &QPushButton::pressed,
            this, &LayoutHandler::removeCurrent);

    connect(&document, &Document::sceneCreated,
            this, &LayoutHandler::newForSceneAction);
    connect(&document, qOverload<Canvas*>(&Document::sceneRemoved),
            this, &LayoutHandler::removeForScene);
}

void LayoutHandler::clear() {
    mSceneLayout->setCurrent(nullptr);
    mTimelineLayout->setCurrent(nullptr);
}

void LayoutHandler::saveCurrent() {
    if(mCurrentId == -1) return;
    mSceneLayout->saveData();
    mTimelineLayout->saveData();
}
