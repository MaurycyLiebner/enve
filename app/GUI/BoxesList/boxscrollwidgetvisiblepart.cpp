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

BoxSingleWidget *BoxScrollWidgetVisiblePart::
            getClosestsSingleWidgetWithTargetType(
                const SWT_TargetTypes &types,
                const int &yPos,
                bool *isBelow) {
    auto singleWidgetUnderMouse = getBSWAtPos(yPos);
    if(singleWidgetUnderMouse ? singleWidgetUnderMouse->isVisible() : false) {
        const auto target = singleWidgetUnderMouse->getTarget();
        if(types.isTargeted(target)) {
            *isBelow = 2*(yPos % MIN_WIDGET_HEIGHT) > MIN_WIDGET_HEIGHT;
            return singleWidgetUnderMouse;
        }
    }
    const int idAtPos = getIdAtPos(yPos);
    for(int i = idAtPos - 1; i >= 0; i--) {
        const auto swAbove = static_cast<BoxSingleWidget*>(
                mSingleWidgets.at(i));
        if(swAbove->isHidden()) continue;
        const auto target = swAbove->getTarget();
        if(types.isTargeted(target)) {
            *isBelow = true;
            return swAbove;
        }
    }
    for(int i = idAtPos + 1; i < mSingleWidgets.count(); i++) {
        const auto swBelow = static_cast<BoxSingleWidget*>(
                mSingleWidgets.at(i));
        if(swBelow->isHidden()) break;
        const auto target = swBelow->getTarget();
        if(types.isTargeted(target)) {
            *isBelow = false;
            return swBelow;
        }
    }

    return nullptr;
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
    mDragging = false;
    const int yPos = event->pos().y();
    if(BoundingBoxMimeData::hasFormat(event->mimeData())) {
        bool below;

        SWT_TargetTypes type;
        type.targetsFunctionList =
                QList<SWT_Checker>({&SingleWidgetTarget::SWT_isBoundingBox});
        const auto singleWidgetUnderMouse =
                getClosestsSingleWidgetWithTargetType(type, yPos, &below);
        if(!singleWidgetUnderMouse) return;

        auto bbMimeData = static_cast<const BoundingBoxMimeData*>(event->mimeData());
        const auto box = bbMimeData->getTarget();
        const auto boxUnderMouse = GetAsPtr(singleWidgetUnderMouse->
                 getTargetAbstraction()->getTarget(), BoundingBox);
        const auto parentGroup = boxUnderMouse->getParentGroup();
        if(!parentGroup || boxUnderMouse->isAncestor(box)) return;
        if(parentGroup != box->getParentGroup()) {
            const auto boxSPtr = GetAsSPtr(box, BoundingBox);
            box->getParentGroup()->removeContainedBox_k(boxSPtr);
            parentGroup->addContainedBox(boxSPtr);
            box->applyTransformationInverted(box->getTransformAnimator());
        }
        if(below) { // add box below
            parentGroup->moveContainedBoxAbove( // boxesgroup list is reversed
                        box,
                        boxUnderMouse);
        } else { // add box above
            parentGroup->moveContainedBoxBelow(
                        box,
                        boxUnderMouse);
        }
    } else if(PixmapEffectMimeData::hasFormat(event->mimeData())) {
        bool below;

        SWT_TargetTypes type;
        type.targetsFunctionList =
                QList<SWT_Checker>({&SingleWidgetTarget::SWT_isPixmapEffect});
        const auto singleWidgetUnderMouse =
                getClosestsSingleWidgetWithTargetType(type, yPos, &below);
        if(!singleWidgetUnderMouse) return;

        auto peMimeData = static_cast<const PixmapEffectMimeData*>(event->mimeData());
        const auto effect = peMimeData->getTarget();
        auto targetUnderMouse = singleWidgetUnderMouse->getTargetAbstraction()->getTarget();
        auto effectUnderMouse = GetAsSPtr(targetUnderMouse, PixmapEffect);

        if(effect != effectUnderMouse) {
            auto underMouseAnimator = effectUnderMouse->getParentEffectAnimators();
            auto draggedAnimator = effect->getParentEffectAnimators();
            if(draggedAnimator != underMouseAnimator) {
                qsptr<PixmapEffect> effectPtr = GetAsSPtr(effect, PixmapEffect);
                underMouseAnimator->getParentBox()->addEffect(effectPtr);
                draggedAnimator->getParentBox()->removeEffect(effectPtr);
            }
            if(below) { // add box below
                underMouseAnimator->ca_moveChildAbove( // boxesgroup list is reversed
                            effect,
                            effectUnderMouse.get());
            } else { // add box above
                underMouseAnimator->ca_moveChildBelow(
                            effect,
                            effectUnderMouse.get());
            }
            underMouseAnimator->getParentBox()->prp_updateInfluenceRangeAfterChanged();
        }
    } else if(PathEffectMimeData::hasFormat(event->mimeData())) {
        bool below;

        SWT_TargetTypes type;
        type.targetsFunctionList =
                QList<SWT_Checker>({&SingleWidgetTarget::SWT_isPathEffect});
        const auto singleWidgetUnderMouse =
                getClosestsSingleWidgetWithTargetType(type, yPos, &below);
        if(!singleWidgetUnderMouse) return;

        const auto pathEffectMimeData =
                static_cast<const PathEffectMimeData*>(event->mimeData());
        const auto effect = GetAsSPtr(pathEffectMimeData->getTarget(), PathEffect);
        const auto targetUnderMouse = singleWidgetUnderMouse->
                getTargetAbstraction()->getTarget();
        const auto effectUnderMouse = static_cast<PathEffect*>(targetUnderMouse);

        if(effect != effectUnderMouse) {
            const auto underMouseAnimator = effectUnderMouse->getParentEffectAnimators();
            const auto draggedAnimator = effect->getParentEffectAnimators();
            if(draggedAnimator != underMouseAnimator) {
                if(underMouseAnimator->isOutline()) {
                    underMouseAnimator->getParentBox()->addOutlinePathEffect(effect);
                    effect->setIsOutlineEffect(true);
                } else if(draggedAnimator->isFill()) {
                    underMouseAnimator->getParentBox()->addFillPathEffect(effect);
                    effect->setIsOutlineEffect(false);
                } else {
                    underMouseAnimator->getParentBox()->addPathEffect(effect);
                    effect->setIsOutlineEffect(false);
                }

                if(draggedAnimator->isOutline()) {
                    draggedAnimator->getParentBox()->removeOutlinePathEffect(effect);
                } else if(draggedAnimator->isFill()) {
                    draggedAnimator->getParentBox()->removeFillPathEffect(effect);
                } else {
                    draggedAnimator->getParentBox()->removePathEffect(effect);
                }
            }
            if(below) { // add box below
                underMouseAnimator->ca_moveChildAbove( // boxesgroup list is reversed
                            effect.get(),
                            effectUnderMouse);
            } else { // add box above
                underMouseAnimator->ca_moveChildBelow(
                            effect.get(),
                            effectUnderMouse);
            }
            underMouseAnimator->getParentBox()->prp_updateInfluenceRangeAfterChanged();
        }
    }
    scheduleUpdateVisibleWidgetsContent();
    MainWindow::getInstance()->queScheduledTasksAndUpdate();
}

