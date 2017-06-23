#include "Boxes/boxesgroup.h"
#include "undoredo.h"
#include <QApplication>
#include "mainwindow.h"
#include "ctrlpoint.h"
#include "Boxes/circle.h"
#include "Boxes/rectangle.h"
#include "BoxesList/boxscrollwidget.h"
#include "textbox.h"
#include "BoxesList/OptimalScrollArea/scrollwidgetvisiblepart.h"
#include "canvaswindow.h"
#include "canvas.h"

bool BoxesGroup::mCtrlsAlwaysVisible = false;

//bool zMoreThan(BoundingBox *box1, BoundingBox *box2)
//{
//    return box1->getZIndex() < box2->getZIndex();
//}

BoxesGroup::BoxesGroup(BoxesGroup *parent) :
    BoundingBox(parent, BoundingBoxType::TYPE_GROUP) {
    setName("Group");
    mFillStrokeSettingsWidget = getMainWindow()->getFillStrokeSettings();
}

BoxesGroup::BoxesGroup(FillStrokeSettingsWidget *fillStrokeSetting) :
    BoundingBox(BoundingBoxType::TYPE_CANVAS) {
    mFillStrokeSettingsWidget = fillStrokeSetting;
}


void BoxesGroup::updateAllBoxes() {
    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mChildBoxes) {
        child->updateAllBoxes();
    }
    scheduleSoftUpdate();
}

void BoxesGroup::clearAllCache() {
    BoundingBox::clearAllCache();
    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mChildBoxes) {
        child->clearAllCache();
    }
}

void BoxesGroup::updateCombinedTransformTmp() {
    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mChildBoxes) {
        child->updateCombinedTransformTmp();
    }
    if(!mIsDescendantCurrentGroup) {
        updateDrawRenderContainerTransform();
    }
}

BoxesGroup::~BoxesGroup() {

}

void BoxesGroup::prp_loadFromSql(const int &boundingBoxId) {
    BoundingBox::prp_loadFromSql(boundingBoxId);
    loadChildrenFromSql(boundingBoxId, false);
}

#include "linkbox.h"
BoundingBox *BoxesGroup::createLink(BoxesGroup *parent) {
    InternalLinkBoxesGroup *linkGroup =
                        new InternalLinkBoxesGroup(this, parent);
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
        box->createSameTransformationLink(linkGroup);
    }
    return linkGroup;
}

BoundingBox *BoxesGroup::createSameTransformationLink(BoxesGroup *parent) {
    SameTransformInternalLinkBoxesGroup *linkGroup =
                        new SameTransformInternalLinkBoxesGroup(this, parent);
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
        box->createSameTransformationLink(linkGroup);
    }
    return linkGroup;
}


BoxesGroup *BoxesGroup::loadChildrenFromSql(const int &thisBoundingBoxId,
                                            const bool &loadInBox) {
    QString thisBoundingBoxIdStr = QString::number(thisBoundingBoxId);
    if(loadInBox) {
        BoxesGroup *newGroup = new BoxesGroup(this);
        newGroup->loadChildrenFromSql(thisBoundingBoxId, false);
        newGroup->centerPivotPosition();
        return newGroup;
    }
    QSqlQuery query;
    QString queryStr;
    queryStr = "SELECT id, boxtype FROM boundingbox "
               "WHERE parentboundingboxid = " + thisBoundingBoxIdStr;
    if(query.exec(queryStr) ) {
        int idId = query.record().indexOf("id");
        int idBoxType = query.record().indexOf("boxtype");
        while(query.next() ) {
            if(static_cast<BoundingBoxType>(
                        query.value(idBoxType).toInt()) == TYPE_VECTOR_PATH ) {
                VectorPath::createPathFromSql(query.value(idId).toInt(), this);
            } else if(static_cast<BoundingBoxType>(
                          query.value(idBoxType).toInt()) == TYPE_GROUP ) {
                BoxesGroup *group = new BoxesGroup(this);
                group->prp_loadFromSql(query.value(idId).toInt());
            } else if(static_cast<BoundingBoxType>(
                          query.value(idBoxType).toInt()) == TYPE_CIRCLE ) {
                Circle *circle = new Circle(this);
                circle->prp_loadFromSql(query.value(idId).toInt());
            } else if(static_cast<BoundingBoxType>(
                          query.value(idBoxType).toInt()) == TYPE_TEXT ) {
                TextBox *textBox = new TextBox(this);
                textBox->prp_loadFromSql(query.value(idId).toInt());
            } else if(static_cast<BoundingBoxType>(
                          query.value(idBoxType).toInt()) == TYPE_RECTANGLE ) {
                Rectangle *rectangle = new Rectangle(this);
                rectangle->prp_loadFromSql(query.value(idId).toInt());
            }
        }
    } else {
        qDebug() << "Could not load children for boxesgroup with id " +
                    thisBoundingBoxIdStr;
    }
    return this;
}

