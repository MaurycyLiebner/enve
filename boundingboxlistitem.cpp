#include "boundingbox.h"
#include "boxeslist.h"
#include "boxesgroup.h"
#include "undoredo.h"
#include <QPainter>

void BoundingBox::handleListItemMousePress(qreal relX, qreal relY) {
    Q_UNUSED(relY);
    if(relX < LIST_ITEM_HEIGHT) {
        setChildrenListItemsVisible(!mChildrenListItemsVisibile);
    } else if(relX < LIST_ITEM_HEIGHT*2) {
        setVisibile(!mVisible);
    } else if(relX < LIST_ITEM_HEIGHT*3) {
        setLocked(!mLocked);
    } else {
        if(isVisibleAndUnlocked() && ((BoxesGroup*)mParent)->isCurrentGroup()) {
            if(isShiftPressed()) {
                if(mSelected) {
                    ((BoxesGroup*)mParent)->removeBoxFromSelection(this);
                } else {
                    ((BoxesGroup*)mParent)->addBoxToSelection(this);
                }
            } else {
                ((BoxesGroup*)mParent)->clearBoxesSelection();
                ((BoxesGroup*)mParent)->addBoxToSelection(this);
            }
            scheduleBoxesListRepaint();
            scheduleRepaint();
        }
    }
}

void BoxesGroup::handleChildListItemMousePress(qreal relX, qreal relY,
                                               qreal y0) {
    qreal currentY = y0;
    foreach(BoundingBox *box, mChildren) {
        qreal boxHeight = box->getListItemHeight();
        if(relY - currentY < boxHeight) {
            box->handleListItemMousePress(relX, relY - currentY);
            break;
        }
        currentY += boxHeight;
    }
}

void BoxesGroup::handleListItemMousePress(qreal relX, qreal relY) {
    if(relY < LIST_ITEM_HEIGHT) {
        BoundingBox::handleListItemMousePress(relX, relY);
    } else {
        handleChildListItemMousePress(relX - LIST_ITEM_HEIGHT, relY,
                                      LIST_ITEM_HEIGHT);
    }
}

void BoundingBox::drawListItem(QPainter *p, qreal drawX, qreal drawY, qreal maxY) {
    Q_UNUSED(maxY);
    if(mSelected) {
        p->setBrush(QColor(185, 185, 185));
    } else {
        p->setBrush(QColor(225, 225, 225));
    }
    p->drawRect(QRectF(drawX, drawY,
                       LIST_ITEM_MAX_WIDTH - drawX, LIST_ITEM_HEIGHT));
    drawX += LIST_ITEM_HEIGHT;
    if(mVisible) {
        p->drawPixmap(drawX, drawY, *BoxesList::VISIBLE_PIXMAP);
    } else {
        p->drawPixmap(drawX, drawY, *BoxesList::INVISIBLE_PIXMAP);
    }
    drawX += LIST_ITEM_HEIGHT;
    if(mLocked) {
        p->drawPixmap(drawX, drawY, *BoxesList::LOCKED_PIXMAP);
    } else {
        p->drawPixmap(drawX, drawY, *BoxesList::UNLOCKED_PIXMAP);
    }
    drawX += 2*LIST_ITEM_HEIGHT;
    p->drawText(QRectF(drawX, drawY,
                       LIST_ITEM_MAX_WIDTH - drawX - LIST_ITEM_HEIGHT,
                       LIST_ITEM_HEIGHT),
                mName, QTextOption(Qt::AlignVCenter));
}

void BoxesGroup::drawChildren(QPainter *p,
                              qreal drawX, qreal drawY,
                              qreal minY, qreal maxY) {
    qreal currentY = drawY;
    qreal currentDrawY = drawY;
    foreach (BoundingBox *box, mChildren) {
        qreal boxHeight = box->getListItemHeight();
        if(currentY >= minY) {
            if(currentY > maxY) {
                break;
            }
            box->drawListItem(p, drawX, currentDrawY, maxY);
            currentDrawY += boxHeight;
        }
        currentY += boxHeight;
    }
}

void BoxesGroup::drawListItem(QPainter *p, qreal drawX, qreal drawY, qreal maxY)
{
    BoundingBox::drawListItem(p, drawX, drawY, maxY);
    if(mChildrenListItemsVisibile) {
        p->drawPixmap(drawX, drawY, *BoxesList::HIDE_CHILDREN);

        drawX += LIST_ITEM_CHILD_INDENT;
        drawY += LIST_ITEM_HEIGHT;
        drawChildren(p, drawX, drawY, 0.f, maxY);
    } else {
        p->drawPixmap(drawX, drawY, *BoxesList::SHOW_CHILDREN);
    }
}

qreal BoxesGroup::getListItemHeight() {
    qreal height = BoundingBox::getListItemHeight();
    if(mChildrenListItemsVisibile) {
        foreach(BoundingBox *box, mChildren) {
            height += box->getListItemHeight();
        }
    }
    return height;
}

qreal BoundingBox::getListItemHeight() {
    return LIST_ITEM_HEIGHT;
}

void BoundingBox::setChildrenListItemsVisible(bool bt)
{
    mChildrenListItemsVisibile = bt;
    scheduleBoxesListRepaint();
}

void BoundingBox::showChildrenListItems()
{
    setChildrenListItemsVisible(true);
}

void BoundingBox::hideChildrenListItems()
{
    setChildrenListItemsVisible(false);
}

void BoundingBox::setName(QString name)
{
    mName = name;
    scheduleBoxesListRepaint();
}

QString BoundingBox::getName()
{
    return mName;
}

void BoundingBox::setVisibile(bool visible, bool saveUndoRedo) {
    if(mVisible == visible) return;
    if(mSelected) {
        ((BoxesGroup*) mParent)->removeBoxFromSelection(this);
    }
    if(saveUndoRedo) {
        addUndoRedo(new SetBoxVisibleUndoRedo(this, mVisible, visible));
    }
    mVisible = visible;
    scheduleBoxesListRepaint();
    scheduleRepaint();
}

void BoundingBox::hide()
{
    setVisibile(false);
}

void BoundingBox::show()
{
    setVisibile(true);
}

bool BoundingBox::isVisibleAndUnlocked() {
    return mVisible && !mLocked;
}

bool BoundingBox::isVisible()
{
    return mVisible;
}

bool BoundingBox::isLocked() {
    return mLocked;
}

void BoundingBox::lock() {
    setLocked(true);
}

void BoundingBox::unlock() {
    setLocked(false);
}

void BoundingBox::setLocked(bool bt) {
    if(bt == mLocked) return;
    if(mSelected) {
        ((BoxesGroup*) mParent)->removeBoxFromSelection(this);
    }
    mLocked = bt;
    scheduleBoxesListRepaint();
}
