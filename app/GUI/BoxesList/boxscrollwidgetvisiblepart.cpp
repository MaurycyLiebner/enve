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
    p.setPen(QPen(QColor(40, 40, 40), 1.));
    auto parentWidgetT = static_cast<BoxScrollWidget*>(mParentWidget);
    int parentContHeight = parentWidgetT->getContentHeight();
    while(currY < parentContHeight) {
        p.drawLine(0, currY, width(), currY);

        currY += MIN_WIDGET_HEIGHT;
    }

    if(mDragging) {
        p.setPen(QPen(Qt::white, 3.));
        p.drawLine(0, mCurrentDragPosId*MIN_WIDGET_HEIGHT,
                   width(), mCurrentDragPosId*MIN_WIDGET_HEIGHT);
    }

    p.end();
}

void BoxScrollWidgetVisiblePart::drawKeys(QPainter *p,
                                          const qreal &pixelsPerFrame,
                                          const int &minViewedFrame,
                                          const int &maxViewedFrame) {
    //p->setPen(QPen(Qt::black, 1.));
    p->setPen(Qt::NoPen);
    for(QWidget *container : mSingleWidgets) {
        static_cast<BoxSingleWidget*>(container)->drawKeys(
                            p, pixelsPerFrame,
                            container->y(),
                            minViewedFrame, maxViewedFrame);
    }
}

Key *BoxScrollWidgetVisiblePart::getKeyAtPos(
        const int &pressX, const int &pressY,
        const qreal &pixelsPerFrame,
        const int &minViewedFrame) {
    int remaining = pressY % MIN_WIDGET_HEIGHT;
    if(remaining < (MIN_WIDGET_HEIGHT - KEY_RECT_SIZE)/2 ||
       remaining > (MIN_WIDGET_HEIGHT + KEY_RECT_SIZE)/2) return nullptr;
    for(QWidget *container : mSingleWidgets) {
        int containerTop = container->y();
        int containerBottom = containerTop + container->height();
        if(containerTop > pressY || containerBottom < pressY) continue;
        return static_cast<BoxSingleWidget*>(container)->
                getKeyAtPos(pressX, pixelsPerFrame,
                            minViewedFrame);
    }
    return nullptr;
}

DurationRectangleMovable *BoxScrollWidgetVisiblePart::getRectangleMovableAtPos(
        const int &pressX,
        const int &pressY,
        const qreal &pixelsPerFrame,
        const int &minViewedFrame) {
    for(QWidget *container : mSingleWidgets) {
        int containerTop = container->y();
        int containerBottom = containerTop + container->height();
        if(containerTop > pressY || containerBottom < pressY) continue;
        return static_cast<BoxSingleWidget*>(container)->
                getRectangleMovableAtPos(pressX, pixelsPerFrame,
                                         minViewedFrame);
    }
    return nullptr;
}

void BoxScrollWidgetVisiblePart::getKeysInRect(QRectF selectionRect,
        qreal pixelsPerFrame,
        QList<Key *>& listKeys) {
    QList<SingleWidgetAbstraction*> abstractions;
//    selectionRect.adjust(-0.5, -(BOX_HEIGHT/* + KEY_RECT_SIZE*/)*0.5,
//                         0.5, (BOX_HEIGHT/* + KEY_RECT_SIZE*/)*0.5);
    selectionRect.adjust(0.5, 0., 0.5, 0.);
    int currY = 0;
    int minX = qRound(selectionRect.top() - MIN_WIDGET_HEIGHT*0.5);
    int minY = qRound(selectionRect.bottom() - MIN_WIDGET_HEIGHT*0.5);

    mMainAbstraction->getAbstractions(
            minX, minY, currY, 0, MIN_WIDGET_HEIGHT,
            abstractions, mCurrentRulesCollection, true, false);

    for(SingleWidgetAbstraction *abs : abstractions) {
        SingleWidgetTarget *target = abs->getTarget();
        if(target->SWT_isAnimator()) {
            Animator *anim_target = GetAsPtr(target, Animator);
            anim_target->anim_getKeysInRect(selectionRect, pixelsPerFrame,
                                           listKeys, KEY_RECT_SIZE);
        }
    }
//    for(SingleWidget *container : mSingleWidgets) {
//        int containerTop = container->y();
//        int containerBottom = containerTop + container->height();
//        if(containerTop > selectionRect.bottom() ||
//           containerBottom < selectionRect.top()) continue;
//        ((BoxSingleWidget*)container)->
//                getKeysInRect(selectionRect,
//                              pixelsPerFrame,
//                              listKeys);
//    }
}

