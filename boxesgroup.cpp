#include "boxesgroup.h"
#include <QApplication>

bool zLessThan(BoundingBox *box1, BoundingBox *box2)
{
    return box1->getZIndex() > box2->getZIndex();
}

BoxesGroup::BoxesGroup(FillStrokeSettingsWidget *fillStrokeSetting, BoundingBox *parent) :
    BoundingBox(parent, BoundingBoxType::TYPE_GROUP)
{
    mFillStrokeSettingsWidget = fillStrokeSetting;
}

BoxesGroup::BoxesGroup(FillStrokeSettingsWidget *fillStrokeSetting, MainWindow *parent) :
    BoundingBox(parent, BoundingBoxType::TYPE_CANVAS)
{
    mFillStrokeSettingsWidget = fillStrokeSetting;
}

bool BoxesGroup::pointInsidePath(QPointF absPos)
{
    foreachBoxInList(mChildren) {
        if(box->pointInsidePath(absPos)) {
            return true;
        }
    }
    return false;
}

QRectF BoxesGroup::getBoundingRect()
{
    QRectF rect;
    foreach(BoundingBox *box, mChildren) {
        rect = rect.united(box->getBoundingRect());
    }
    return rect;
}

void BoxesGroup::draw(QPainter *p)
{
    foreachBoxInList(mChildren) {
        box->draw(p);
    }
}

void BoxesGroup::drawBoundingRect(QPainter *p) {
    QPen pen = p->pen();
    if(mIsCurrentGroup) {
        p->setPen(QPen(QColor(255, 0, 0, 125), 1.f, Qt::DashLine));
    } else {
        p->setPen(QPen(QColor(0, 0, 0, 125), 1.f, Qt::DashLine));
    }
    p->setBrush(Qt::NoBrush);
    p->drawRect(getBoundingRect());
    p->setPen(pen);
}

void BoxesGroup::setIsCurrentGroup(bool bT)
{
    mIsCurrentGroup = bT;
    scheduleRepaint();
}

BoundingBox *BoxesGroup::getBoxAtFromAllAncestors(QPointF absPos)
{
    BoundingBox *boxAtPos = NULL;
    foreachBoxInListInverted(mChildren) {
        boxAtPos = box->getBoxAtFromAllAncestors(absPos);
        if(boxAtPos != NULL) {
            break;
        }
    }
    return boxAtPos;
}

void BoxesGroup::setFillSettings(PaintSettings fillSettings,
                                 bool saveUndoRedo)
{
    if(saveUndoRedo) {
        startNewUndoRedoSet();
    }
    foreachBoxInList(mChildren) {
        box->setFillSettings(fillSettings, saveUndoRedo);
    }
    if(saveUndoRedo) {
        finishUndoRedoSet();
    }
}

void BoxesGroup::setStrokeSettings(StrokeSettings strokeSettings,
                                   bool saveUndoRedo)
{
    if(saveUndoRedo) {
        startNewUndoRedoSet();
    }
    foreachBoxInList(mChildren) {
        box->setStrokeSettings(strokeSettings, saveUndoRedo);
    }
    if(saveUndoRedo) {
        finishUndoRedoSet();
    }
}

void BoxesGroup::setSelectedFillSettings(PaintSettings fillSettings, bool saveUndoRedo)
{
    if(saveUndoRedo) {
        startNewUndoRedoSet();
    }
    foreachBoxInList(mSelectedBoxes) {
        box->setFillSettings(fillSettings, saveUndoRedo);
    }
    if(saveUndoRedo) {
        finishUndoRedoSet();
    }
}

void BoxesGroup::setSelectedStrokeSettings(StrokeSettings strokeSettings, bool saveUndoRedo)
{
    if(saveUndoRedo) {
        startNewUndoRedoSet();
    }
    foreachBoxInList(mSelectedBoxes) {
        box->setStrokeSettings(strokeSettings, saveUndoRedo);
    }
    if(saveUndoRedo) {
        finishUndoRedoSet();
    }
}

void BoxesGroup::startStrokeTransform()
{
    foreachBoxInList(mChildren) {
        box->startStrokeTransform();
    }
}

void BoxesGroup::startFillTransform()
{
    foreachBoxInList(mChildren) {
        box->startFillTransform();
    }
}

