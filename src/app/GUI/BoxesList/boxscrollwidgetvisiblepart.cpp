#include "boxscrollwidgetvisiblepart.h"
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
#include "singlewidgetabstraction.h"
#include "GUI/keysview.h"
#include "Animators/rastereffectanimators.h"

BoxScroller::BoxScroller(
        ScrollWidget * const parent) :
    ScrollWidgetVisiblePart(parent) {
    setAcceptDrops(true);
    mScrollTimer = new QTimer(this);
}

QWidget *BoxScroller::createNewSingleWidget() {
    return new BoxSingleWidget(this);
}

void BoxScroller::paintEvent(QPaintEvent *) {
    QPainter p(this);

//    p.fillRect(rect(), Qt::red);
    int currY = MIN_WIDGET_DIM;
    p.setPen(QPen(QColor(40, 40, 40), 1));
    auto parentWidgetT = static_cast<BoxScrollWidget*>(mParentWidget);
    int parentContHeight = parentWidgetT->getContentHeight();
    while(currY < parentContHeight) {
        p.drawLine(0, currY, width(), currY);
        currY += MIN_WIDGET_DIM;
    }

    if(mDragging) {
        p.setPen(QPen(Qt::white, 3));
        p.drawLine(mCurrentDragLine);
    }

    p.end();
}

void BoxScroller::drawKeys(QPainter * const p,
                                          const qreal pixelsPerFrame,
                                          const FrameRange &viewedFrameRange) {
    p->save();
    p->setPen(Qt::NoPen);
    for(const auto& container : mSingleWidgets) {
        const auto bsw = static_cast<BoxSingleWidget*>(container);
        p->save();
        bsw->drawKeys(p, pixelsPerFrame, viewedFrameRange);
        p->restore();
        p->translate(0, container->height());
    }
    p->restore();
}

Key *BoxScroller::getKeyAtPos(
        const int pressX, const int pressY,
        const qreal pixelsPerFrame,
        const int minViewedFrame) {
    const int remaining = pressY % MIN_WIDGET_DIM;
    if(remaining < (MIN_WIDGET_DIM - KEY_RECT_SIZE)/2 ||
       remaining > (MIN_WIDGET_DIM + KEY_RECT_SIZE)/2) return nullptr;
    for(const auto& container : mSingleWidgets) {
        const int containerTop = container->y();
        const int containerBottom = containerTop + container->height();
        if(containerTop > pressY || containerBottom < pressY) continue;
        const auto bsw = static_cast<BoxSingleWidget*>(container);
        return bsw->getKeyAtPos(pressX, pixelsPerFrame, minViewedFrame);
    }
    return nullptr;
}

DurationRectangleMovable *BoxScroller::getRectangleMovableAtPos(
        const int pressX,
        const int pressY,
        const qreal pixelsPerFrame,
        const int minViewedFrame) {
    for(const auto& container : mSingleWidgets) {
        const int containerTop = container->y();
        const int containerBottom = containerTop + container->height();
        if(containerTop > pressY || containerBottom < pressY) continue;
        const auto bsw = static_cast<BoxSingleWidget*>(container);
        return bsw->getRectangleMovableAtPos(pressX, pixelsPerFrame,
                                             minViewedFrame);
    }
    return nullptr;
}

void BoxScroller::getKeysInRect(
        QRectF selectionRect,
        const qreal pixelsPerFrame,
        QList<Key *>& listKeys) {
    if(!mMainAbstraction) return;
    QList<SWT_Abstraction*> abstractions;
//    selectionRect.adjust(-0.5, -(BOX_HEIGHT/* + KEY_RECT_SIZE*/)*0.5,
//                         0.5, (BOX_HEIGHT/* + KEY_RECT_SIZE*/)*0.5);
    selectionRect.adjust(0.5, 0, 0.5, 0);
    const int minX = qRound(selectionRect.top() - MIN_WIDGET_DIM*0.5);
    const int minY = qRound(selectionRect.bottom() - MIN_WIDGET_DIM*0.5);
    int currY = 0;
    const SetAbsFunc setter = [&abstractions](SWT_Abstraction * abs, int) {
        abstractions.append(abs);
    };
    mMainAbstraction->setAbstractions(
            minX, minY, currY, 0, MIN_WIDGET_DIM,
            setter, mCurrentRulesCollection, true, false);

    for(const auto& abs : abstractions) {
        const auto target = abs->getTarget();
        if(target->SWT_isAnimator()) {
            const auto anim_target = GetAsPtr(target, Animator);
            anim_target->anim_getKeysInRect(selectionRect, pixelsPerFrame,
                                            listKeys, KEY_RECT_SIZE);
        }
    }
}

