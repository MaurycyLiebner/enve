#include "Boxes/boxesgroup.h"
#include "undoredo.h"
#include <QApplication>
#include "mainwindow.h"
#include "ctrlpoint.h"
#include "Boxes/circle.h"
#include "Boxes/rectangle.h"
#include "BoxesList/boxscrollwidget.h"

bool BoxesGroup::mCtrlsAlwaysVisible = false;

//bool zMoreThan(BoundingBox *box1, BoundingBox *box2)
//{
//    return box1->getZIndex() < box2->getZIndex();
//}

BoxesGroup::BoxesGroup(BoxesGroup *parent) :
    BoundingBox(parent, BoundingBoxType::TYPE_GROUP)
{
    setName("Group");
    mFillStrokeSettingsWidget = getMainWindow()->getFillStrokeSettings();
}

void BoxesGroup::updateAllBoxes() {
    foreach(BoundingBox *child, mChildBoxes) {
        child->updateAllBoxes();
    }
    scheduleUpdate();
}

void BoxesGroup::clearCache() {
    BoundingBox::clearCache();
    foreach(BoundingBox *child, mChildBoxes) {
        child->clearCache();
    }
}

void BoxesGroup::updateCombinedTransformTmp() {
    mCombinedTransformMatrix = mRelativeTransformMatrix*
                               mParent->getCombinedTransform();
    foreach(BoundingBox *child, mChildBoxes) {
        child->updateCombinedTransformTmp();
    }
    if(!mIsDescendantCurrentGroup) {
        mOldRenderContainer->updatePaintTransformGivenNewCombinedTransform(
                                                mCombinedTransformMatrix);
    }
}

BoxesGroup::BoxesGroup(FillStrokeSettingsWidget *fillStrokeSetting) :
    BoundingBox(BoundingBoxType::TYPE_CANVAS)
{   
    mFillStrokeSettingsWidget = fillStrokeSetting;
}

BoxesGroup::~BoxesGroup() {
    foreach(BoundingBox *box, mChildBoxes) {
        box->decNumberPointers();
    }
}

void BoxesGroup::loadFromSql(int boundingBoxId) {
    BoundingBox::loadFromSql(boundingBoxId);
    loadChildrenFromSql(boundingBoxId, false);
}

#include "linkbox.h"
BoundingBox *BoxesGroup::createLink(BoxesGroup *parent) {
    InternalLinkBoxesGroup *linkGroup =
                        new InternalLinkBoxesGroup(this, parent);
    foreach(BoundingBox *box, mChildBoxes) {
        box->createSameTransformationLink(linkGroup);
    }
    return linkGroup;
}

BoundingBox *BoxesGroup::createSameTransformationLink(BoxesGroup *parent) {
    SameTransformInternalLinkBoxesGroup *linkGroup =
                        new SameTransformInternalLinkBoxesGroup(this, parent);
    foreach(BoundingBox *box, mChildBoxes) {
        box->createSameTransformationLink(linkGroup);
    }
    return linkGroup;
}


BoxesGroup *BoxesGroup::loadChildrenFromSql(int thisBoundingBoxId,
                                            bool loadInBox) {
    QString thisBoundingBoxIdStr = QString::number(thisBoundingBoxId);
    if(loadInBox) {
        BoxesGroup *newGroup = new BoxesGroup(this);
        newGroup->loadChildrenFromSql(thisBoundingBoxId, false);
        newGroup->centerPivotPosition();
        return newGroup;
    }
    QSqlQuery query;
    QString queryStr;
    queryStr = "SELECT id, boxtype FROM boundingbox WHERE parentboundingboxid = " + thisBoundingBoxIdStr;
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
                group->loadFromSql(query.value(idId).toInt());
            } else if(static_cast<BoundingBoxType>(
                          query.value(idBoxType).toInt()) == TYPE_CIRCLE ) {
                Circle *circle = new Circle(this);
                circle->loadFromSql(query.value(idId).toInt());
            } else if(static_cast<BoundingBoxType>(
                          query.value(idBoxType).toInt()) == TYPE_TEXT ) {
                TextBox *textBox = new TextBox(this);
                textBox->loadFromSql(query.value(idId).toInt());
            } else if(static_cast<BoundingBoxType>(
                          query.value(idBoxType).toInt()) == TYPE_RECTANGLE ) {
                Rectangle *rectangle = new Rectangle(this);
                rectangle->loadFromSql(query.value(idId).toInt());
            }
        }
    } else {
        qDebug() << "Could not load children for boxesgroup with id " + thisBoundingBoxIdStr;
    }
    return this;
}

