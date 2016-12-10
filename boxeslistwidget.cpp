#include "boxeslistwidget.h"

QPixmap *BoxesListWidget::VISIBLE_PIXMAP;
QPixmap *BoxesListWidget::INVISIBLE_PIXMAP;
QPixmap *BoxesListWidget::HIDE_CHILDREN;
QPixmap *BoxesListWidget::SHOW_CHILDREN;
QPixmap *BoxesListWidget::LOCKED_PIXMAP;
QPixmap *BoxesListWidget::UNLOCKED_PIXMAP;
QPixmap *BoxesListWidget::ANIMATOR_CHILDREN_VISIBLE;
QPixmap *BoxesListWidget::ANIMATOR_CHILDREN_HIDDEN;
QPixmap *BoxesListWidget::ANIMATOR_RECORDING;
QPixmap *BoxesListWidget::ANIMATOR_NOT_RECORDING;
bool BoxesListWidget::mStaticPixmapsLoaded = false;

int BoxesListWidget::LIST_ITEM_HEIGHT = 20;
int BoxesListWidget::LIST_ITEM_MAX_WIDTH = 250;
int BoxesListWidget::LIST_ITEM_CHILD_INDENT = 20;

WidgetContainer::WidgetContainer(QWidget *parent) : QWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);
    mAllLayout = new QVBoxLayout(this);
    mAllLayout->setAlignment(Qt::AlignTop);
    mAllLayout->setSpacing(0);
    mAllLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mAllLayout);
    //mAllLayout->addWidget(mTargetBoxWidget);
    mDetailsWidget = new QWidget(this);
    mChildWidgetsLayout = new QVBoxLayout(mDetailsWidget);
    mChildWidgetsLayout->setAlignment(Qt::AlignTop);
    mChildWidgetsLayout->setSpacing(0);
    mChildWidgetsLayout->setContentsMargins(BoxesListWidget::getListItemChildIndent(), 0, 0, 0);
    mDetailsWidget->setLayout(mChildWidgetsLayout);
    mAllLayout->addWidget(mDetailsWidget);

    mDetailsWidget->hide();
}

void WidgetContainer::setDetailsVisible(bool visible)
{
    mDetailsWidget->setVisible(visible);
}

void WidgetContainer::setTopWidget(QWidget *widget) {
    mAllLayout->insertWidget(0, widget);
}

void WidgetContainer::addChildWidget(QWidget *widget) {
    mChildWidgetsLayout->addWidget(widget);
}

ComplexAnimatorItemWidgetContainer::ComplexAnimatorItemWidgetContainer(QrealAnimator *target,
                                                                       QWidget *parent) :
    WidgetContainer(parent)
{
    if(target->isComplexAnimator()) {
        mTargetAnimatorWidget = new ComplexAnimatorItemWidget(target, this);

        connect((ComplexAnimatorItemWidget*)mTargetAnimatorWidget, SIGNAL(detailsVisibilityChanged(bool)),
                mDetailsWidget, SLOT(setVisible(bool)));
        ((ComplexAnimator*)target)->addAllAnimatorsToComplexAnimatorItemWidgetContainer(this);
    } else {
        mTargetAnimatorWidget = new QrealAnimatorItemWidget(target, this);
    }

    setTopWidget(mTargetAnimatorWidget);
}

void ComplexAnimatorItemWidgetContainer::addChildAnimator(QrealAnimator *animator)
{
    ComplexAnimatorItemWidgetContainer *itemWidget = new ComplexAnimatorItemWidgetContainer(animator, this);

    addChildWidget(itemWidget);

    mChildWidgets << itemWidget;
}

void ComplexAnimatorItemWidgetContainer::removeChildAnimator(QrealAnimator *animator) {

}

BoxItemWidgetContainer::BoxItemWidgetContainer(BoundingBox *target,
                                               QWidget *parent) :
    WidgetContainer(parent)
{
    mTargetBoxWidget = new BoxItemWidget(target, this);
    setTopWidget(mTargetBoxWidget);
    connect(mTargetBoxWidget, SIGNAL(detailsVisibilityChanged(bool)),
            mDetailsWidget, SLOT(setVisible(bool)));

    target->addAllAnimatorsToBoxItemWidgetContainer(this);
}

BoxItemWidget::BoxItemWidget(BoundingBox *target, QWidget *parent) : QWidget(parent)
{
    setFixedHeight(BoxesListWidget::getListItemHeight());
    mTargetBox = target;
}

void BoxItemWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setPen(Qt::NoPen);
    if(mTargetBox->isSelected()) {
        p.setBrush(QColor(185, 185, 255));
    } else {
        p.setBrush(QColor(225, 225, 225));
    }
    p.drawRect(rect());

    int drawX = 0;
    if(mDetailsVisible) {
        p.drawPixmap(drawX, 0, *BoxesListWidget::HIDE_CHILDREN);
    } else {
        p.drawPixmap(drawX, 0, *BoxesListWidget::SHOW_CHILDREN);
    }
    drawX += BoxesListWidget::getListItemHeight();
    if(mTargetBox->isVisible()) {
        p.drawPixmap(drawX, 0, *BoxesListWidget::VISIBLE_PIXMAP);
    } else {
        p.drawPixmap(drawX, 0, *BoxesListWidget::INVISIBLE_PIXMAP);
    }
    drawX += BoxesListWidget::getListItemHeight();
    if(mTargetBox->isLocked()) {
        p.drawPixmap(drawX, 0, *BoxesListWidget::LOCKED_PIXMAP);
    } else {
        p.drawPixmap(drawX, 0, *BoxesListWidget::UNLOCKED_PIXMAP);
    }
    drawX += 2*BoxesListWidget::getListItemHeight();
    p.drawText(QRect(drawX, 0,
                     width() - drawX -
                     BoxesListWidget::getListItemHeight(),
                     BoxesListWidget::getListItemHeight()),
               mTargetBox->getName(), QTextOption(Qt::AlignVCenter));

    p.end();
}

void BoxItemWidget::setDetailsVisibile(bool bT)
{
    mDetailsVisible = bT;
    emit detailsVisibilityChanged(bT);
}

#include <QInputDialog>
#include "mainwindow.h"
void BoxItemWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->x() < 20) {
        setDetailsVisibile(!mDetailsVisible);
    } else if(event->x() < 40) {
        mTargetBox->setVisibile(!mTargetBox->isVisible());
    } else if(event->x() < 60) {
        mTargetBox->setLocked(!mTargetBox->isLocked());
    } else {
        if(event->button() == Qt::RightButton) {
            if(event->x() >= 60) {
                    QMenu menu(this);


                    menu.addAction("Rename");
                    QAction *selected_action = menu.exec(event->globalPos());
                    if(selected_action != NULL)
                    {
                        if(selected_action->text() == "Rename") {
                            rename();
                        }
                    } else {

                    }
            }
        } else {
            mTargetBox->selectionChangeTriggered(event->modifiers() & Qt::ShiftModifier);
        }
    }

    MainWindow::getInstance()->callUpdateSchedulers();
}

void BoxItemWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    if(e->x() >= 60 ) {
        if(e->modifiers() & Qt::ShiftModifier) {
            mousePressEvent(e);
        } else {
            rename();
        }
    }
}

void BoxItemWidget::rename() {
    bool ok;
    QString text = QInputDialog::getText(this, tr("New name dialog"),
                                         tr("Name:"), QLineEdit::Normal,
                                         mTargetBox->getName(), &ok);
    if(ok) {
        mTargetBox->setName(text);
        update();
    }
}

void BoxItemWidgetContainer::addAnimatorWidgetForAnimator(QrealAnimator *animator)
{
    addAnimatorWidget(new ComplexAnimatorItemWidgetContainer(animator, this));
}

void BoxItemWidgetContainer::removeAnimatorWidgetForAnimator(QrealAnimator *animator)
{
    foreach(ComplexAnimatorItemWidgetContainer *animatorWidget, mAnimatorsContainers) {
        if(animatorWidget->getTargetAnimator() == animator) {
            removeAnimatorWidget(animatorWidget);
            break;
        }
    }
}

void BoxItemWidgetContainer::addAnimatorWidget(ComplexAnimatorItemWidgetContainer *widget)
{
    mChildWidgetsLayout->addWidget(widget);
}

void BoxItemWidgetContainer::removeAnimatorWidget(ComplexAnimatorItemWidgetContainer *widget)
{
    mAnimatorsContainers.removeOne(widget);
    delete widget;
}

#include "qrealanimatorvalueslider.h"
QrealAnimatorItemWidget::QrealAnimatorItemWidget(QrealAnimator *target,
                                                 QWidget *parent) :
    QWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);
    setFixedHeight(BoxesListWidget::getListItemHeight());
    mTargetAnimator = target;
    if(!target->isComplexAnimator()) {
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 20, 0);
        layout->setAlignment(Qt::AlignRight);
        setLayout(layout);
        QrealAnimatorValueSlider *valueSlider = new QrealAnimatorValueSlider(target);
        valueSlider->setWheelInteractionEnabled(false);
        layout->addWidget(valueSlider);
    }
}

void QrealAnimatorItemWidget::draw(QPainter *p) {
    if(mTargetAnimator->isCurrentAnimator()) {
        p->fillRect(-5, 0,
                   5, height(),
                   mTargetAnimator->getAnimatorColor());
        p->fillRect(0, 0,
                   BoxesListWidget::getListItemMaxWidth(), BoxesListWidget::getListItemHeight(),
                   QColor(255, 255, 255, 125));
    }
    int drawX = 0;
    if(mTargetAnimator->isRecording()) {
        p->drawPixmap(drawX, 0, *BoxesListWidget::ANIMATOR_RECORDING);
    } else {
        p->drawPixmap(drawX, 0, *BoxesListWidget::ANIMATOR_NOT_RECORDING);
    }
    p->setPen(Qt::black);
    drawX += 2*BoxesListWidget::getListItemChildIndent();
    p->drawText(drawX, 0,
               width() - 80.,
               BoxesListWidget::getListItemHeight(),
               Qt::AlignVCenter | Qt::AlignLeft,
               mTargetAnimator->getName() );
//    p->setPen(Qt::blue);
//    p->drawText(width() - 80., 0,
//               70., BoxesListWidget::getListItemHeight(),
//               Qt::AlignVCenter | Qt::AlignLeft,
//               " " + mTargetAnimator->getValueText() );
}

void QrealAnimatorItemWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    draw(&p);
    p.end();
}

ComplexAnimatorItemWidget::ComplexAnimatorItemWidget(QrealAnimator *target,
                                                     QWidget *parent) :
    QrealAnimatorItemWidget(target, parent) {

}

void ComplexAnimatorItemWidget::draw(QPainter *p) {
    int drawX = 0;
    QrealAnimatorItemWidget::draw(p);
    if(mTargetAnimator->isDescendantRecording() && !mTargetAnimator->isRecording()) {
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        p->setBrush(Qt::red);
        p->setPen(Qt::NoPen);
        p->drawEllipse(QPointF(BoxesListWidget::getListItemChildIndent()*0.5,
                              BoxesListWidget::getListItemHeight()*0.5),
                       2.5, 2.5);
        p->restore();
    }
    drawX += BoxesListWidget::getListItemChildIndent();
    if(mDetailsVisible) {
        p->drawPixmap(drawX, 0, *BoxesListWidget::ANIMATOR_CHILDREN_VISIBLE);
    } else {
        p->drawPixmap(drawX, 0, *BoxesListWidget::ANIMATOR_CHILDREN_HIDDEN);
    }
}

void QrealAnimatorItemWidget::mousePressEvent(QMouseEvent *event) {
    if(event->x() < 20) {
        mTargetAnimator->setRecording(!mTargetAnimator->isRecording());
    }

    MainWindow::getInstance()->callUpdateSchedulers();
}

void ComplexAnimatorItemWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->x() < 20) {
        mTargetAnimator->setRecording(!mTargetAnimator->isRecording());
    } else {
        setDetailsVisibile(!mDetailsVisible);
    }

    MainWindow::getInstance()->callUpdateSchedulers();
}

void ComplexAnimatorItemWidget::setDetailsVisibile(bool bT) {
    mDetailsVisible = bT;
    emit detailsVisibilityChanged(bT);
}

BoxesListWidget::BoxesListWidget(QWidget *parent) : QWidget(parent)
{
    loadStaticPixmaps();
    mBoxesLayout = new QVBoxLayout(this);
    setLayout(mBoxesLayout);
    mBoxesLayout->setAlignment(Qt::AlignTop);
    mBoxesLayout->setSpacing(0);
    mBoxesLayout->setMargin(0);
    mBoxesLayout->setContentsMargins(0, 0, 0, 20);
    setContentsMargins(0, 0, 0, 0);

    connect(MainWindow::getInstance()->getCanvas(), SIGNAL(changeChildZSignal(int,int)),
            this, SLOT(changeItemZ(int,int)));
//    QLabel *label = new QLabel(this);
//    label->setStyleSheet("background-color: black");
//    label->setFixedHeight(1);
//    mBoxesLayout->addWidget(label);
}

void BoxesListWidget::loadStaticPixmaps()
{
    if(mStaticPixmapsLoaded) return;
    VISIBLE_PIXMAP = new QPixmap("pixmaps/icons/ink_visible.png");
    INVISIBLE_PIXMAP = new QPixmap("pixmaps/icons/ink_hidden.png");
    HIDE_CHILDREN = new QPixmap("pixmaps/icons/list_hide_children.png");
    SHOW_CHILDREN = new QPixmap("pixmaps/icons/list_show_children.png");
    LOCKED_PIXMAP = new QPixmap("pixmaps/icons/ink_lock_locked.png");
    UNLOCKED_PIXMAP = new QPixmap("pixmaps/icons/ink_lock_unlocked.png");
    ANIMATOR_CHILDREN_VISIBLE = new QPixmap(
                "pixmaps/icons/animator_children_visible.png");
    ANIMATOR_CHILDREN_HIDDEN = new QPixmap(
                "pixmaps/icons/animator_children_hidden.png");
    ANIMATOR_RECORDING = new QPixmap(
                "pixmaps/icons/recording.png");
    ANIMATOR_NOT_RECORDING = new QPixmap(
                "pixmaps/icons/not_recording.png");
    mStaticPixmapsLoaded = true;
}

void BoxesListWidget::addItemForBox(BoundingBox *box)
{
    BoxItemWidgetContainer *itemWidgetContainer = new BoxItemWidgetContainer(box, this);
    mBoxesLayout->addWidget(itemWidgetContainer);
    mBoxWidgetContainers << itemWidgetContainer;
}

void BoxesListWidget::removeItemFromBox(BoundingBox *box) {
    foreach(BoxItemWidgetContainer *widget, mBoxWidgetContainers) {
        if(widget->getTargetBox() == box) {
            mBoxWidgetContainers.removeOne(widget);
            delete widget;
        }
    }
}

void BoxesListWidget::changeItemZ(int from, int to) {
    mBoxesLayout->insertItem(to, mBoxesLayout->takeAt(from));
}