int BoxScroller::getIdAtPos(const int yPos) const {
    return yPos / MIN_WIDGET_DIM;
}

BoxSingleWidget * BoxScroller::getBSWAtPos(const int yPos) const {
    const int idAtYPos = getIdAtPos(yPos);
    if(idAtYPos < 0) return nullptr;
    if(idAtYPos >= mSingleWidgets.count()) return nullptr;
    return static_cast<BoxSingleWidget*>(mSingleWidgets.at(idAtYPos));
}

BoxSingleWidget * BoxScroller::getLastVisibleBSW() const {
    for(int i = mSingleWidgets.count() - 1; i >= 0; i--) {
        const auto bsw = mSingleWidgets.at(i);
        if(bsw->isVisible()) return static_cast<BoxSingleWidget*>(bsw);
    }
    return nullptr;
}

BoxScroller::DropTarget BoxScroller::getClosestDropTarget(const int yPos) const {
    BoxSingleWidget * targetBSW = nullptr;
    DropTypes supportedDropTypes = DROP_NONE;
    targetBSW = getBSWAtPos(yPos);
    if(targetBSW) {
        if(targetBSW->isHidden() && mMainAbstraction) {
            const bool dropToMainAbs =
                    droppingSupported(mMainAbstraction,
                                      mMainAbstraction->getChildrenCount());
            if(dropToMainAbs) {
                return DropTarget{mMainAbstraction,
                                  mMainAbstraction->getChildrenCount()};
            }
        } else {
            if(targetBSW->getTargetAbstraction() == mCurrentlyDragged.fPtr)
                return {nullptr, DROP_NONE};
            supportedDropTypes = dropOnBSWSupported(targetBSW);
        }
    }

    const int idAtPos = getIdAtPos(yPos);
    const int relYPos = yPos % MIN_WIDGET_DIM;
    bool above = relYPos < MIN_WIDGET_DIM*0.5;
    for(int i = idAtPos - 1; i >= 0; i--) {
        if(supportedDropTypes) break;
        targetBSW = static_cast<BoxSingleWidget*>(
                    mSingleWidgets.at(i));
        if(targetBSW->isHidden()) continue;
        if(targetBSW->getTargetAbstraction() == mCurrentlyDragged.fPtr)
            return {nullptr, DROP_NONE};
        supportedDropTypes = dropOnBSWSupported(targetBSW) &
                (DROP_ABOVE | DROP_BELOW);
        above = false;
    }

    for(int i = idAtPos + 1; i < mSingleWidgets.count(); i++) {
        if(supportedDropTypes) break;
        targetBSW = static_cast<BoxSingleWidget*>(
                mSingleWidgets.at(i));
        if(targetBSW->isHidden()) break;
        if(targetBSW->getTargetAbstraction() == mCurrentlyDragged.fPtr)
            return {nullptr, DROP_NONE};
        supportedDropTypes = dropOnBSWSupported(targetBSW) &
                (DROP_ABOVE | DROP_BELOW);
        above = true;
    }

    if(!supportedDropTypes) return {nullptr , DROP_NONE};
    if(!targetBSW) return {nullptr , DROP_NONE};
    const auto targetAbs = targetBSW->getTargetAbstraction();
    if(!targetAbs) return {nullptr , DROP_NONE};
    const bool suppAbove = supportedDropTypes & DROP_ABOVE;
    const bool suppInto = supportedDropTypes & DROP_INTO;
    const bool suppBelow = supportedDropTypes & DROP_BELOW;
    if(suppInto) {
        //if(qAbs(relYPos - MIN_WIDGET_HEIGHT*0.5) < MIN_WIDGET_HEIGHT*0.5) {
        if(!above) {
            int firstId = 0;
            if(mCurrentlyDragged.fType == Dragged::BOX) {
                const auto targetUnderMouse = targetAbs->getTarget();
                if(targetUnderMouse->SWT_isContainerBox()) {
                    const auto bbUnderMouse = GetAsPtr(targetUnderMouse, ContainerBox);
                    firstId = bbUnderMouse->ca_getNumberOfChildren();
                }
            }
            return DropTarget{targetAbs, firstId};
        }
    }
    if((above || ! suppBelow) && suppAbove) {
        return DropTarget{targetAbs->getParent(),
                          targetAbs->getIdInParent()};
    } else if(suppBelow) {
        return DropTarget{targetAbs->getParent(),
                          targetAbs->getIdInParent() + 1};
    }
    return {nullptr , DROP_NONE};
}