int BoxesGroup::saveToSql(QSqlQuery *query, int parentId)
{
    int boundingBoxId = BoundingBox::saveToSql(query, parentId);
    query->exec(QString("INSERT INTO boxesgroup (boundingboxid) VALUES (%1)").
                arg(boundingBoxId));
    foreach(BoundingBox *box, mChildBoxes) {
        box->saveToSql(query, boundingBoxId);
    }
    return boundingBoxId;
}

bool BoxesGroup::relPointInsidePath(QPointF relPos) {
    if(mRelBoundingRect.contains(relPos)) {
        QPointF absPos = mapRelativeToAbsolute(relPos);
        foreach(BoundingBox *box, mChildBoxes) {
            if(box->absPointInsidePath(absPos)) {
                return true;
            }
        }
    }
    return false;
}

void BoxesGroup::updateAfterFrameChanged(int currentFrame) {
    BoundingBox::updateAfterFrameChanged(currentFrame);
    BoundingBoxRenderContainer *cont =
                    getRenderContainerAtFrame(currentFrame);
    if(cont != NULL) {
        mOldRenderContainer = cont;
        mOldRenderContainer->updatePaintTransformGivenNewCombinedTransform(
                                        mCombinedTransformMatrix);
        if(getParentCanvas()->isPreviewing()) return;
    }
    foreach(BoundingBox *box, mChildBoxes) {
        box->updateAfterFrameChanged(currentFrame);
    }
}

void BoxesGroup::setFillGradient(Gradient *gradient, bool finish) {
    foreach(BoundingBox *box, mChildBoxes) {
        box->setFillGradient(gradient, finish);
    }
}

void BoxesGroup::setStrokeGradient(Gradient *gradient, bool finish)
{
    foreach(BoundingBox *box, mChildBoxes) {
        box->setStrokeGradient(gradient, finish);
    }
}

void BoxesGroup::setFillFlatColor(Color color, bool finish)
{
    foreach(BoundingBox *box, mChildBoxes) {
        box->setFillFlatColor(color, finish);
    }
}

void BoxesGroup::setStrokeFlatColor(Color color, bool finish)
{
    foreach(BoundingBox *box, mChildBoxes) {
        box->setStrokeFlatColor(color, finish);
    }
}

void BoxesGroup::setFillPaintType(PaintType paintType,
                                  Color color, Gradient *gradient)
{
    foreach(BoundingBox *box, mChildBoxes) {
        box->setFillPaintType(paintType, color, gradient);
    }
}

void BoxesGroup::setStrokePaintType(PaintType paintType,
                                    Color color, Gradient *gradient)
{
    foreach(BoundingBox *box, mChildBoxes) {
        box->setStrokePaintType(paintType, color, gradient);
    }
}

void BoxesGroup::setStrokeCapStyle(Qt::PenCapStyle capStyle)
{
    foreach(BoundingBox *box, mChildBoxes) {
        box->setStrokeCapStyle(capStyle);
    }
}

void BoxesGroup::setStrokeJoinStyle(Qt::PenJoinStyle joinStyle)
{
    foreach(BoundingBox *box, mChildBoxes) {
        box->setStrokeJoinStyle(joinStyle);
    }
}

void BoxesGroup::setStrokeWidth(qreal strokeWidth, bool finish)
{
    foreach(BoundingBox *box, mChildBoxes) {
        box->setStrokeWidth(strokeWidth, finish);
    }
}

void BoxesGroup::startSelectedStrokeWidthTransform()
{
    foreach(BoundingBox *box, mChildBoxes) {
        box->startSelectedStrokeWidthTransform();
    }
}

void BoxesGroup::startSelectedStrokeColorTransform()
{
    foreach(BoundingBox *box, mChildBoxes) {
        box->startSelectedStrokeColorTransform();
    }
}

void BoxesGroup::startSelectedFillColorTransform()
{
    foreach(BoundingBox *box, mChildBoxes) {
        box->startSelectedFillColorTransform();
    }
}

void BoxesGroup::makeDuplicate(BoundingBox *targetBox) {
    BoundingBox::makeDuplicate(targetBox);
    foreach(BoundingBox *child, mChildBoxes) {
        child->createDuplicate((BoxesGroup*)targetBox);
    }
}