int BoxesGroup::saveToSql(QSqlQuery *query, const int &parentId) {
    int boundingBoxId = BoundingBox::saveToSql(query, parentId);

    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
        box->saveToSql(query, boundingBoxId);
    }
    return boundingBoxId;
}

bool BoxesGroup::relPointInsidePath(const QPointF &relPos) {
    if(mRelBoundingRect.contains(relPos)) {
        QPointF absPos = mapRelPosToAbs(relPos);
        Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
            if(box->absPointInsidePath(absPos)) {
                return true;
            }
        }
    }
    return false;
}

void BoxesGroup::setStrokeCapStyle(const Qt::PenCapStyle &capStyle) {
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
        box->setStrokeCapStyle(capStyle);
    }
}

void BoxesGroup::setStrokeJoinStyle(const Qt::PenJoinStyle &joinStyle) {
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
        box->setStrokeJoinStyle(joinStyle);
    }
}

void BoxesGroup::setStrokeWidth(const qreal &strokeWidth, const bool &finish) {
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
        box->setStrokeWidth(strokeWidth, finish);
    }
}

void BoxesGroup::startSelectedStrokeWidthTransform() {
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
        box->startSelectedStrokeWidthTransform();
    }
}

void BoxesGroup::startSelectedStrokeColorTransform()
{
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
        box->startSelectedStrokeColorTransform();
    }
}

void BoxesGroup::startSelectedFillColorTransform()
{
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
        box->startSelectedFillColorTransform();
    }
}

void BoxesGroup::makeDuplicate(Property *targetBox) {
    BoundingBox::makeDuplicate(targetBox);
    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mChildBoxes) {
        child->createDuplicate((BoxesGroup*)targetBox);
    }
}

BoundingBox *BoxesGroup::createNewDuplicate(BoxesGroup *parent) {
    return new BoxesGroup(parent);
}

void BoxesGroup::updateRelBoundingRect() {
    SkPath boundingPaths = SkPath();
    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mChildBoxes) {
        SkPath childPath = child->getRelBoundingRectPath();
        childPath.transform(QMatrixToSkMatrix(child->getRelativeTransform()));
        boundingPaths.addPath(childPath);
    }
    mRelBoundingRectSk = boundingPaths.computeTightBounds();
    mRelBoundingRect = SkRectToQRectF(mRelBoundingRectSk);

    BoundingBox::updateRelBoundingRect();
}

void BoxesGroup::updateEffectsMargin() {
    qreal childrenMargin = 0.;
    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mChildBoxes) {
        childrenMargin = qMax(child->getEffectsMargin(),
                              childrenMargin);
    }
    BoundingBox::updateEffectsMargin();
    mEffectsMargin += childrenMargin;
}

void BoxesGroup::drawUpdatePixmapSk(SkCanvas *canvas) {
    if(shouldPaintOnImage()) {
        BoundingBox::drawUpdatePixmapSk(canvas);
    } else {
        SkPaint paint;
        paint.setAlpha(qRound(mUpdateOpacity*255));
        paint.setBlendMode(mBlendModeSk);
        canvas->saveLayer(NULL, &paint);
        Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
            box->drawUpdatePixmapSk(canvas);
        }
        canvas->restore();
    }
}

void BoxesGroup::addChildAwaitingUpdate(BoundingBox *child) {
    for(int i = 0; i < mChildrenAwaitingUpdate.count(); i++) {
        if(mChildrenAwaitingUpdate.at(i) == child) {
            mChildrenAwaitingUpdate.removeAt(i);
        }
    }
    mChildrenAwaitingUpdate << child->ref<BoundingBox>();

    if(mParent == NULL) return;
    scheduleSoftUpdate();
}

void BoxesGroup::beforeUpdate() {
//    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mChildrenAwaitingUpdate) {
//        child->beforeUpdate();
//        mUpdateChildrenAwaitingUpdate.append(child);
//    }

//    mChildrenAwaitingUpdate.clear();
    BoundingBox::beforeUpdate();
}

void BoxesGroup::schedulerProccessed() {
    BoundingBox::schedulerProccessed();
    Q_FOREACH(const QSharedPointer<BoundingBox> &child,
              mChildrenAwaitingUpdate) {
        child->schedulerProccessed();
        MainWindow::getInstance()->getCanvasWindow()->
                addUpdatableAwaitingUpdate(child.data());
    }

    mChildrenAwaitingUpdate.clear();
}