void BoxScroller::stopScrolling() {
    if(mScrollTimer->isActive()) {
        mScrollTimer->disconnect();
        mScrollTimer->stop();
    }
}
#include "PathEffects/patheffect.h"
#include "PathEffects/patheffectanimators.h"
void BoxScroller::dropEvent(QDropEvent *event) {
    stopScrolling();
    updateDraggedFromMimeData(event->mimeData());
    mLastDragMoveY = event->pos().y();
    updateDropTarget();
    if(mCurrentlyDragged.isValid() && mDropTarget.isValid()) {
        if(mDropTarget.drop(mCurrentlyDragged)) {
            planScheduleUpdateVisibleWidgetsContent();
            Document::sInstance->actionFinished();
        } else update();
    }
    mDragging = false;
    mCurrentlyDragged.reset();
    mDropTarget.reset();
}

void BoxScroller::dragEnterEvent(QDragEnterEvent *event) {
    const auto mimeData = event->mimeData();
    mLastDragMoveY = event->pos().y();
    updateDraggedFromMimeData(mimeData);
    updateDropTarget();
    //mDragging = true;
    if(mCurrentlyDragged.isValid()) event->acceptProposedAction();
    update();
}

void BoxScroller::dragLeaveEvent(QDragLeaveEvent *event) {
    mCurrentlyDragged.reset();
    mDropTarget.reset();
    mDragging = false;
    stopScrolling();
//    if(mScrollTimer->isActive()) {
//        QPoint mousePos = mapFromGlobal(QCursor::pos());
//        if(mousePos.x() < 20 || mousePos.x() > width() - 20) {
//            mScrollTimer->disconnect();
//            mScrollTimer->stop();
//        } else {
//            mScrollTimer->setInterval(100);
//        }
//    }
    event->accept();
    update();
}

#include <QDebug>
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

bool BoxScroller::droppingSupported(
        const SWT_Abstraction * const targetAbs,
        const int idInTarget) const {
    if(!targetAbs) return false;
    if(targetAbs == mCurrentlyDragged.fPtr) return false;
    const auto targetSWT = targetAbs->getTarget();
    if(!targetSWT) return false;
    if(mCurrentlyDragged.fType == Dragged::BOX) {
        if(!targetSWT->SWT_isContainerBox()) return false;
        const auto draggedAbs = mCurrentlyDragged.fPtr;
        const auto draggedBox = static_cast<BoundingBox*>(
                    draggedAbs->getTarget());
        const auto targetGroup = static_cast<const ContainerBox*>(
                    targetSWT);
        if(idInTarget < targetGroup->ca_getNumberOfChildren()) return false;
        if(targetGroup->isAncestor(draggedBox)) return false;
    } else if(mCurrentlyDragged.fType == Dragged::RASTER_EFFECT) {
        if(!targetSWT->SWT_isRasterEffectAnimators()) return false;
    } else if(mCurrentlyDragged.fType == Dragged::PATH_EFFECT) {
        if(!targetSWT->SWT_isPathEffectAnimators()) return false;
    } else return false;

    return true;
}

