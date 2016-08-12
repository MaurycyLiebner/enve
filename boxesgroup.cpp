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

void BoxesGroup::setFillStrokeSettings(PaintSettings fillSettings,
                                       StrokeSettings strokeSettings)
{
    foreachBoxInList(mChildren) {
        box->setFillStrokeSettings(fillSettings, strokeSettings);
    }
}

void BoxesGroup::setFillSettings(PaintSettings fillSettings)
{
    foreachBoxInList(mChildren) {
        box->setFillSettings(fillSettings);
    }
}

void BoxesGroup::setStrokeSettings(StrokeSettings strokeSettings)
{
    foreachBoxInList(mChildren) {
        box->setStrokeSettings(strokeSettings);
    }
}

void BoxesGroup::setSelectedFillSettings(PaintSettings fillSettings)
{
    foreachBoxInList(mSelectedBoxes) {
        box->setFillSettings(fillSettings);
    }
}

void BoxesGroup::setSelectedStrokeSettings(StrokeSettings strokeSettings)
{
    foreachBoxInList(mSelectedBoxes) {
        box->setStrokeSettings(strokeSettings);
    }
}

void BoxesGroup::setSelectedFillStrokeSettings(PaintSettings fillSettings,
                                               StrokeSettings strokeSettings)
{
    foreachBoxInList(mSelectedBoxes) {
        box->setFillStrokeSettings(fillSettings, strokeSettings);
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
        point->remove();
    }
    mSelectedPoints.clear();

    finishUndoRedoSet();
}

void BoxesGroup::removeSelectedBoxesAndClearList()
{
    startNewUndoRedoSet();

    foreachBoxInList(mSelectedBoxes) {
        BoundingBox::removeChild(box);
        box->deselect();
    }
    mSelectedBoxes.clear();

    finishUndoRedoSet();
}

void BoxesGroup::removeChild(BoundingBox *box)
{
    if(box->isSelected()) {
        removeBoxFromSelection(box);
    }
    BoundingBox::removeChild(box);
}

void BoxesGroup::clearPointsSelection()
{
    foreach(MovablePoint *point, mSelectedPoints) {
        point->deselect();
    }
    mSelectedPoints.clear();
}

void BoxesGroup::addBoxToSelection(BoundingBox *box) {
    if(box->isSelected()) {
        return;
    }
    box->select();
    mSelectedBoxes.append(box);
    qSort(mSelectedBoxes.begin(), mSelectedBoxes.end(), zLessThan);
    mFillStrokeSettingsWidget->setCurrentSettings(box->getFillSettings(),
                                            box->getStrokeSettings());
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
    mSelectedBoxes.removeOne(box);
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
    mSelectedBoxes.clear();
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
    mSelectedBoxes.clear();
    finishUndoRedoSet();
    return newGroup;
}
