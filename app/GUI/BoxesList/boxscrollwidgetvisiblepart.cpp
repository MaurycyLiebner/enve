#include "boxscrollwidgetvisiblepart.h"
#include "boxsinglewidget.h"
#include <QPainter>
#include "Animators/qrealanimator.h"
#include "boxscrollwidget.h"
#include <QTimer>
#include <QMimeData>
#include "Boxes/boundingbox.h"
#include "Boxes/boxesgroup.h"
#include "GUI/mainwindow.h"
#include "global.h"
#include "singlewidgetabstraction.h"
#include "GUI/keysview.h"
#include "PixmapEffects/pixmapeffect.h"
#include "Animators/effectanimators.h"

BoxScrollWidgetVisiblePart::BoxScrollWidgetVisiblePart(
        ScrollWidget *parent) :
    ScrollWidgetVisiblePart(parent) {
    setAcceptDrops(true);
    mScrollTimer = new QTimer(this);
}

QWidget *BoxScrollWidgetVisiblePart::createNewSingleWidget() {
    return new BoxSingleWidget(this);
}

void BoxScrollWidgetVisiblePart::paintEvent(QPaintEvent *) {
    QPainter p(this);

//    p.fillRect(rect(), Qt::red);
    int currY = MIN_WIDGET_HEIGHT;
    p.setPen(QPen(QColor(40, 40, 40), 1));
    auto parentWidgetT = static_cast<BoxScrollWidget*>(mParentWidget);
    int parentContHeight = parentWidgetT->getContentHeight();
    while(currY < parentContHeight) {
        p.drawLine(0, currY, width(), currY);
        currY += MIN_WIDGET_HEIGHT;
    }

    if(mDragging) {
        p.setPen(QPen(Qt::white, 3));
        p.drawLine(mCurrentDragLine);
    }

    p.end();
}

void BoxScrollWidgetVisiblePart::drawKeys(QPainter *p,
                                          const qreal &pixelsPerFrame,
                                          const int &minViewedFrame,
                                          const int &maxViewedFrame) {
    //p->setPen(QPen(Qt::black, 1));
    p->setPen(Qt::NoPen);
    for(const auto& container : mSingleWidgets) {
        const auto bsw = static_cast<BoxSingleWidget*>(container);
        bsw->drawKeys(p, pixelsPerFrame, container->y(),
                      minViewedFrame, maxViewedFrame);
    }
}

Key *BoxScrollWidgetVisiblePart::getKeyAtPos(
        const int &pressX, const int &pressY,
        const qreal &pixelsPerFrame,
        const int &minViewedFrame) {
    const int remaining = pressY % MIN_WIDGET_HEIGHT;
    if(remaining < (MIN_WIDGET_HEIGHT - KEY_RECT_SIZE)/2 ||
       remaining > (MIN_WIDGET_HEIGHT + KEY_RECT_SIZE)/2) return nullptr;
    for(const auto& container : mSingleWidgets) {
        const int containerTop = container->y();
        const int containerBottom = containerTop + container->height();
        if(containerTop > pressY || containerBottom < pressY) continue;
        const auto bsw = static_cast<BoxSingleWidget*>(container);
        return bsw->getKeyAtPos(pressX, pixelsPerFrame, minViewedFrame);
    }
    return nullptr;
}