BoxScroller::DropTypes BoxScroller::dropOnSWTSupported(
        SingleWidgetTarget const * const swtUnderMouse) const {
    if(!swtUnderMouse) return DropType::DROP_NONE;
    const auto absUnderMouse = swtUnderMouse->SWT_getAbstractionForWidget(mId);
    if(!absUnderMouse) return DropType::DROP_NONE;
    const auto parentAbs = absUnderMouse->getParent();
    if(!parentAbs) return DropType::DROP_NONE;
    const int targetIdInParent = absUnderMouse->getIdInParent();
    const bool dropAbove = droppingSupported(parentAbs, targetIdInParent);
    int firstId = 0;
    if(mCurrentlyDragged.fType == Dragged::BOX) {
        const auto targetUnderMouse = absUnderMouse->getTarget();
        if(targetUnderMouse->SWT_isContainerBox()) {
            const auto bbUnderMouse = GetAsPtr(targetUnderMouse, ContainerBox);
            firstId = bbUnderMouse->ca_getNumberOfChildren();
        }
    }
    const bool dropInto = droppingSupported(absUnderMouse, firstId);
    const bool dropBelow = droppingSupported(parentAbs, targetIdInParent + 1);
    return (dropAbove ? DropType::DROP_ABOVE : DropType::DROP_NONE) |
           (dropInto ? DropType::DROP_INTO : DropType::DROP_NONE) |
           (dropBelow ? DropType::DROP_BELOW : DropType::DROP_NONE);
}

BoxScroller::DropTypes BoxScroller::dropOnBSWSupported(
        BoxSingleWidget const * const bswUnderMouse) const {
    if(!bswUnderMouse) return DropType::DROP_NONE;
    if(bswUnderMouse->isHidden()) return DropType::DROP_NONE;
    const auto swtUnderMouse = bswUnderMouse->getTarget();
    return dropOnSWTSupported(swtUnderMouse);
}

void BoxScroller::updateDragLine() {
    mDragging = false;
    if(!mDropTarget.isValid()) return;
    int i = -1;
    for(const auto& widget : mSingleWidgets) {
        i++;
        const auto bsw = static_cast<BoxSingleWidget*>(widget);
        if(bsw->isHidden()) break;
        const auto abs = bsw->getTargetAbstraction();
        if(!abs) continue;
        if(abs->getParent() == mDropTarget.fTargetParent) {
            int yPos;
            if(abs->getIdInParent() == mDropTarget.fTargetId) { // above
                yPos = bsw->y();
            } else if(abs->getIdInParent() == mDropTarget.fTargetId - 1) { // below
                const auto absParent = abs->getParent();
                yPos = bsw->y() + abs->getHeight(getCurrentRulesCollection(),
                                                 true,
                                                 absParent->isMainTarget(),
                                                 MIN_WIDGET_DIM);
            } else continue;
            mCurrentDragLine = QLine(bsw->x(), yPos, width(), yPos);
            mDragging = true;
        }
    }
}

void BoxScroller::updateDropTarget() {
    mDropTarget = getClosestDropTarget(mLastDragMoveY);
    updateDragLine();
}

void BoxScroller::scrollUp() {
    mParentWidget->scrollParentAreaBy(-MIN_WIDGET_DIM);
    updateDropTarget();
    update();
}

void BoxScroller::scrollDown() {
    mParentWidget->scrollParentAreaBy(MIN_WIDGET_DIM);
    updateDropTarget();
    update();
}

