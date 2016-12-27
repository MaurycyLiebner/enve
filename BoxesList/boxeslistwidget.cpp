#include "boxeslistwidget.h"
#include "mainwindow.h"
#include "boxitemwidgetcontainer.h"
#include "boxesgroupwidgetcontainer.h"
#include "boxesgroup.h"

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

BoxesListWidget::BoxesListWidget(QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::NoFocus);
    loadStaticPixmaps();
    mBoxesLayout = new QVBoxLayout(this);
    setLayout(mBoxesLayout);
    mBoxesLayout->setAlignment(Qt::AlignTop);
    mBoxesLayout->setSpacing(0);
    mBoxesLayout->setMargin(0);
    mBoxesLayout->setContentsMargins(0, 0, 0, 20);
    setContentsMargins(0, 0, 0, 0);

    Canvas *canvas = MainWindow::getInstance()->getCanvas();
    connect(canvas, SIGNAL(changeChildZSignal(int,int)),
            this, SLOT(changeItemZ(int,int)));
    connect(canvas, SIGNAL(addAnimatedBoundingBoxSignal(BoundingBox*)),
            this, SLOT(addItemForBox(BoundingBox*)));
    connect(canvas, SIGNAL(removeAnimatedBoundingBoxSignal(BoundingBox*)),
            this, SLOT(removeItemForBox(BoundingBox*)));
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

void BoxesListWidget::drawKeys(QPainter *p, qreal pixelsPerFrame, int viewedTop,
                               int minViewedFrame, int maxViewedFrame) {
    foreach(BoxItemWidgetContainer *container, mBoxWidgetContainers) {
        int containerTop = container->y() - viewedTop;
        int containerBottom = containerTop + container->height();
        if(containerTop > height() || containerBottom < 0) continue;
        container->drawKeys(p, pixelsPerFrame,
                            containerTop, height(),
                            minViewedFrame, maxViewedFrame);
    }
}

QrealKey *BoxesListWidget::getKeyAtPos(int pressX, int pressY, qreal pixelsPerFrame,
                                       int viewedTop, int minViewedFrame) {
    int remaining = pressY % getListItemHeight();
    if(remaining < (getListItemHeight() - KEY_RECT_SIZE)/2 ||
       remaining > (getListItemHeight() + KEY_RECT_SIZE)/2) return NULL;
    foreach(BoxItemWidgetContainer *container, mBoxWidgetContainers) {
        int containerTop = container->y() - viewedTop;
        int containerBottom = containerTop + container->height();
        if(containerTop > pressY || containerBottom < pressY) continue;
        return container->getKeyAtPos(pressX, pressY, pixelsPerFrame,
                               height(), containerTop, minViewedFrame);
    }
    return NULL;
}

void BoxesListWidget::getKeysInRect(QRectF selectionRect, int viewedTop,
                                    qreal pixelsPerFrame, int minViewedFrame,
                                    QList<QrealKey*> *listKeys) {
    selectionRect.adjust(0, -getListItemHeight()*0.75,
                         0, getListItemHeight()*0.75);
    foreach(BoxItemWidgetContainer *container, mBoxWidgetContainers) {
        int containerTop = container->y() - viewedTop;
        int containerBottom = containerTop + container->height();
        if(containerTop > selectionRect.bottom() || containerBottom < selectionRect.top()) continue;
        container->getKeysInRect(selectionRect, containerTop,
                                 pixelsPerFrame, minViewedFrame,
                                 listKeys);
    }
}

void BoxesListWidget::addItemForBox(BoundingBox *box)
{
    BoxItemWidgetContainer *itemWidgetContainer;
    int insertIndex = 0;
    int boxZ = box->getZIndex();
    foreach(BoxItemWidgetContainer *widget, mBoxWidgetContainers) {
        int zT = widget->getTargetBox()->getZIndex();
        if(zT > boxZ) insertIndex++;
    }
    if(box->isGroup()) {
        BoxesGroup *group = (BoxesGroup*)box;
        itemWidgetContainer = new BoxesGroupWidgetContainer(group, this);
        connect(group, &BoxesGroup::addAnimatedBoundingBoxSignal,
                (BoxesGroupWidgetContainer*)itemWidgetContainer,
                &BoxesGroupWidgetContainer::addWidgetForChildBox);
        connect(group, &BoxesGroup::removeAnimatedBoundingBoxSignal,
                (BoxesGroupWidgetContainer*)itemWidgetContainer,
                &BoxesGroupWidgetContainer::removeWidgetForChildBox);
    } else {
        itemWidgetContainer = new BoxItemWidgetContainer(box, this);
    }
    mBoxesLayout->insertWidget(insertIndex, itemWidgetContainer);
    mBoxWidgetContainers << itemWidgetContainer;
}

void BoxesListWidget::removeItemForBox(BoundingBox *box) {
    foreach(BoxItemWidgetContainer *widget, mBoxWidgetContainers) {
        if(widget->getTargetBox() == box) {
            mBoxWidgetContainers.removeOne(widget);
            delete widget;
        }
    }
}

void BoxesListWidget::changeItemZ(int from, int to) {
    BoxItemWidgetContainer *toInsert;
    int insertIndex = 0;
    foreach(BoxItemWidgetContainer *widget, mBoxWidgetContainers) {
        int zT = widget->getTargetBox()->getZIndex();
        if(zT == to) {
            toInsert = widget;
            mBoxesLayout->removeWidget(toInsert);
            continue;
        }
        if(zT > to) insertIndex++;
    }
    mBoxesLayout->insertWidget(insertIndex,
                               toInsert);
}