void BoxesGroup::processUpdate() {
//    Q_FOREACH(const QSharedPointer<BoundingBox> &child,
//              mUpdateChildrenAwaitingUpdate) {
//        child->processUpdate();
//    }
    BoundingBox::processUpdate();
}

void BoxesGroup::afterUpdate() {
//    Q_FOREACH(const QSharedPointer<BoundingBox> &child,
//            mUpdateChildrenAwaitingUpdate) {
//        child->afterUpdate();
//    }
//    mUpdateChildrenAwaitingUpdate.clear();
    BoundingBox::afterUpdate();

}

void BoxesGroup::drawPixmapSk(SkCanvas *canvas) {
    if(shouldPaintOnImage()) {
        BoundingBox::drawPixmapSk(canvas);
    } else {
        SkPaint paint;
        paint.setAlpha(qRound(mTransformAnimator->getOpacity()*2.55));
        paint.setBlendMode(mBlendModeSk);
        canvas->saveLayer(NULL, &paint);
        Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
            //box->draw(p);
            box->drawPixmapSk(canvas);
        }
        canvas->restore();
    }
}

void BoxesGroup::drawSelectedSk(SkCanvas *canvas,
                                 const CanvasMode &currentCanvasMode,
                                 const SkScalar &invScale) {
    if(isVisibleAndInVisibleDurationRect()) {
        canvas->save();
        drawBoundingRectSk(canvas, invScale);
        if(currentCanvasMode == MOVE_PATH && !mIsCurrentGroup) {
            mTransformAnimator->getPivotMovablePoint()->
                    drawSk(canvas, invScale);
        }
        canvas->restore();
    }
}

void BoxesGroup::drawSk(SkCanvas *canvas) {
    canvas->save();
    canvas->concat(QMatrixToSkMatrix(mUpdateTransform.inverted()));
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
        //box->draw(p);
        box->drawUpdatePixmapSk(canvas);
    }

    canvas->restore();
}

void BoxesGroup::setIsCurrentGroup(const bool &bT) {
    mIsCurrentGroup = bT;
    if(!bT) {
        if(mChildBoxes.isEmpty() && mParent != NULL) {
            mParent->removeChild(this);
        }
    }
    setDescendantCurrentGroup(bT);
}

bool BoxesGroup::isCurrentGroup() {
    return mIsCurrentGroup;
}

bool BoxesGroup::isDescendantCurrentGroup() {
    return mIsDescendantCurrentGroup;
}

bool BoxesGroup::shouldPaintOnImage() {
    return !mIsDescendantCurrentGroup;
    return mEffectsAnimators->hasEffects() &&
           !mIsDescendantCurrentGroup;
}

void BoxesGroup::setDescendantCurrentGroup(const bool &bT) {
    mIsDescendantCurrentGroup = bT;
    if(!bT) {
        scheduleSoftUpdate();
    }
    if(mParent == NULL) return;
    mParent->setDescendantCurrentGroup(bT);
}

BoundingBox *BoxesGroup::getPathAtFromAllAncestors(const QPointF &absPos) {
    BoundingBox *boxAtPos = NULL;
    //Q_FOREACHBoxInListInverted(mChildren) {
    for(int i = mChildBoxes.count() - 1; i >= 0; i--) {
        const QSharedPointer<BoundingBox> &box = mChildBoxes.at(i);
        if(box->isVisibleAndUnlocked()) {
            boxAtPos = box->getPathAtFromAllAncestors(absPos);
            if(boxAtPos != NULL) {
                break;
            }
        }
    }
    return boxAtPos;
}

void BoxesGroup::ungroup() {
    //clearBoxesSelection();
    BoxesGroup *parentGroup = mParent.data();
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
        box->applyTransformation(mTransformAnimator.data());
        removeChild(box.data());
        parentGroup->addChild(box.data());
    }
    mParent->removeChild(this);
}

PaintSettings *BoxesGroup::getFillSettings() {
    if(mChildBoxes.isEmpty()) return NULL;
    return mChildBoxes.first()->getFillSettings();
}

StrokeSettings *BoxesGroup::getStrokeSettings() {
    if(mChildBoxes.isEmpty()) return NULL;
    return mChildBoxes.first()->getStrokeSettings();
}