void BoxesGroup::finishStrokeTransform()
{
    startNewUndoRedoSet();
    foreachBoxInList(mChildren) {
        box->finishStrokeTransform();
    }
    finishUndoRedoSet();
}

void BoxesGroup::finishFillTransform()
{
    startNewUndoRedoSet();
    foreachBoxInList(mChildren) {
        box->finishFillTransform();
    }
    finishUndoRedoSet();
}

void BoxesGroup::startSelectedStrokeTransform()
{
    foreachBoxInList(mSelectedBoxes) {
        box->startStrokeTransform();
    }
}

void BoxesGroup::startSelectedFillTransform()
{
    foreachBoxInList(mSelectedBoxes) {
        box->startFillTransform();
    }
}

void BoxesGroup::finishSelectedStrokeTransform()
{
    startNewUndoRedoSet();
    foreachBoxInList(mSelectedBoxes) {
        box->finishStrokeTransform();
    }
    finishUndoRedoSet();
}

void BoxesGroup::finishSelectedFillTransform()
{
    startNewUndoRedoSet();
    foreachBoxInList(mSelectedBoxes) {
        box->finishFillTransform();
    }
    finishUndoRedoSet();
}

void BoxesGroup::rotateSelectedBy(qreal rotBy, QPointF absOrigin,
                                  bool startTrans)
{
    if(startTrans) {
        foreachBoxInList(mSelectedBoxes) {
            box->startTransform();
            box->rotateBy(rotBy, absOrigin);
        }
    } else {
        foreachBoxInList(mSelectedBoxes) {
            box->rotateBy(rotBy, absOrigin);
        }
    }
}

QPointF BoxesGroup::getSelectedPivotPos()
{
    if(mSelectedBoxes.isEmpty()) return QPointF(0.f, 0.f);
    QPointF posSum = QPointF(0.f, 0.f);
    int count = mSelectedBoxes.length();
    foreachBoxInList(mSelectedBoxes) {
        posSum += box->getPivotAbsPos();
    }
    return posSum/count;
}

bool BoxesGroup::isSelectionEmpty()
{
    return mSelectedBoxes.isEmpty();
}

void BoxesGroup::setSelectedPivotAbsPos(QPointF absPos)
{
    if(mSelectedBoxes.count() == 1) {
        mSelectedBoxes.first()->setPivotAbsPos(absPos);
    }
}

void BoxesGroup::ungroup() {
    startNewUndoRedoSet();
    clearBoxesSelection();
    BoxesGroup *parentGroup = (BoxesGroup*) mParent;
    foreachBoxInListInverted(mChildren) {
        box->applyTransformation(mTransformMatrix);
        BoundingBox::removeChild(box);
        parentGroup->addChild(box);
    }
    mParent->removeChild(this);
    finishUndoRedoSet();
}

void BoxesGroup::ungroupSelected()
{
    foreachBoxInList(mSelectedBoxes) {
        if(box->isGroup()) {
            ((BoxesGroup*) box)->ungroup();
        }
    }
}

void BoxesGroup::drawSelected(QPainter *p, CanvasMode currentCanvasMode)
{
    foreachBoxInList(mSelectedBoxes) {
        box->drawSelected(p, currentCanvasMode);
    }
    drawBoundingRect(p);
}

void BoxesGroup::removeSelectedPointsAndClearList()
{
    startNewUndoRedoSet();

    foreach(MovablePoint *point, mSelectedPoints) {
        point->deselect();
        point->remove();
    }
    mSelectedPoints.clear(); schedulePivotUpdate();

    finishUndoRedoSet();
}

void BoxesGroup::removeSelectedBoxesAndClearList()
{
    startNewUndoRedoSet();

    foreachBoxInList(mSelectedBoxes) {
        BoundingBox::removeChild(box);
        box->deselect();
    }
    mSelectedBoxes.clear(); schedulePivotUpdate();

    finishUndoRedoSet();
}

void BoxesGroup::removeChildFromList(int id, bool saveUndoRedo)
{
    BoundingBox *box = mChildren.at(id);
    if(box->isSelected()) {
        removeBoxFromSelection(box);
    }
    BoundingBox::removeChildFromList(id, saveUndoRedo);
}

void BoxesGroup::clearPointsSelection()
{
    foreach(MovablePoint *point, mSelectedPoints) {
        point->deselect();
    }
    mSelectedPoints.clear();
}

