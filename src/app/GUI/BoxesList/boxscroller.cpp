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

#include "boxscroller.h"
#include "boxsinglewidget.h"
#include <QPainter>
#include "Animators/qrealanimator.h"
#include "boxscrollwidget.h"
#include <QTimer>
#include <QMimeData>
#include "Boxes/boundingbox.h"
#include "Boxes/containerbox.h"
#include "GUI/mainwindow.h"
#include "GUI/global.h"
#include "swt_abstraction.h"
#include "GUI/keysview.h"
#include "RasterEffects/rastereffectcollection.h"

BoxScroller::BoxScroller(ScrollWidget * const parent) :
    ScrollWidgetVisiblePart(parent) {
    setAcceptDrops(true);
    mScrollTimer = new QTimer(this);
}

QWidget *BoxScroller::createNewSingleWidget() {
    return new BoxSingleWidget(this);
}

void BoxScroller::paintEvent(QPaintEvent *) {
    QPainter p(this);

    int currY = MIN_WIDGET_DIM;
    p.setPen(QPen(QColor(40, 40, 40), 1));
    const auto parent = static_cast<BoxScrollWidget*>(parentWidget());
    const int parentContHeight = parent->getContentHeight() - MIN_WIDGET_DIM;
    while(currY < parentContHeight) {
        p.drawLine(0, currY, width(), currY);
        currY += MIN_WIDGET_DIM;
    }

    if(mDropTarget.isValid()) {
        p.setPen(QPen(Qt::white, 2));
        p.drawRect(mCurrentDragRect);
    }

    p.end();
}

bool BoxScroller::tryDropIntoAbs(SWT_Abstraction* const abs,
                                 const int idInAbs,
                                 DropTarget& dropTarget) {
    if(!abs) return false;
    const auto target = abs->getTarget();
    const int id = qBound(0, idInAbs, abs->childrenCount());
    if(!target->SWT_dropIntoSupport(id, mCurrentMimeData)) return false;
    dropTarget = DropTarget{abs, id, DropType::into};
    return true;
}

BoxScroller::DropTarget BoxScroller::getClosestDropTarget(const int yPos) {
    const auto mainAbs = getMainAbstration();
    if(!mainAbs) return DropTarget();
    const int idAtPos = yPos / MIN_WIDGET_DIM;
    DropTarget target;
    const auto& wids = widgets();
    const int nWidgets = wids.count();
    if(idAtPos >= 0 && idAtPos < nWidgets) {
        const auto bsw = static_cast<BoxSingleWidget*>(wids.at(idAtPos));
        if(bsw->isHidden()) {
            const int nChildren = mainAbs->childrenCount();
            if(tryDropIntoAbs(mainAbs, nChildren, target)) {
                mCurrentDragRect = QRect(0, visibleCount()*MIN_WIDGET_DIM, width(), 1);
                return target;
            }
        } else if(bsw->getTargetAbstraction()) {
            const auto abs = bsw->getTargetAbstraction();
            const bool above = yPos % MIN_WIDGET_DIM < MIN_WIDGET_DIM*0.5;
            bool dropOn = false;
            {
                const qreal posFrac = qreal(yPos)/MIN_WIDGET_DIM;
                if(qAbs(qRound(posFrac) - posFrac) > 0.333) dropOn = true;
                if(!above && abs->contentVisible() &&
                   abs->childrenCount() > 0) dropOn = true;
            }
            for(const bool iDropOn : {dropOn, !dropOn}) {
                if(iDropOn) {
                    if(abs->getTarget()->SWT_dropSupport(mCurrentMimeData)) {
                        mCurrentDragRect = bsw->rect().translated(bsw->pos());
                        return {abs, 0, DropType::on};
                    }
                } else {
                    const auto parentAbs = abs->getParent();
                    if(parentAbs) {
                        const int id = abs->getIdInParent() + (above ? 0 : 1);
                        if(tryDropIntoAbs(parentAbs, id, target)) {
                            const int y = bsw->y() + (above ? 0 : abs->getHeight());
                            mCurrentDragRect = QRect(bsw->x(), y,  width(), 1);
                            return target;
                        }
                    }
                }
            }
        }
    }
    for(int i = idAtPos - 1; i >= 0; i--) {
        const auto bsw = static_cast<BoxSingleWidget*>(wids.at(i));
        if(!bsw->isHidden() && bsw->getTargetAbstraction()) {
            const auto abs = bsw->getTargetAbstraction();
            if(abs->getTarget()->SWT_dropSupport(mCurrentMimeData)) {
                mCurrentDragRect = bsw->rect().translated(bsw->pos());
                return {abs, 0, DropType::on};
            }
            const auto parentAbs = abs->getParent();
            if(parentAbs) {
                const int id = abs->getIdInParent() + 1;
                if(tryDropIntoAbs(parentAbs, id, target)) {
                    const int y = bsw->y() + abs->getHeight();
                    mCurrentDragRect = QRect(bsw->x(), y, width(), 1);
                    return target;
                }
            }
        }
    }

    for(int i = idAtPos + 1; i < wids.count(); i++) {
        const auto bsw = static_cast<BoxSingleWidget*>(wids.at(i));
        if(!bsw->isHidden() && bsw->getTargetAbstraction()) {
            const auto abs = bsw->getTargetAbstraction();
            if(tryDropIntoAbs(abs, 0, target)) {
                mCurrentDragRect = QRect(bsw->x() + MIN_WIDGET_DIM,
                                         bsw->y() + MIN_WIDGET_DIM,
                                         width(), 1);
                return target;
            }
        }
    }
    return DropTarget();
}