BoxSingleWidget *BoxScrollWidgetVisiblePart::
            getClosestsSingleWidgetWithTargetTypeLookBelow(
                const SWT_TargetTypes &type,
                const int &yPos,
                bool *isBelow) {
    int idAtYPos = yPos / MIN_WIDGET_HEIGHT;
    int targetId = (yPos + MIN_WIDGET_HEIGHT/2) /
                    MIN_WIDGET_HEIGHT;
    if(idAtYPos < mSingleWidgets.count() &&
       idAtYPos >= 0) {
        auto singleWidgetUnderMouse = static_cast<BoxSingleWidget*>(
                mSingleWidgets.at(idAtYPos));
        while(singleWidgetUnderMouse->isHidden()) {
            idAtYPos++;
            if(idAtYPos >= mSingleWidgets.count()) return nullptr;
            singleWidgetUnderMouse = static_cast<BoxSingleWidget*>(
                    mSingleWidgets.at(idAtYPos));
        }
        auto targetAbs = singleWidgetUnderMouse->getTargetAbstraction();
        while(!type.isTargeted(targetAbs->getTarget()) ) {
            idAtYPos++;
            if(idAtYPos >= mSingleWidgets.count()) return nullptr;
            singleWidgetUnderMouse = static_cast<BoxSingleWidget*>(
                    mSingleWidgets.at(idAtYPos));
            targetAbs = singleWidgetUnderMouse->getTargetAbstraction();
        }
        *isBelow = targetId > idAtYPos;
        return singleWidgetUnderMouse;
    }
    return nullptr;
}

BoxSingleWidget *BoxScrollWidgetVisiblePart::
            getClosestsSingleWidgetWithTargetType(
                const SWT_TargetTypes &types,
                const int &yPos,
                bool *isBelow) {
    int idAtYPos = yPos / MIN_WIDGET_HEIGHT;
    int targetId = (yPos + MIN_WIDGET_HEIGHT/2)/
                    MIN_WIDGET_HEIGHT;
    if(idAtYPos < mSingleWidgets.count() &&
       idAtYPos >= 0) {
        BoxSingleWidget *singleWidgetUnderMouse =
                static_cast<BoxSingleWidget*>(
                mSingleWidgets.at(idAtYPos));
        while(singleWidgetUnderMouse->isHidden()) {
            idAtYPos--;
            if(idAtYPos < 0) {
                return getClosestsSingleWidgetWithTargetTypeLookBelow(
                            types, yPos, isBelow);
            }
            singleWidgetUnderMouse = static_cast<BoxSingleWidget*>(
                    mSingleWidgets.at(idAtYPos));
        }
        while(!types.isTargeted(singleWidgetUnderMouse->getTargetAbstraction()->
                                getTarget())) {
            idAtYPos--;
            if(idAtYPos < 0) {
                return getClosestsSingleWidgetWithTargetTypeLookBelow(
                            types, yPos, isBelow);
            }
            singleWidgetUnderMouse = static_cast<BoxSingleWidget*>(
                    mSingleWidgets.at(idAtYPos));
        }
        *isBelow = targetId > idAtYPos;
        return singleWidgetUnderMouse;
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
    if(BoundingBoxMimeData::hasFormat(event->mimeData())) {
        const int yPos = event->pos().y();
        bool below;

        SWT_TargetTypes type;
        type.targetsFunctionList =
                QList<SWT_Checker>({&SingleWidgetTarget::SWT_isBoundingBox});
        const auto singleWidgetUnderMouse =
                getClosestsSingleWidgetWithTargetType(
                    type, yPos, &below);
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
        int yPos = event->pos().y();
        bool below;

        SWT_TargetTypes type;
        type.targetsFunctionList =
                QList<SWT_Checker>({&SingleWidgetTarget::SWT_isPixmapEffect});
        BoxSingleWidget *singleWidgetUnderMouse =
                getClosestsSingleWidgetWithTargetType(
                    type,
                    yPos,
                    &below);
        if(!singleWidgetUnderMouse) return;

        auto peMimeData = static_cast<const PixmapEffectMimeData*>(event->mimeData());
        PixmapEffect* effect = peMimeData->getTarget();
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
        int yPos = event->pos().y();
        bool below;

        SWT_TargetTypes type;
        type.targetsFunctionList =
                QList<SWT_Checker>({&SingleWidgetTarget::SWT_isPathEffect});
        BoxSingleWidget *singleWidgetUnderMouse =
                getClosestsSingleWidgetWithTargetType(type, yPos, &below);
        if(!singleWidgetUnderMouse) return;

        auto pathEffectMimeData =
                static_cast<const PathEffectMimeData*>(event->mimeData());
        qsptr<PathEffect> effect =
                GetAsSPtr(pathEffectMimeData->getTarget(), PathEffect);
        auto targetUnderMouse = singleWidgetUnderMouse->
                getTargetAbstraction()->getTarget();
        PathEffect *effectUnderMouse =
                static_cast<PathEffect*>(targetUnderMouse);

        if(effect != effectUnderMouse) {
            PathEffectAnimators *underMouseAnimator =
                                     effectUnderMouse->getParentEffectAnimators();
            PathEffectAnimators *draggedAnimator =
                                     effect->getParentEffectAnimators();
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
    BoxSingleWidget *singleWidgetUnderMouse =
            getClosestsSingleWidgetWithTargetType(mLastDragMoveTargetTypes,
                                                  mLastDragMoveY,
                                                  &below);
    if(singleWidgetUnderMouse) {
        int currentDragPosId = singleWidgetUnderMouse->y()/MIN_WIDGET_HEIGHT;
        if(below) {
            //currentDragPosId++;
            auto targetUnderMouse =
                    singleWidgetUnderMouse->getTargetAbstraction();
            currentDragPosId += targetUnderMouse->getHeight(
                        getCurrentRulesCollection(), true, false,
                        MIN_WIDGET_HEIGHT)/MIN_WIDGET_HEIGHT;
        }
        mDragging = true;
        mCurrentDragPosId = currentDragPosId;
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