void BoxesGroup::setCurrentFillStrokeSettingsFromBox(BoundingBox *box) {
    mFillStrokeSettingsWidget->setCurrentSettings(box->getFillSettings(),
                                                  box->getStrokeSettings());
}

void BoxesGroup::addBoxToSelection(BoundingBox *box) {
    if(box->isSelected()) {
        return;
    }
    box->select();
    mSelectedBoxes.append(box); schedulePivotUpdate();
    qSort(mSelectedBoxes.begin(), mSelectedBoxes.end(), zLessThan);
    setCurrentFillStrokeSettingsFromBox(box);
}

void BoxesGroup::addPointToSelection(MovablePoint *point)
{
    if(point->isSelected()) {
        return;
    }
    point->select();
    mSelectedPoints.append(point);
}

void BoxesGroup::removeBoxFromSelection(BoundingBox *box) {
    box->deselect();
    mSelectedBoxes.removeOne(box); schedulePivotUpdate();
}

void BoxesGroup::removePointFromSelection(MovablePoint *point) {
    point->deselect();
    mSelectedPoints.removeOne(point);
}

void BoxesGroup::clearBoxesSelection()
{
    foreachBoxInList(mSelectedBoxes) {
        box->deselect();
    }
    mSelectedBoxes.clear(); schedulePivotUpdate();
}

void BoxesGroup::bringSelectedBoxesToFront() {
    foreachBoxInList(mSelectedBoxes) {
        box->bringToFront();
    }
}

void BoxesGroup::bringSelectedBoxesToEnd() {
    foreachBoxInList(mSelectedBoxes) {
        box->bringToEnd();
    }
}

void BoxesGroup::moveSelectedBoxesDown() {
    foreachBoxInList(mSelectedBoxes) {
        box->moveDown();
    }
}

void BoxesGroup::moveSelectedBoxesUp() {
    foreachBoxInList(mSelectedBoxes) {
        box->moveUp();
    }
}

void BoxesGroup::deselectAllBoxes() {
    foreachBoxInList(mSelectedBoxes) {
        removeBoxFromSelection(box);
    }
}

void BoxesGroup::selectAllBoxes() {
    foreachBoxInList(mChildren) {
        if(box->isSelected()) continue;
        addBoxToSelection(box);
    }
}

void BoxesGroup::connectPoints()
{
    QList<PathPoint*> selectedPathPoints;
    foreach(MovablePoint *point, mSelectedPoints) {
        if(point->isPathPoint()) {
            selectedPathPoints.append( (PathPoint*) point);
        }
    }
    if(selectedPathPoints.count() == 2) {
        startNewUndoRedoSet();

        PathPoint *firstPoint = selectedPathPoints.first();
        PathPoint *secondPoint = selectedPathPoints.last();
        if(firstPoint->isEndPoint() && secondPoint->isEndPoint()) {
            firstPoint->connectToPoint(secondPoint);
        }
        finishUndoRedoSet();
        scheduleRepaint();
    }

    callUpdateSchedulers();
}

void BoxesGroup::disconnectPoints()
{
    QList<PathPoint*> selectedPathPoints;
    foreach(MovablePoint *point, mSelectedPoints) {
        if(point->isPathPoint()) {
            selectedPathPoints.append( (PathPoint*) point);
        }
    }
    if(selectedPathPoints.count() == 2) {
        startNewUndoRedoSet();

        PathPoint *firstPoint = selectedPathPoints.first();
        PathPoint *secondPoint = selectedPathPoints.last();
        firstPoint->disconnectFromPoint(secondPoint);

        finishUndoRedoSet();
        scheduleRepaint();
    }

    callUpdateSchedulers();
}

void BoxesGroup::mergePoints()
{
    QList<PathPoint*> selectedPathPoints;
    foreach(MovablePoint *point, mSelectedPoints) {
        if(point->isPathPoint()) {
            selectedPathPoints.append( (PathPoint*) point);
        }
    }
    if(selectedPathPoints.count() == 2) {
        startNewUndoRedoSet();

        PathPoint *firstPoint = selectedPathPoints.first();
        PathPoint *secondPoint = selectedPathPoints.last();
        QPointF sumPos = firstPoint->getAbsolutePos() + secondPoint->getAbsolutePos();
        firstPoint->remove();
        secondPoint->moveToAbs(sumPos/2);

        finishUndoRedoSet();
        scheduleRepaint();
    }

    callUpdateSchedulers();
}

