#include "vectorshapesmenu.h"
#include "pathpoint.h"
#include "qrealanimatorvalueslider.h"
#include "Boxes/vectorpath.h"
#include "mainwindow.h"

VectorShapesMenu::VectorShapesMenu(QWidget *parent) : QWidget(parent) {
    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);

    setFocusPolicy(Qt::NoFocus);
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
    mScrollLayout->setAlignment(Qt::AlignTop);
    mScrollLayout->setSpacing(0);
    mScrollLayout->setMargin(0);
    QLabel *label = new QLabel(this);
    label->setStyleSheet("background-color: black");
    label->setFixedHeight(1);
    mScrollLayout->addWidget(label);

    mScrollArea->setWidget(mScrollWidget);

    mButtonsLayout = new QHBoxLayout();
    mNewShapeButton = new QPushButton("+", this);
    mRemoveShapeButton = new QPushButton("-", this);
    mEditShapeButton = new QPushButton("edit", this);
    mCancelEditButton = new QPushButton("cancel", this);

    mNewShapeMenu = new QMenu(this);
    mNewShapeMenu->addAction("Relative", this, SLOT(createNewRelativeShape()) );
    mNewShapeMenu->addAction("Absolute", this, SLOT(createNewAbsoluteShape()) );
    mNewShapeButton->setMenu(mNewShapeMenu);

//    connect(mNewShapeButton, SIGNAL(pressed()),
//            this, SLOT(createNewShape()));
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

void VectorShapesMenu::createNewRelativeShape() {
    createNewShape(true);
}

void VectorShapesMenu::createNewAbsoluteShape() {
    createNewShape(false);
}

void VectorShapesMenu::createNewShape(bool relative) {
    if(mCurrentVectorPath == NULL) return;
    addShapeWidgetForShape(mCurrentVectorPath->createNewShape(relative));
}

void VectorShapesMenu::removeCurrentShape() {
    if(mSelectedShapeWidget == NULL) return;
    mCurrentVectorPath->removeShape(mSelectedShapeWidget->getShape());
    mShapeWidgets.removeOne(mSelectedShapeWidget);
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
    if(mEditingShape) cancelEditCurrentShape();
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

void VectorShapesMenu::updateDisplayedShapes() {
    setSelectedBoundingBox(mCurrentVectorPath);
}

void VectorShapesMenu::clearShapes() {
    foreach(ShapeWidget *widget, mShapeWidgets) {
        delete widget;
    }
    mShapeWidgets.clear();
    mSelectedShapeWidget = NULL;
}

void VectorShapesMenu::addShapeWidgetForShape(VectorPathShape *shape) {
    ShapeWidget *shapeWidget = new ShapeWidget(shape, this);
    mScrollLayout->insertWidget(mScrollLayout->count() - 1, shapeWidget);
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
    label->setFixedHeight(20);
    mLayout->addWidget(label);
    mLayout->addWidget(new QrealAnimatorValueSlider(0., 1., 0.1,
                                                shape->getInfluenceAnimator(),
                                                this) );
    mLayout->setSpacing(0);
    mLayout->setMargin(0);

    setLayout(mLayout);
    setSelected(false);

    setFixedHeight(19);
}

void ShapeWidget::paintEvent(QPaintEvent *)
 {
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ShapeWidget::setSelected(bool bT)
{
    if(bT) {
        setStyleSheet("background-color: rgb(125, 125, 255); "
                      "border: 1px solid black; "
                      "border-right-width: 0px");
    } else {
        setStyleSheet("background-color: rgb(255, 255, 255); "
                      "border: 1px solid black; "
                      "border-right-width: 0px");
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