void BoxScrollWidgetVisiblePart::dragEnterEvent(QDragEnterEvent *event) {
    //mDragging = true;
    if(BoundingBoxMimeData::hasFormat(event->mimeData()) ||
       PixmapEffectMimeData::hasFormat(event->mimeData()) ||
       PathEffectMimeData::hasFormat(event->mimeData())) {
        event->acceptProposedAction();
    }
}

void BoxScrollWidgetVisiblePart::dragLeaveEvent(QDragLeaveEvent *event) {
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
    int yPos = event->pos().y();

    if(yPos < 30) {
        if(!mScrollTimer->isActive()) {
            connect(mScrollTimer, SIGNAL(timeout()),
                    this, SLOT(scrollUp()));
            mScrollTimer->start(300);
        }
    } else if(yPos > height() - 30) {
        if(!mScrollTimer->isActive()) {
            connect(mScrollTimer, SIGNAL(timeout()),
                    this, SLOT(scrollDown()));
            mScrollTimer->start(300);
        }
    } else {
        mScrollTimer->disconnect();
        mScrollTimer->stop();
    }
    mLastDragMoveY = yPos;
    if(BoundingBoxMimeData::hasFormat(event->mimeData())) {
        mLastDragMoveTargetTypes.targetsFunctionList =
                QList<SWT_Checker>(
                        {&SingleWidgetTarget::SWT_isBoundingBox,
                         &SingleWidgetTarget::SWT_isBoxesGroup});
    } else if(PixmapEffectMimeData::hasFormat(event->mimeData())) {
        mLastDragMoveTargetTypes.targetsFunctionList =
                QList<SWT_Checker>(
                        {&SingleWidgetTarget::SWT_isPixmapEffect});
    } else if(PathEffectMimeData::hasFormat(event->mimeData())) {
        mLastDragMoveTargetTypes.targetsFunctionList =
                QList<SWT_Checker>(
                        {&SingleWidgetTarget::SWT_isPathEffect});
    }

    updateDraggingHighlight();
}

void BoxScrollWidgetVisiblePart::updateDraggingHighlight() {
    mDragging = false;
    bool below;
    const auto singleWidgetUnderMouse =
            getClosestsSingleWidgetWithTargetType(mLastDragMoveTargetTypes,
                                                  mLastDragMoveY,
                                                  &below);
    if(singleWidgetUnderMouse) {
        int currentDragPosId = singleWidgetUnderMouse->y()/MIN_WIDGET_HEIGHT;
        if(below) {
            //currentDragPosId++;
            const auto targetUnderMouse =
                    singleWidgetUnderMouse->getTargetAbstraction();
            currentDragPosId += targetUnderMouse->getHeight(
                        getCurrentRulesCollection(), true, false,
                        MIN_WIDGET_HEIGHT)/MIN_WIDGET_HEIGHT;
        }
        mDragging = true;

        mCurrentDragLine = QLine(singleWidgetUnderMouse->x(),
                                 currentDragPosId*MIN_WIDGET_HEIGHT,
                                 width(),
                                 currentDragPosId*MIN_WIDGET_HEIGHT);
    }
    update();
}

void BoxScrollWidgetVisiblePart::scrollUp() {
    mParentWidget->scrollParentAreaBy(-MIN_WIDGET_HEIGHT);
    updateDraggingHighlight();
}

void BoxScrollWidgetVisiblePart::scrollDown() {
    mParentWidget->scrollParentAreaBy(MIN_WIDGET_HEIGHT);
    updateDraggingHighlight();
}