BoundingBox *BoxesGroup::createNewDuplicate(BoxesGroup *parent) {
    return new BoxesGroup(parent);
}

void BoxesGroup::updateRelBoundingRect() {
    QPainterPath boundingPaths = QPainterPath();
    foreach(BoundingBox *child, mChildBoxes) {
        boundingPaths.addPath(
                    child->getRelativeTransform().
                    map(child->getRelBoundingRectPath()));
    }
    mRelBoundingRect = boundingPaths.boundingRect();

    BoundingBox::updateRelBoundingRect();
}

void BoxesGroup::updateEffectsMargin() {
    qreal childrenMargin = 0.;
    foreach(BoundingBox *child, mChildBoxes) {
        childrenMargin = qMax(child->getEffectsMargin(),
                              childrenMargin);
    }
    BoundingBox::updateEffectsMargin();
    mEffectsMargin += childrenMargin;
}

void BoxesGroup::drawUpdatePixmap(QPainter *p) {
    if(shouldPaintOnImage()) {
        BoundingBox::drawUpdatePixmap(p);
    } else {
        foreach(BoundingBox *box, mChildBoxes) {
            box->drawUpdatePixmap(p);
        }
    }
}

void BoxesGroup::addChildAwaitingUpdate(BoundingBox *child) {
    mChildrenAwaitingUpdate.removeOne(child);
    mChildrenAwaitingUpdate << child;

    if(mParent == NULL) return;
    scheduleUpdate();
}

void BoxesGroup::beforeUpdate() {
    foreach(BoundingBox *child, mChildrenAwaitingUpdate) {
        child->beforeUpdate();
        mUpdateChildrenAwaitingUpdate.append(child);
    }

    mChildrenAwaitingUpdate.clear();
    BoundingBox::beforeUpdate();
}

void BoxesGroup::processUpdate() {
    foreach(BoundingBox *child, mUpdateChildrenAwaitingUpdate) {
        child->processUpdate();
    }
    BoundingBox::processUpdate();
}

void BoxesGroup::afterUpdate() {
    foreach(BoundingBox *child, mUpdateChildrenAwaitingUpdate) {
        child->afterUpdate();
    }
    mUpdateChildrenAwaitingUpdate.clear();
    BoundingBox::afterUpdate();

}

void BoxesGroup::draw(QPainter *p) {
    if(shouldUpdateAndDraw()) {
        p->save();
        p->setTransform(QTransform(
                            mUpdateTransform.inverted()),
                            true);
        foreach(BoundingBox *box, mChildBoxes) {
            //box->draw(p);
            box->drawUpdatePixmap(p);
        }

        p->restore();
    }
}

void BoxesGroup::drawPixmap(QPainter *p) {
    if(shouldPaintOnImage()) {
        BoundingBox::drawPixmap(p);
    } else {
        foreach(BoundingBox *box, mChildBoxes) {
            //box->draw(p);
            box->drawPixmap(p);
        }
    }
}

void BoxesGroup::drawBoundingRect(QPainter *p) {
    p->save();

    QPen pen;
    if(mIsCurrentGroup) {
        pen = QPen(QColor(255, 0, 0, 125), 1.f, Qt::DashLine);
    } else {
        pen = QPen(QColor(0, 0, 0, 125), 1.f, Qt::DashLine);
    }
    pen.setCosmetic(true);
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);

    p->setTransform(QTransform(mCombinedTransformMatrix), true);
    p->drawPath(mRelBoundingRectPath);

    p->restore();
}

void BoxesGroup::setIsCurrentGroup(bool bT) {
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
    return mEffectsAnimators.hasEffects() &&
           !mIsDescendantCurrentGroup;
}

void BoxesGroup::setDescendantCurrentGroup(const bool &bT) {
    mIsDescendantCurrentGroup = bT;
    if(!bT) {
        scheduleUpdate();
    }
    if(mParent == NULL) return;
    mParent->setDescendantCurrentGroup(bT);
}