void BoxesGroup::setPointCtrlsMode(CtrlsMode mode) {
    startNewUndoRedoSet();
    foreach(MovablePoint *point, mSelectedPoints) {
        if(point->isPathPoint()) {
            ( (PathPoint*)point)->setCtrlsMode(mode);
        }
    }
    finishUndoRedoSet();
    scheduleRepaint();

    callUpdateSchedulers();
}

BoundingBox *BoxesGroup::getBoxAt(QPointF absPos) {
    BoundingBox *boxAtPos = NULL;

    foreachBoxInListInverted(mChildren) {
        if(box->pointInsidePath(absPos)) {
            boxAtPos = box;
            break;
        }
    }
    return boxAtPos;
}

MovablePoint *BoxesGroup::getPointAt(QPointF absPos, CanvasMode currentMode) {
    MovablePoint *pointAtPos = NULL;
    foreachBoxInList(mSelectedBoxes) {
        pointAtPos = box->getPointAt(absPos, currentMode);
        if(pointAtPos != NULL) {
            break;
        }
    }
    return pointAtPos;
}

void BoxesGroup::finishSelectedPointsTransform()
{
    startNewUndoRedoSet();
    foreach(MovablePoint *point, mSelectedPoints) {
        point->finishTransform();
    }
    finishUndoRedoSet();
}

void BoxesGroup::addContainedBoxesToSelection(QRectF rect)
{
    foreachBoxInList(mChildren) {
        if(box->isContainedIn(rect) ) {
            addBoxToSelection(box);
        }
    }
}

void BoxesGroup::finishSelectedBoxesTransform()
{
    startNewUndoRedoSet();
    foreachBoxInList(mSelectedBoxes) {
        box->finishTransform();
    }
    finishUndoRedoSet();
}

void BoxesGroup::moveSelectedPointsBy(QPointF by, bool startTransform)
{
    if(startTransform) {
        foreach(MovablePoint *point, mSelectedPoints) {
            point->startTransform();
            point->moveBy(by);
        }
    } else {
        foreach(MovablePoint *point, mSelectedPoints) {
            point->moveBy(by);
        }
    }
}

void BoxesGroup::moveSelectedBoxesBy(QPointF by, bool startTransform)
{
    if(startTransform) {
        foreachBoxInList(mSelectedBoxes) {
            box->startTransform();
            box->moveBy(by);
        }
    } else {
        foreachBoxInList(mSelectedBoxes) {
            box->moveBy(by);
        }
    }
}

void BoxesGroup::selectAndAddContainedPointsToSelection(QRectF absRect)
{
    foreachBoxInList(mSelectedBoxes) {
        box->selectAndAddContainedPointsToList(absRect, &mSelectedPoints);
    }
}

void BoxesGroup::updatePivotPosition() {
    if(!mPivotChanged) {
        if(mChildren.isEmpty()) return;
        QPointF posSum = QPointF(0.f, 0.f);
        int count = mChildren.length();
        foreachBoxInList(mChildren) {
            posSum += box->getPivotAbsPos();
        }
        setPivotAbsPos(posSum/count, false, false);
    }
}

void BoxesGroup::select()
{
    BoundingBox::select();
    updatePivotPosition();
}

bool BoxesGroup::isShiftPressed() {
    return QApplication::keyboardModifiers() & Qt::ShiftModifier;
}

bool BoxesGroup::isCtrlPressed() {
    return (QApplication::keyboardModifiers() & Qt::ControlModifier);
}

BoxesGroup* BoxesGroup::groupSelectedBoxes() {
    if(mSelectedBoxes.count() < 2) {
        return NULL;
    }
    startNewUndoRedoSet();
    BoxesGroup *newGroup = new BoxesGroup(mFillStrokeSettingsWidget, this);
    foreachBoxInListInverted(mSelectedBoxes) {
        BoundingBox::removeChild(box);
        box->deselect();
        newGroup->addChild(box);
    }
    newGroup->selectAllBoxes();
    mSelectedBoxes.clear(); schedulePivotUpdate();
    finishUndoRedoSet();
    return newGroup;
}