void BoxScroller::updateDraggedFromMimeData(
        const QMimeData * const mimeData) {
    if(!mimeData->hasFormat("enveInternalFormat")) {
        mCurrentlyDragged = Dragged{nullptr, Dragged::NONE};
        return;
    }
    const auto emimeData = static_cast<const eMimeData*>(mimeData);
    const SingleWidgetTarget * swt = nullptr;
    Dragged::Type type = Dragged::NONE;
    if(emimeData->hasType<BoundingBox>()) {
        swt = emimeData->getObjects<BoundingBox>().first();
        type = Dragged::BOX;
    } else if(emimeData->hasType<RasterEffect>()) {
        swt = emimeData->getObjects<RasterEffect>().first();
        type = Dragged::RASTER_EFFECT;
    } else if(emimeData->hasType<PathEffect>()) {
        swt = emimeData->getObjects<PathEffect>().first();
        type = Dragged::PATH_EFFECT;
    }

    const auto abs = swt ? swt->SWT_getAbstractionForWidget(mId) : nullptr;
    mCurrentlyDragged = abs ? Dragged{abs, type} :
                              Dragged{nullptr, Dragged::NONE};
}

bool BoxScroller::DropTarget::drop(
        const BoxScroller::Dragged &dragged) {
    if(!isValid() || !dragged.isValid()) return false;
    const auto draggedAbs = dragged.fPtr;
    const auto draggedSWT = draggedAbs->getTarget();
    const auto targetSWT = fTargetParent->getTarget();
    if(dragged.fType == Dragged::BOX) {
        const auto draggedBox = GetAsSPtr(draggedSWT, BoundingBox);
        const auto targetGroup = GetAsPtr(targetSWT, ContainerBox);
        const auto currentDraggedParent = draggedBox->getParentGroup();
        int boxTargetId = targetGroup->abstractionIdToBoxId(fTargetId) + 1;
        if(currentDraggedParent != targetGroup) {
            currentDraggedParent->removeContainedBox_k(draggedBox);
            targetGroup->insertContainedBox(boxTargetId, draggedBox);
        } else {
            const int targetBoxId = targetGroup->abstractionIdToBoxId(fTargetId) + 1;
            if(targetBoxId == draggedBox->getZIndex() ||
               targetBoxId == draggedBox->getZIndex() + 1) return false;

            if(draggedBox->getZIndex() < boxTargetId) boxTargetId--;
            currentDraggedParent->moveContainedBoxInList(
                        draggedBox.get(), boxTargetId);
        }
    } else if(dragged.fType == Dragged::RASTER_EFFECT) {
        const auto draggedEffect = GetAsSPtr(draggedSWT, ShaderEffect);
        auto targetParent = static_cast<RasterEffectAnimators*>(targetSWT);
        auto currentParent = draggedEffect->getParent<RasterEffectAnimators>();
        if(currentParent != targetParent) {
            targetParent->getParentBox()->addRasterEffect(draggedEffect);
            currentParent->getParentBox()->removeRasterEffect(draggedEffect);
        } else {
            if(fTargetId == draggedAbs->getIdInParent() ||
               fTargetId == draggedAbs->getIdInParent() + 1) return false;
            int targetId = fTargetId;
            if(draggedAbs->getIdInParent() < fTargetId) targetId--;
            currentParent->ca_moveChildInList(draggedEffect.get(), fTargetId);
        }
        draggedEffect->prp_afterWholeInfluenceRangeChanged();
    } else if(dragged.fType == Dragged::PATH_EFFECT) {
        const auto draggedEffect = GetAsSPtr(draggedSWT, PathEffect);
        const auto targetParent = static_cast<PathEffectAnimators*>(targetSWT);
        const auto currentParent = draggedEffect->getParent<PathEffectAnimators>();

        if(currentParent != targetParent) {
            currentParent->removeChild(draggedEffect);
            targetParent->addChild(draggedEffect);
        } else {
            if(fTargetId == draggedAbs->getIdInParent() ||
               fTargetId == draggedAbs->getIdInParent() + 1) return false;
            int targetId = fTargetId;
            if(draggedAbs->getIdInParent() < fTargetId) targetId--;
            currentParent->ca_moveChildInList(draggedEffect.get(), targetId);
        }
        draggedEffect->prp_afterWholeInfluenceRangeChanged();
    } else return false;
    return true;
}
