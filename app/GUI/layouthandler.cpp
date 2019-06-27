#include "layouthandler.h"

#include <QPushButton>

LayoutHandler::LayoutHandler(Document& document) {
    const auto canvasComboLayout = new QHBoxLayout;
    canvasComboLayout->setSpacing(0);
    canvasComboLayout->setMargin(0);

    mComboWidget = new QWidget;
    mComboWidget->setLayout(canvasComboLayout);

    mComboBox = new QComboBox;
    mComboBox->setMinimumContentsLength(20);
    mComboBox->setObjectName("currentLayoutComboBox");
    mComboBox->setLayoutDirection(Qt::RightToLeft);
    const int comboDim = mComboBox->sizeHint().height();

    const auto newLayPush = new QPushButton("+", mComboWidget);
    newLayPush->setObjectName("addCanvasButton");
    newLayPush->setFixedSize(comboDim, comboDim);

    const auto removeLayPush = new QPushButton("x", mComboWidget);
    removeLayPush->setObjectName("removeCanvasButton");
    removeLayPush->setFixedSize(comboDim, comboDim);

    canvasComboLayout->addWidget(mComboBox);
    canvasComboLayout->addWidget(newLayPush);
    canvasComboLayout->addWidget(removeLayPush);

    mSceneLayout = new SceneLayout(document);
    mTimelineLayout = new TimelineLayout(document);

    connect(mComboBox, qOverload<int>(&QComboBox::activated),
            this, &LayoutHandler::setCurrent);
    connect(mComboBox, &QComboBox::editTextChanged,
            this, &LayoutHandler::renameCurrent);

    connect(newLayPush, &QPushButton::pressed,
            this, &LayoutHandler::newLayout);
    connect(removeLayPush, &QPushButton::pressed,
            this, &LayoutHandler::removeCurrent);

    connect(&document, &Document::sceneCreated,
            this, &LayoutHandler::newForScene);
    connect(&document, qOverload<Canvas*>(&Document::sceneRemoved),
            this, &LayoutHandler::removeForScene);
}

template <typename T>
void saveAllChildrenLayoutsData(QWidget* const parent) {
    const auto cww = dynamic_cast<T*>(parent);
    if(cww) {
        cww->saveDataToLayout();
        return;
    }
    const auto children = parent->children();
    for(const auto& child : children) {
        const auto childWidget = qobject_cast<QWidget*>(child);
        if(!childWidget) continue;
        const auto cww = dynamic_cast<T*>(childWidget);
        if(cww) cww->saveDataToLayout();
        else saveAllChildrenLayoutsData<T>(childWidget);
    }
}

void LayoutHandler::saveCurrent() {
    if(mCurrentId == -1) return;
    auto& current = mLayouts[uint(mCurrentId)];
    saveAllChildrenLayoutsData<CanvasWindowWrapper>(mSceneLayout);
    saveAllChildrenLayoutsData<TimelineWrapper>(mTimelineLayout);
    current.fCanvas = mSceneLayout->extract();
    current.fTimeline = mTimelineLayout->extract();
}