void BoxScroller::stopScrolling() {
    if(mScrollTimer->isActive()) {
        mScrollTimer->disconnect();
        mScrollTimer->stop();
    }
}

void BoxScroller::dropEvent(QDropEvent *event) {
    stopScrolling();
    mCurrentMimeData = event->mimeData();
    mLastDragMoveY = event->pos().y();
    updateDropTarget();
    if(mDropTarget.isValid()) {
        const auto targetAbs = mDropTarget.fTargetParent;
        const auto target = targetAbs->getTarget();
        if(mDropTarget.fDropType == DropType::on) {
            target->SWT_drop(mCurrentMimeData);
        } else if(mDropTarget.fDropType == DropType::into) {
            target->SWT_dropInto(mDropTarget.fTargetId, mCurrentMimeData);
        }
        planScheduleUpdateVisibleWidgetsContent();
        Document::sInstance->actionFinished();
    }
    mCurrentMimeData = nullptr;
    mDropTarget.reset();
}

void BoxScroller::dragEnterEvent(QDragEnterEvent *event) {
    const auto mimeData = event->mimeData();
    mLastDragMoveY = event->pos().y();
    mCurrentMimeData = mimeData;
    updateDropTarget();
    //mDragging = true;
    if(mCurrentMimeData) event->acceptProposedAction();
    update();
}

void BoxScroller::dragLeaveEvent(QDragLeaveEvent *event) {
    mCurrentMimeData = nullptr;
    mDropTarget.reset();
    stopScrolling();
    event->accept();
    update();
}

void BoxScroller::dragMoveEvent(QDragMoveEvent *event) {
    event->acceptProposedAction();
    const int yPos = event->pos().y();

    if(yPos < 30) {
        if(!mScrollTimer->isActive()) {
            connect(mScrollTimer, &QTimer::timeout,
                    this, &BoxScroller::scrollUp);
            mScrollTimer->start(300);
        }
    } else if(yPos > height() - 30) {
        if(!mScrollTimer->isActive()) {
            connect(mScrollTimer, &QTimer::timeout,
                    this, &BoxScroller::scrollDown);
            mScrollTimer->start(300);
        }
    } else {
        mScrollTimer->disconnect();
        mScrollTimer->stop();
    }
    mLastDragMoveY = yPos;

    updateDropTarget();
    update();
}

void BoxScroller::updateDropTarget() {
    mDropTarget = getClosestDropTarget(mLastDragMoveY);
}

void BoxScroller::scrollUp() {
    parentWidget()->scrollParentAreaBy(-MIN_WIDGET_DIM);
    updateDropTarget();
    update();
}

void BoxScroller::scrollDown() {
    parentWidget()->scrollParentAreaBy(MIN_WIDGET_DIM);
    updateDropTarget();
    update();
}