BoundingBox *BoxesGroup::getPathAtFromAllAncestors(QPointF absPos) {
    BoundingBox *boxAtPos = NULL;
    //foreachBoxInListInverted(mChildren) {
    BoundingBox *box;
    foreachInverted(box, mChildBoxes) {
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
    BoxesGroup *parentGroup = (BoxesGroup*) mParent;
    foreach(BoundingBox *box, mChildBoxes) {
        box->applyTransformation(&mTransformAnimator);
        removeChild(box);
        parentGroup->addChild(box);
    }
    mParent->removeChild(this);
}

PaintSettings *BoxesGroup::getFillSettings() {
    return mChildBoxes.first()->getFillSettings();
}

StrokeSettings *BoxesGroup::getStrokeSettings() {
    return mChildBoxes.first()->getStrokeSettings();
}

bool BoxesGroup::getCtrlsAlwaysVisible()
{
    return BoxesGroup::mCtrlsAlwaysVisible;
}

void BoxesGroup::setCtrlsAlwaysVisible(bool bT) {
    BoxesGroup::mCtrlsAlwaysVisible = bT;
    MainWindow::getInstance()->getCanvasWidget()->getCurrentCanvas()->updateSelectedPointsAfterCtrlsVisiblityChanged();
}

void BoxesGroup::setCurrentFillStrokeSettingsFromBox(BoundingBox *box) {
    mFillStrokeSettingsWidget->setCurrentSettings(box->getFillSettings(),
                                                  box->getStrokeSettings());
}

void BoxesGroup::applyCurrentTransformation() {
    QPointF absPivot = getPivotAbsPos();
    qreal rotation = mTransformAnimator.rot();
    qreal scaleX = mTransformAnimator.xScale();
    qreal scaleY = mTransformAnimator.yScale();
    foreach(BoundingBox *box, mChildBoxes) {
        box->saveTransformPivotAbsPos(absPivot);
        box->startTransform();
        box->rotateRelativeToSavedPivot(rotation);
        box->finishTransform();
        box->startTransform();
        box->scaleRelativeToSavedPivot(scaleX, scaleY);
        box->finishTransform();
    }

    mTransformAnimator.resetRotation(true);
    mTransformAnimator.resetScale(true);
}

void BoxesGroup::selectAllBoxesFromBoxesGroup() {
    foreach(BoundingBox *box, mChildBoxes) {
        if(box->isSelected()) continue;
        getParentCanvas()->addBoxToSelection(box);
    }
}

void BoxesGroup::deselectAllBoxesFromBoxesGroup() {
    foreach(BoundingBox *box, mChildBoxes) {
        if(box->isSelected()) {
            getParentCanvas()->removeBoxFromSelection(box);
        }
    }
}

BoundingBox *BoxesGroup::getBoxAt(QPointF absPos) {
    BoundingBox *boxAtPos = NULL;

    BoundingBox *box;
    foreachInverted(box, mChildBoxes) {
        if(box->isVisibleAndUnlocked()) {
            if(box->absPointInsidePath(absPos)) {
                boxAtPos = box;
                break;
            }
        }
    }
    return boxAtPos;
}

void BoxesGroup::addContainedBoxesToSelection(QRectF rect)
{
    foreach(BoundingBox *box, mChildBoxes) {
        if(box->isVisibleAndUnlocked()) {
            if(box->isContainedIn(rect) ) {
                getParentCanvas()->addBoxToSelection(box);
            }
        }
    }
}

void BoxesGroup::addChild(BoundingBox *child) {
    //child->setParent(this);
    addChildToListAt(mChildBoxes.count(), child);
}

void BoxesGroup::addChildToListAt(int index,
                                  BoundingBox *child,
                                  bool saveUndoRedo) {
    child->setParent(this);
    child->setNoCache(mType != TYPE_CANVAS);
    mChildBoxes.insert(index, child);
    updateRelBoundingRect();
    updateChildrenId(index, saveUndoRedo);
    if(saveUndoRedo) {
        addUndoRedo(new AddChildToListUndoRedo(this, index, child));
    }
    child->incNumberPointers();

    scheduleEffectsMarginUpdate();
    if(!mPivotChanged) scheduleCenterPivot();

    //SWT_addChildAbstractionForTargetToAll(child);
    SWT_addChildAbstractionForTargetToAllAt(child,
                                            mActiveAnimators.count());
}

void BoxesGroup::updateChildrenId(int firstId, bool saveUndoRedo) {
    updateChildrenId(firstId, mChildBoxes.length() - 1, saveUndoRedo);
}

void BoxesGroup::updateChildrenId(int firstId, int lastId, bool saveUndoRedo) {
    for(int i = firstId; i <= lastId; i++) {
        mChildBoxes.at(i)->setZListIndex(i, saveUndoRedo);
    }
}

void BoxesGroup::removeChildFromList(int id, bool saveUndoRedo) {
    BoundingBox *box = mChildBoxes.at(id);
    box->clearCache();
    if(box->isSelected()) {
        box->removeFromSelection();
    }
    if(saveUndoRedo) {
        addUndoRedo(new RemoveChildFromListUndoRedo(this, id,
                                                    mChildBoxes.at(id)) );
    }
    mChildBoxes.removeAt(id);
    updateRelBoundingRect();
    if(box->isGroup()) {
        BoxesGroup *group = (BoxesGroup*) box;
        if(group->isCurrentGroup()) {
            mMainWindow->getCanvasWidget()->getCurrentCanvas()->
                    setCurrentBoxesGroup(group->getParent());
        }
    }
    updateChildrenId(id, saveUndoRedo);

    box->decNumberPointers();

    scheduleEffectsMarginUpdate();

    SWT_removeChildAbstractionForTargetFromAll(box);
}

void BoxesGroup::removeChild(BoundingBox *child)
{
    int index = mChildBoxes.indexOf(child);
    if(index < 0) {
        return;
    }
    removeChildFromList(index);
    if(mChildBoxes.isEmpty() &&
       mParent != NULL) {
        mParent->removeChild(this);
    }
    //child->setParent(NULL);
    if(!mPivotChanged) centerPivotPosition();
}


void BoxesGroup::increaseChildZInList(BoundingBox *child)
{
    int index = mChildBoxes.indexOf(child);
    if(index == mChildBoxes.count() - 1) {
        return;
    }
    moveChildInList(child, index, index + 1);
}

void BoxesGroup::decreaseChildZInList(BoundingBox *child)
{
    int index = mChildBoxes.indexOf(child);
    if(index == 0) {
        return;
    }
    moveChildInList(child, index, index - 1);
}

void BoxesGroup::bringChildToEndList(BoundingBox *child)
{
    int index = mChildBoxes.indexOf(child);
    if(index == mChildBoxes.count() - 1) {
        return;
    }
    moveChildInList(child, index, mChildBoxes.length() - 1);
}

void BoxesGroup::bringChildToFrontList(BoundingBox *child)
{
    int index = mChildBoxes.indexOf(child);
    if(index == 0) {
        return;
    }
    moveChildInList(child, index, 0);
}

void BoxesGroup::moveChildInList(BoundingBox *child,
                                 int from, int to,
                                 bool saveUndoRedo) {
    mChildBoxes.move(from, to);
    updateChildrenId(qMin(from, to), qMax(from, to), saveUndoRedo);
    SWT_moveChildAbstractionForTargetToInAll(child, mChildBoxes.count() - to - 1
                                                    + mActiveAnimators.count());
    if(saveUndoRedo) {
        addUndoRedo(new MoveChildInListUndoRedo(child, from, to, this) );
    }

    scheduleUpdate();
}

void BoxesGroup::moveChildBelow(BoundingBox *boxToMove,
                                BoundingBox *below) {
    int indexFrom = mChildBoxes.indexOf(boxToMove);
    int indexTo = mChildBoxes.indexOf(below);
    if(indexFrom > indexTo) {
        indexTo++;
    }
    moveChildInList(boxToMove,
                    indexFrom,
                    indexTo);
}

void BoxesGroup::moveChildAbove(BoundingBox *boxToMove,
                                BoundingBox *above) {
    int indexFrom = mChildBoxes.indexOf(boxToMove);
    int indexTo = mChildBoxes.indexOf(above);
    if(indexFrom < indexTo) {
        indexTo--;
    }
    moveChildInList(boxToMove,
                    indexFrom,
                    indexTo);
}

void BoxesGroup::updateAfterCombinedTransformationChanged() {
    foreach(BoundingBox *child, mChildBoxes) {
        child->updateCombinedTransform();
    }
}

#include "BoxesList/OptimalScrollArea/singlewidgetabstraction.h"
void BoxesGroup::SWT_addChildrenAbstractions(
        SingleWidgetAbstraction *abstraction,
        ScrollWidgetVisiblePart *visiblePartWidget) {
    BoundingBox::SWT_addChildrenAbstractions(abstraction,
                                             visiblePartWidget);

    foreach(BoundingBox *child, mChildBoxes) {
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