DurationRectangleMovable *BoxScrollWidgetVisiblePart::getRectangleMovableAtPos(
        const int &pressX,
        const int &pressY,
        const qreal &pixelsPerFrame,
        const int &minViewedFrame) {
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

void BoxScrollWidgetVisiblePart::getKeysInRect(
        QRectF selectionRect,
        const qreal& pixelsPerFrame,
        QList<Key *>& listKeys) {
    QList<SingleWidgetAbstraction*> abstractions;
//    selectionRect.adjust(-0.5, -(BOX_HEIGHT/* + KEY_RECT_SIZE*/)*0.5,
//                         0.5, (BOX_HEIGHT/* + KEY_RECT_SIZE*/)*0.5);
    selectionRect.adjust(0.5, 0, 0.5, 0);
    const int minX = qRound(selectionRect.top() - MIN_WIDGET_HEIGHT*0.5);
    const int minY = qRound(selectionRect.bottom() - MIN_WIDGET_HEIGHT*0.5);
    int currY = 0;
    mMainAbstraction->getAbstractions(
            minX, minY, currY, 0, MIN_WIDGET_HEIGHT,
            abstractions, mCurrentRulesCollection, true, false);

    for(const auto& abs : abstractions) {
        const auto target = abs->getTarget();
        if(target->SWT_isAnimator()) {
            const auto anim_target = GetAsPtr(target, Animator);
            anim_target->anim_getKeysInRect(selectionRect, pixelsPerFrame,
                                            listKeys, KEY_RECT_SIZE);
        }
    }
}

int BoxScrollWidgetVisiblePart::getIdAtPos(const int& yPos) const {
    return yPos / MIN_WIDGET_HEIGHT;
}

BoxSingleWidget * BoxScrollWidgetVisiblePart::getBSWAtPos(const int& yPos) const {
    const int idAtYPos = getIdAtPos(yPos);
    if(idAtYPos < 0) return nullptr;
    if(idAtYPos >= mSingleWidgets.count()) return nullptr;
    return static_cast<BoxSingleWidget*>(mSingleWidgets.at(idAtYPos));
}

BoxSingleWidget * BoxScrollWidgetVisiblePart::getLastVisibleBSW() const {
    for(int i = mSingleWidgets.count() - 1; i >= 0; i--) {
        const auto bsw = mSingleWidgets.at(i);
        if(bsw->isVisible()) return static_cast<BoxSingleWidget*>(bsw);
    }
    return nullptr;
}

#define DropTarget_ BoxScrollWidgetVisiblePart::DropTarget
DropTarget_ BoxScrollWidgetVisiblePart::getClosestDropTarget(
        const int &yPos) const {
    BoxSingleWidget * targetBSW = nullptr;
    DropTypes supportedDropTypes = DROP_NONE;
    targetBSW = getBSWAtPos(yPos);

    supportedDropTypes = dropOnBSWSupported(targetBSW);

    const int idAtPos = getIdAtPos(yPos);

    for(int i = idAtPos - 1; i >= 0; i--) {
        if(supportedDropTypes) break;
        targetBSW = static_cast<BoxSingleWidget*>(
                    mSingleWidgets.at(i));
        supportedDropTypes = dropOnBSWSupported(targetBSW) & DROP_ABOVE;
    }

    for(int i = idAtPos + 1; i < mSingleWidgets.count(); i++) {
        if(supportedDropTypes) break;
        targetBSW = static_cast<BoxSingleWidget*>(
                mSingleWidgets.at(i));
        supportedDropTypes = dropOnBSWSupported(targetBSW) & DROP_BELOW;
    }

    if(!supportedDropTypes) return {nullptr , DROP_NONE};
    if(!targetBSW) return {nullptr , DROP_NONE};
    const auto targetAbs = targetBSW->getTargetAbstraction();
    if(!targetAbs) return {nullptr , DROP_NONE};
    const bool suppAbove = supportedDropTypes & DROP_ABOVE;
    const bool suppInto = supportedDropTypes & DROP_INTO;
    const int relYPos = yPos % MIN_WIDGET_HEIGHT;
    if(suppInto) {
        if(qAbs(relYPos - MIN_WIDGET_HEIGHT*0.5) < MIN_WIDGET_HEIGHT*0.5) {
            return DropTarget{targetAbs, 0};
        }
    }
    const bool above = relYPos < MIN_WIDGET_HEIGHT*0.5;
    if(above && suppAbove) {
        return DropTarget{targetAbs->getParent(),
                          targetAbs->getIdInParent()};
    } else {
        return DropTarget{targetAbs->getParent(),
                          targetAbs->getIdInParent() + 1};
    }
}

void BoxScrollWidgetVisiblePart::stopScrolling() {
    if(mScrollTimer->isActive()) {
        mScrollTimer->disconnect();
        mScrollTimer->stop();
    }
}
#include "PathEffects/patheffect.h"
#include "PathEffects/patheffectanimators.h"
void BoxScrollWidgetVisiblePart::dropEvent(QDropEvent *event) {
    stopScrolling();
    updateDraggedFromMimeData(event->mimeData());
    mLastDragMoveY = event->pos().y();
    updateDropTarget();
    if(mCurrentlyDragged.isValid() && mDropTarget.isValid()) {
        if(mDropTarget.drop(mCurrentlyDragged)) {
            scheduleUpdateVisibleWidgetsContent();
            MainWindow::getInstance()->queScheduledTasksAndUpdate();
        } else update();
    }
    mDragging = false;
    mCurrentlyDragged.reset();
    mDropTarget.reset();
}

void BoxScrollWidgetVisiblePart::dragEnterEvent(QDragEnterEvent *event) {
    const auto mimeData = event->mimeData();
    mLastDragMoveY = event->pos().y();
    updateDraggedFromMimeData(mimeData);
    //mDragging = true;
    if(mCurrentlyDragged.isValid()) event->acceptProposedAction();
}

void BoxScrollWidgetVisiblePart::dragLeaveEvent(QDragLeaveEvent *event) {
    mCurrentlyDragged.reset();
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
    update();
    event->accept();
}

#include <QDebug>
void BoxScrollWidgetVisiblePart::dragMoveEvent(QDragMoveEvent *event) {
    event->acceptProposedAction();
    const int yPos = event->pos().y();

    if(yPos < 30) {
        if(!mScrollTimer->isActive()) {
            connect(mScrollTimer, &QTimer::timeout,
                    this, &BoxScrollWidgetVisiblePart::scrollUp);
            mScrollTimer->start(300);
        }
    } else if(yPos > height() - 30) {
        if(!mScrollTimer->isActive()) {
            connect(mScrollTimer, &QTimer::timeout,
                    this, &BoxScrollWidgetVisiblePart::scrollDown);
            mScrollTimer->start(300);
        }
    } else {
        mScrollTimer->disconnect();
        mScrollTimer->stop();
    }
    mLastDragMoveY = yPos;

    updateDropTarget();
}

bool BoxScrollWidgetVisiblePart::droppingSupported(
        const SingleWidgetAbstraction * const targetAbs,
        const int& idInTarget) const {
    if(!targetAbs) return false;
    if(targetAbs == mCurrentlyDragged.fPtr) return false;
    const auto targetSWT = targetAbs->getTarget();
    if(!targetSWT) return false;
    if(mCurrentlyDragged.fType == Dragged::BOX) {
        if(!targetSWT->SWT_isBoxesGroup()) return false;
        const auto draggedAbs = mCurrentlyDragged.fPtr;
        const auto draggedBox = static_cast<BoundingBox*>(
                    draggedAbs->getTarget());
        const auto targetGroup = static_cast<const BoxesGroup*>(
                    targetSWT);
        const auto currentParent = draggedBox->getParentGroup();
        if(currentParent == targetGroup &&
           idInTarget == draggedAbs->getIdInParent()) return false;
        if(targetGroup->isAncestor(draggedBox)) return false;
    } else if(mCurrentlyDragged.fType == Dragged::RASTER_EFFECT) {
        if(!targetSWT->SWT_isPixmapEffectAnimators()) return false;
        const auto draggedAbs = mCurrentlyDragged.fPtr;
        const auto draggedEffect = static_cast<PixmapEffect*>(
                    draggedAbs->getTarget());
        const auto targetParent = static_cast<const EffectAnimators*>(
                    targetSWT);
        const auto currentParent = draggedEffect->getParentEffectAnimators();
        if(currentParent == targetParent &&
           idInTarget == draggedAbs->getIdInParent()) return false;
    } else if(mCurrentlyDragged.fType == Dragged::PATH_EFFECT) {
        if(!targetSWT->SWT_isPathEffectAnimators()) return false;
        const auto draggedAbs = mCurrentlyDragged.fPtr;
        const auto draggedEffect = static_cast<PathEffect*>(
                    draggedAbs->getTarget());
        const auto targetParent = static_cast<const PathEffectAnimators*>(
                    targetSWT);
        const auto currentParent = draggedEffect->getParentEffectAnimators();
        if(currentParent == targetParent &&
           idInTarget == draggedAbs->getIdInParent()) return false;
    } else return false;

    return true;
}
#define DropTypes_ BoxScrollWidgetVisiblePart::DropTypes
DropTypes_ BoxScrollWidgetVisiblePart::dropOnBSWSupported(
        BoxSingleWidget const * const bswUnderMouse) const {
    if(!bswUnderMouse) return DropType::DROP_NONE;
    if(bswUnderMouse->isHidden()) return DropType::DROP_NONE;
    const auto swtUnderMouse = bswUnderMouse->getTarget();
    if(!swtUnderMouse) return DropType::DROP_NONE;
    const auto absUnderMouse = swtUnderMouse->SWT_getAbstractionForWidget(mId);
    if(!absUnderMouse) return DropType::DROP_NONE;
    const auto parentAbs = absUnderMouse->getParent();
    if(!parentAbs) return DropType::DROP_NONE;
    const int targetIdInParent = absUnderMouse->getIdInParent();
    const bool dropAbove = droppingSupported(parentAbs, targetIdInParent);
    const bool dropInto = droppingSupported(absUnderMouse, 0);
    const bool dropBelow = droppingSupported(parentAbs, targetIdInParent + 1);
    return (dropAbove ? DropType::DROP_ABOVE : DropType::DROP_NONE) |
           (dropInto ? DropType::DROP_INTO : DropType::DROP_NONE) |
           (dropBelow ? DropType::DROP_BELOW : DropType::DROP_NONE);
}

void BoxScrollWidgetVisiblePart::updateDropTarget() {
    mDropTarget = getClosestDropTarget(mLastDragMoveY);
}

void BoxScrollWidgetVisiblePart::scrollUp() {
    mParentWidget->scrollParentAreaBy(-MIN_WIDGET_HEIGHT);
    updateDropTarget();
    update();
}

void BoxScrollWidgetVisiblePart::scrollDown() {
    mParentWidget->scrollParentAreaBy(MIN_WIDGET_HEIGHT);
    updateDropTarget();
    update();
}

void BoxScrollWidgetVisiblePart::updateDraggedFromMimeData(
        const QMimeData * const mimeData) {
    if(BoundingBoxMimeData::hasFormat(mimeData)) {
        mLastDragMoveTargetTypes.fTargetsFunctionList = QList<SWT_Checker>(
                        {&SingleWidgetTarget::SWT_isBoundingBox,
                         &SingleWidgetTarget::SWT_isBoxesGroup});
    } else if(PixmapEffectMimeData::hasFormat(mimeData)) {
        mLastDragMoveTargetTypes.fTargetsFunctionList = QList<SWT_Checker>(
                        {&SingleWidgetTarget::SWT_isPixmapEffect});
    } else if(PathEffectMimeData::hasFormat(mimeData)) {
        mLastDragMoveTargetTypes.fTargetsFunctionList = QList<SWT_Checker>(
                        {&SingleWidgetTarget::SWT_isPathEffect});
    }

    const SingleWidgetTarget * swt = nullptr;
    Dragged::Type type = Dragged::NONE;
    if(BoundingBoxMimeData::hasFormat(mimeData)) {
        auto bbMimeData = static_cast<const BoundingBoxMimeData*>(mimeData);
        swt = bbMimeData->getTarget();
        type = Dragged::BOX;
    } else if(PixmapEffectMimeData::hasFormat(mimeData)) {
        auto peMimeData = static_cast<const PixmapEffectMimeData*>(mimeData);
        swt = peMimeData->getTarget();
        type = Dragged::RASTER_EFFECT;
    } else if(PathEffectMimeData::hasFormat(mimeData)) {
        auto peMimeData = static_cast<const PathEffectMimeData*>(mimeData);
        swt = peMimeData->getTarget();
        type = Dragged::PATH_EFFECT;
    }

    const auto abs = swt ? swt->SWT_getAbstractionForWidget(mId) : nullptr;
    mCurrentlyDragged = abs ? Dragged{abs, type} :
                              Dragged{nullptr, Dragged::NONE};
}

bool BoxScrollWidgetVisiblePart::DropTarget::drop(
        const BoxScrollWidgetVisiblePart::Dragged &dragged) {
    if(!isValid() || !dragged.isValid()) return false;
    const auto draggedAbs = dragged.fPtr;
    const auto draggedSWT = draggedAbs->getTarget();
    const auto targetSWT = fTargetParent->getTarget();
    if(dragged.fType == Dragged::BOX) {
        const auto draggedBox = GetAsSPtr(draggedSWT, BoundingBox);
        const auto targetGroup = GetAsPtr(targetSWT, BoxesGroup);
        const auto currentDraggedParent = draggedBox->getParentGroup();
        if(currentDraggedParent != targetGroup) {
            currentDraggedParent->removeContainedBox_k(draggedBox);
            targetGroup->addContainedBoxToListAt(fTargetId, draggedBox);
            draggedBox->applyTransformationInverted(
                        draggedBox->getTransformAnimator());
        } else {
            int targetId = fTargetId;
            if(draggedAbs->getIdInParent() < fTargetId) targetId--;
            currentDraggedParent->moveContainedBoxInList(
                        draggedBox.get(), targetId);
        }
    } else if(dragged.fType == Dragged::RASTER_EFFECT) {
        const auto draggedEffect = GetAsSPtr(draggedSWT, PixmapEffect);
        auto targetParent = static_cast<EffectAnimators*>(targetSWT);
        auto currentParent = draggedEffect->getParentEffectAnimators();
        if(currentParent != targetParent) {
            targetParent->getParentBox()->addEffect(draggedEffect);
            currentParent->getParentBox()->removeEffect(draggedEffect);
        } else {
            int targetId = fTargetId;
            if(draggedAbs->getIdInParent() < fTargetId) targetId--;
            currentParent->ca_moveChildInList(draggedEffect.get(), fTargetId);
        }
        targetParent->getParentBox()->prp_updateInfluenceRangeAfterChanged();
    } else if(dragged.fType == Dragged::PATH_EFFECT) {
        const auto draggedEffect = GetAsSPtr(draggedSWT, PathEffect);
        const auto targetParent = static_cast<PathEffectAnimators*>(targetSWT);
        const auto currentParent = draggedEffect->getParentEffectAnimators();
        if(currentParent->isOutline()) {
            currentParent->getParentBox()->removeOutlinePathEffect(draggedEffect);
        } else if(currentParent->isFill()) {
            currentParent->getParentBox()->removeFillPathEffect(draggedEffect);
        } else {
            currentParent->getParentBox()->removePathEffect(draggedEffect);
        }

        if(currentParent != targetParent) {
            if(targetParent->isOutline()) {
                targetParent->getParentBox()->addOutlinePathEffect(draggedEffect);
                draggedEffect->setIsOutlineEffect(true);
            } else if(targetParent->isFill()) {
                targetParent->getParentBox()->addFillPathEffect(draggedEffect);
                draggedEffect->setIsOutlineEffect(false);
            } else {
                targetParent->getParentBox()->addPathEffect(draggedEffect);
                draggedEffect->setIsOutlineEffect(false);
            }
        } else {
            int targetId = fTargetId;
            if(draggedAbs->getIdInParent() < fTargetId) targetId--;
            currentParent->ca_moveChildInList(draggedEffect.get(), targetId);
        }
        targetParent->getParentBox()->prp_updateInfluenceRangeAfterChanged();
    } else return false;
    return true;
}
