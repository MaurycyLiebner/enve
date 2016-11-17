#include "vectorshapesmenu.h"
#include "pathpoint.h"
#include "qrealanimatorvalueslider.h"
#include "vectorpath.h"
#include "mainwindow.h"

VectorShapesMenu::VectorShapesMenu(QWidget *parent) : QWidget(parent) {
    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);

    setMinimumSize(200, 200);
    mScrollArea = new QScrollArea(this);
    mScrollArea->setBackgroundRole(QPalette::Window);
    mScrollArea->setFrameShadow(QFrame::Plain);
    mScrollArea->setFrameShape(QFrame::NoFrame);
    mScrollArea->setWidgetResizable(true);

    mScrollWidget = new QWidget(this);
    mScrollWidget->setSizePolicy(QSizePolicy::MinimumExpanding,
                                 QSizePolicy::MinimumExpanding);
    mScrollLayout = new QVBoxLayout(mScrollWidget);
    mScrollWidget->setLayout(mScrollLayout);
    mScrollArea->setWidget(mScrollWidget);

    mButtonsLayout = new QHBoxLayout();
    mNewShapeButton = new QPushButton("+", this);
    mRemoveShapeButton = new QPushButton("-", this);
    mEditShapeButton = new QPushButton("edit", this);
    mCancelEditButton = new QPushButton("cancel", this);

    connect(mNewShapeButton, SIGNAL(pressed()),
            this, SLOT(createNewShape()));
    connect(mRemoveShapeButton, SIGNAL(pressed()),
            this, SLOT(removeCurrentShape()));
    connect(mEditShapeButton, SIGNAL(pressed()),
            this, SLOT(editCurrrentShape()));
    connect(mCancelEditButton, SIGNAL(pressed()),
            this, SLOT(cancelEditCurrentShape()));

    mButtonsLayout->addWidget(mNewShapeButton);
    mButtonsLayout->addWidget(mRemoveShapeButton);
    mButtonsLayout->addWidget(mEditShapeButton);
    mButtonsLayout->addWidget(mCancelEditButton);
    mCancelEditButton->hide();

    mMainLayout->addWidget(mScrollArea);
    mMainLayout->addLayout(mButtonsLayout);
}

void VectorShapesMenu::createNewShape() {
    if(mCurrentVectorPath == NULL) return;
    addShapeWidgetForShape(mCurrentVectorPath->createNewShape());
}

void VectorShapesMenu::removeCurrentShape() {
    if(mSelectedShapeWidget == NULL) return;
    mCurrentVectorPath->removeShape(mSelectedShapeWidget->getShape());
    delete mSelectedShapeWidget;
    mSelectedShapeWidget = NULL;
}

void VectorShapesMenu::setAllButEditButtonEnabled(bool enabled) {
    foreach(ShapeWidget *shapeWidget, mShapeWidgets) {
        shapeWidget->setEnabled(enabled);
    }
    mNewShapeButton->setEnabled(enabled);
    mRemoveShapeButton->setEnabled(enabled);
}

void VectorShapesMenu::editCurrrentShape() {
    if(mSelectedShapeWidget == NULL) return;
    mEditingShape = true;
    setAllButEditButtonEnabled(false);
    mCurrentVectorPath->editShape(mSelectedShapeWidget->getShape());
    mEditShapeButton->setText("finish");
    disconnect(mEditShapeButton, SIGNAL(pressed()),
            this, SLOT(editCurrrentShape()));
    connect(mEditShapeButton, SIGNAL(pressed()),
            this, SLOT(finishEditingCurrrentShape()));
    mCancelEditButton->show();
    MainWindow::getInstance()->callUpdateSchedulers();
}

void VectorShapesMenu::finishEditingCurrrentShape() {
    if(mSelectedShapeWidget == NULL) return;
    mEditingShape = false;
    setAllButEditButtonEnabled(true);
    mCurrentVectorPath->finishEditingShape(mSelectedShapeWidget->getShape());
    mEditShapeButton->setText("edit");
    disconnect(mEditShapeButton, SIGNAL(pressed()),
            this, SLOT(finishEditingCurrrentShape()));
    connect(mEditShapeButton, SIGNAL(pressed()),
            this, SLOT(editCurrrentShape()));
    mCancelEditButton->hide();
    MainWindow::getInstance()->callUpdateSchedulers();
}

void VectorShapesMenu::cancelEditCurrentShape()
{
    if(mSelectedShapeWidget == NULL) return;
    mEditingShape = false;
    setAllButEditButtonEnabled(true);
    mCurrentVectorPath->cancelEditingShape();
    mEditShapeButton->setText("edit");
    disconnect(mEditShapeButton, SIGNAL(pressed()),
            this, SLOT(finishEditingCurrrentShape()));
    connect(mEditShapeButton, SIGNAL(pressed()),
            this, SLOT(editCurrrentShape()));
    mCancelEditButton->hide();
    MainWindow::getInstance()->callUpdateSchedulers();
}

void VectorShapesMenu::setSelectedBoundingBox(BoundingBox *box) {
    if(mCurrentVectorPath != NULL) {
        clearShapes();
    }
    if(box == NULL) {
        mCurrentVectorPath = NULL;
        mSelectedShapeWidget = NULL;
    } else if(box->isVectorPath() ) {
        mCurrentVectorPath = ((VectorPath*)box);
        mCurrentVectorPath->addShapesToShapesMenu(this);
    }
}

void VectorShapesMenu::clearShapes() {
    foreach(ShapeWidget *widget, mShapeWidgets) {
        delete widget;
    }
    mShapeWidgets.clear();
}

void VectorShapesMenu::addShapeWidgetForShape(VectorPathShape *shape) {
    ShapeWidget *shapeWidget = new ShapeWidget(shape, this);
    mScrollLayout->addWidget(shapeWidget);
    mShapeWidgets << shapeWidget;
}

void VectorShapesMenu::setSelected(ShapeWidget *widget) {
    if(mSelectedShapeWidget != NULL) mSelectedShapeWidget->setSelected(false);
    mSelectedShapeWidget = widget;
    if(mSelectedShapeWidget != NULL) mSelectedShapeWidget->setSelected(true);
}

ShapeWidget::ShapeWidget(VectorPathShape *shape, VectorShapesMenu *parent) :
    QWidget(parent)
{
    mParentShape = shape;
    mParentMenu = parent;
    //setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    //setMinimumHeight(25);

    mLayout = new QHBoxLayout(this);
    QLabel *label = new QLabel(shape->getName(), this);
    mLayout->addWidget(label);
    mLayout->addWidget(new QrealAnimatorValueSlider(0., 1.,
                                                shape->getInfluenceAnimator(),
                                                this));
    mLayout->setSpacing(0);
    mLayout->setMargin(0);
    setLayout(mLayout);
    setSelected(false);

    setFixedHeight(25);
}

void ShapeWidget::setSelected(bool bT)
{
    if(bT) {
        setStyleSheet("background-color: rgb(125, 125, 255)");
    } else {
        setStyleSheet("background-color: rgb(255, 255, 255)");
    }
    mSelected = bT;
}

void ShapeWidget::mousePressEvent(QMouseEvent *event)
{
    if(mSelected) return;
    mParentMenu->setSelected(this);
}

VectorPathShape *ShapeWidget::getShape()
{
    return mParentShape;
}