void BoxesGroup::setCurrentFillStrokeSettingsFromBox(BoundingBox *box) {
    mFillStrokeSettingsWidget->setCurrentSettings(box->getFillSettings(),
                                                  box->getStrokeSettings());
}

void BoxesGroup::applyCurrentTransformation() {
    QPointF absPivot = getPivotAbsPos();
    qreal rotation = mTransformAnimator->rot();
    qreal scaleX = mTransformAnimator->xScale();
    qreal scaleY = mTransformAnimator->yScale();
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
        box->saveTransformPivotAbsPos(absPivot);
        box->startTransform();
        box->rotateRelativeToSavedPivot(rotation);
        box->finishTransform();
        box->startTransform();
        box->scaleRelativeToSavedPivot(scaleX, scaleY);
        box->finishTransform();
    }

    mTransformAnimator->resetRotation(true);
    mTransformAnimator->resetScale(true);
}

void BoxesGroup::selectAllBoxesFromBoxesGroup() {
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
        if(box->isSelected()) continue;
        getParentCanvas()->addBoxToSelection(box.data());
    }
}

void BoxesGroup::deselectAllBoxesFromBoxesGroup() {
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
        if(box->isSelected()) {
            getParentCanvas()->removeBoxFromSelection(box.data());
        }
    }
}

BoundingBox *BoxesGroup::getBoxAt(const QPointF &absPos) {
    BoundingBox *boxAtPos = NULL;

    for(int i = mChildBoxes.count() - 1; i >= 0; i--) {
        const QSharedPointer<BoundingBox> &box = mChildBoxes.at(i);
        if(box->isVisibleAndUnlocked()) {
            if(box->absPointInsidePath(absPos)) {
                boxAtPos = box.data();
                break;
            }
        }
    }
    return boxAtPos;
}

void BoxesGroup::addContainedBoxesToSelection(const QRectF &rect) {
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
        if(box->isVisibleAndUnlocked()) {
            if(box->isContainedIn(rect) ) {
                getParentCanvas()->addBoxToSelection(box.data());
            }
        }
    }
}

void BoxesGroup::addChild(BoundingBox *child) {
    //child->setParent(this);
    addChildToListAt(mChildBoxes.count(), child);
}

void BoxesGroup::addChildToListAt(const int &index,
                                  BoundingBox *child,
                                  const bool &saveUndoRedo) {
    if(saveUndoRedo) {
        child->setParent(this);
        addUndoRedo(new AddChildToListUndoRedo(this, index, child));
    }
    connect(child, SIGNAL(prp_absFrameRangeChanged(int,int)),
            this, SLOT(prp_updateAfterChangedAbsFrameRange(int,int)));
    mChildBoxes.insert(index, child->ref<BoundingBox>());
    updateRelBoundingRect();
    updateChildrenId(index, saveUndoRedo);

    scheduleEffectsMarginUpdate();

    //SWT_addChildAbstractionForTargetToAll(child);
    SWT_addChildAbstractionForTargetToAllAt(child,
                                            ca_mChildAnimators.count());
    child->prp_setAbsFrame(anim_mCurrentAbsFrame);

    child->prp_updateInfluenceRangeAfterChanged();
}

void BoxesGroup::updateChildrenId(const int &firstId,
                                  const bool &saveUndoRedo) {
    updateChildrenId(firstId, mChildBoxes.length() - 1, saveUndoRedo);
}

void BoxesGroup::updateChildrenId(const int &firstId,
                                  const int &lastId,
                                  const bool &saveUndoRedo) {
    for(int i = firstId; i <= lastId; i++) {
        mChildBoxes.at(i)->setZListIndex(i, saveUndoRedo);
    }
}

void BoxesGroup::prp_setAbsFrame(const int &frame) {
    BoundingBox::prp_setAbsFrame(frame);

    updateDrawRenderContainerTransform();
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
        box->prp_setAbsFrame(frame);
    }
}

void BoxesGroup::removeChildFromList(const int &id,
                                     const bool &saveUndoRedo) {
    BoundingBox *box = mChildBoxes.at(id).data();
    box->clearAllCache();
    if(box->isSelected()) {
        box->removeFromSelection();
    }
    disconnect(box, 0, this, 0);
    if(saveUndoRedo) {
        addUndoRedo(new RemoveChildFromListUndoRedo(this, id,
                                                   box) );
    }
    mChildBoxes.removeAt(id);

    updateRelBoundingRect();
    if(box->isGroup()) {
        BoxesGroup *group = (BoxesGroup*) box;
        if(group->isCurrentGroup()) {
            mMainWindow->getCanvasWindow()->getCurrentCanvas()->
                    setCurrentBoxesGroup(group->getParent());
        }
    }
    updateChildrenId(id, saveUndoRedo);

    scheduleEffectsMarginUpdate();

    SWT_removeChildAbstractionForTargetFromAll(box);
}

int BoxesGroup::getChildBoxIndex(BoundingBox *child) {
    int index = -1;
    for(int i = 0; i < mChildBoxes.count(); i++) {
        if(mChildBoxes.at(i) == child) {
            index = i;
        }
    }
    return index;
}

void BoxesGroup::removeChild(BoundingBox *child) {
    const int &index = getChildBoxIndex(child);
    if(index < 0) {
        return;
    }
    removeChildFromList(index);
    if(mChildBoxes.isEmpty() &&
       mParent != NULL) {
        mParent->removeChild(this);
    }
    //child->setParent(NULL);
}


void BoxesGroup::increaseChildZInList(BoundingBox *child) {
    const int &index = getChildBoxIndex(child);
    if(index == mChildBoxes.count() - 1) {
        return;
    }
    moveChildInList(child, index, index + 1);
}

void BoxesGroup::decreaseChildZInList(BoundingBox *child) {
    const int &index = getChildBoxIndex(child);
    if(index == 0) {
        return;
    }
    moveChildInList(child, index, index - 1);
}

void BoxesGroup::bringChildToEndList(BoundingBox *child) {
    const int &index = getChildBoxIndex(child);
    if(index == mChildBoxes.count() - 1) {
        return;
    }
    moveChildInList(child, index, mChildBoxes.length() - 1);
}

void BoxesGroup::bringChildToFrontList(BoundingBox *child) {
    const int &index = getChildBoxIndex(child);
    if(index == 0) {
        return;
    }
    moveChildInList(child, index, 0);
}

void BoxesGroup::moveChildInList(BoundingBox *child,
                                 const int &from, const int &to,
                                 const bool &saveUndoRedo) {
    mChildBoxes.move(from, to);
    updateChildrenId(qMin(from, to), qMax(from, to), saveUndoRedo);
    SWT_moveChildAbstractionForTargetToInAll(child, mChildBoxes.count() - to - 1
                                                    + ca_mChildAnimators.count());
    if(saveUndoRedo) {
        addUndoRedo(new MoveChildInListUndoRedo(child, from, to, this) );
    }

    scheduleSoftUpdate();

    clearAllCache();
}

void BoxesGroup::moveChildBelow(BoundingBox *boxToMove,
                                BoundingBox *below) {
    const int &indexFrom = getChildBoxIndex(boxToMove);
    int indexTo = getChildBoxIndex(below);
    if(indexFrom > indexTo) {
        indexTo++;
    }
    moveChildInList(boxToMove,
                    indexFrom,
                    indexTo);
}

void BoxesGroup::moveChildAbove(BoundingBox *boxToMove,
                                BoundingBox *above) {
    const int &indexFrom = getChildBoxIndex(boxToMove);
    int indexTo = getChildBoxIndex(above);
    if(indexFrom < indexTo) {
        indexTo--;
    }
    moveChildInList(boxToMove,
                    indexFrom,
                    indexTo);
}

void BoxesGroup::updateAfterCombinedTransformationChanged() {
    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mChildBoxes) {
        child->updateCombinedTransform();
    }
}

void BoxesGroup::updateAfterCombinedTransformationChangedAfterFrameChagne() {
    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mChildBoxes) {
        child->updateCombinedTransformAfterFrameChange();
    }
}

#include "BoxesList/OptimalScrollArea/singlewidgetabstraction.h"
void BoxesGroup::SWT_addChildrenAbstractions(
        SingleWidgetAbstraction *abstraction,
        ScrollWidgetVisiblePart *visiblePartWidget) {
    BoundingBox::SWT_addChildrenAbstractions(abstraction,
                                             visiblePartWidget);

    Q_FOREACH(const QSharedPointer<BoundingBox> &child, mChildBoxes) {
        abstraction->addChildAbstraction(
                    child->SWT_getAbstractionForWidget(visiblePartWidget));
    }
}

bool BoxesGroup::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                     const bool &parentSatisfies,
                                     const bool &parentMainTarget) {
    const SWT_Rule &rule = rules.rule;
    if(rule == SWT_Selected) {
        return BoundingBox::SWT_shouldBeVisible(rules,
                                                parentSatisfies,
                                                parentMainTarget) &&
                !isCurrentGroup();
    }
    return BoundingBox::SWT_shouldBeVisible(rules,
                                            parentSatisfies,
                                            parentMainTarget);
}
