#include "boxesgroup.h"
#include "undoredo.h"
#include <QApplication>
#include "mainwindow.h"
#include "ctrlpoint.h"

bool zLessThan(BoundingBox *box1, BoundingBox *box2)
{
    return box1->getZIndex() > box2->getZIndex();
}

BoxesGroup::BoxesGroup(FillStrokeSettingsWidget *fillStrokeSetting, BoxesGroup *parent) :
    BoundingBox(parent, BoundingBoxType::TYPE_GROUP)
{
    mFillStrokeSettingsWidget = fillStrokeSetting;
}

BoxesGroup::BoxesGroup(int boundingBoxId,
                       FillStrokeSettingsWidget *fillStrokeSetting,
                       BoxesGroup *parent) : BoundingBox(boundingBoxId,
                                                          parent, TYPE_GROUP) {
    mFillStrokeSettingsWidget = fillStrokeSetting;
    loadChildrenFromSql(QString::number(boundingBoxId), false);
}

PathPoint *BoxesGroup::createNewPointOnLineNearSelected(QPointF absPos) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        PathPoint *point = box->createNewPointOnLineNear(absPos);
        if(point != NULL) {
            return point;
        }
    }
    return NULL;
}

void BoxesGroup::setDisplayedFillStrokeSettingsFromLastSelected() {
    if(mSelectedBoxes.isEmpty()) return;
    setCurrentFillStrokeSettingsFromBox(mSelectedBoxes.last() );
}

BoxesGroup::BoxesGroup(FillStrokeSettingsWidget *fillStrokeSetting) :
    BoundingBox(BoundingBoxType::TYPE_CANVAS)
{
    mFillStrokeSettingsWidget = fillStrokeSetting;
}

bool BoxesGroup::pointInsidePath(QPointF absPos)
{
    foreach(BoundingBox *box, mChildren) {
        if(box->pointInsidePath(absPos)) {
            return true;
        }
    }
    return false;
}

void BoxesGroup::updateAfterFrameChanged(int currentFrame)
{
    BoundingBox::updateAfterFrameChanged(currentFrame);
    foreach(BoundingBox *box, mChildren) {
        box->updateAfterFrameChanged(currentFrame);
    }
}

void BoxesGroup::setChildrenRenderCombinedTransform() {
    foreach(BoundingBox *box, mChildren) {
        box->setRenderCombinedTransform();
    }
}

void BoxesGroup::setRenderCombinedTransform() {
    BoundingBox::setRenderCombinedTransform();
    setChildrenRenderCombinedTransform();
}

void BoxesGroup::clearPointsSelectionOrDeselect() {
    if(mSelectedPoints.isEmpty() ) {
        deselectAllBoxes();
    } else {
        clearPointsSelection();
    }
}

void BoxesGroup::setSelectedFillGradient(Gradient *gradient, bool finish) {
    if(finish) {
        startNewUndoRedoSet();
    }
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setFillGradient(gradient, finish);
    }
    if(finish) {
        finishUndoRedoSet();
    }
}

void BoxesGroup::setSelectedStrokeGradient(Gradient *gradient, bool finish) {
    if(finish) {
        startNewUndoRedoSet();
    }
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setStrokeGradient(gradient, finish);
    }
    if(finish) {
        finishUndoRedoSet();
    }
}

void BoxesGroup::setSelectedFillFlatColor(Color color, bool finish) {
    if(finish) {
        startNewUndoRedoSet();
    }
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setFillFlatColor(color, finish);
    }
    if(finish) {
        finishUndoRedoSet();
    }
}

void BoxesGroup::setSelectedStrokeFlatColor(Color color, bool finish) {
    if(finish) {
        startNewUndoRedoSet();
    }
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setStrokeFlatColor(color, finish);
    }
    if(finish) {
        finishUndoRedoSet();
    }
}

void BoxesGroup::setSelectedFillPaintType(PaintType paintType, Color color, Gradient *gradient) {
    startNewUndoRedoSet();
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setFillPaintType(paintType, color, gradient);
    }
    finishUndoRedoSet();
}

void BoxesGroup::setSelectedStrokePaintType(PaintType paintType, Color color, Gradient *gradient) {
    startNewUndoRedoSet();
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setStrokePaintType(paintType, color, gradient);
    }
    finishUndoRedoSet();
}

void BoxesGroup::setSelectedCapStyle(Qt::PenCapStyle capStyle) {
    startNewUndoRedoSet();
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setStrokeCapStyle(capStyle);
    }
    finishUndoRedoSet();
}

void BoxesGroup::setSelectedJoinStyle(Qt::PenJoinStyle joinStyle) {
    startNewUndoRedoSet();
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setStrokeJoinStyle(joinStyle);
    }
    finishUndoRedoSet();
}

void BoxesGroup::setSelectedStrokeWidth(qreal strokeWidth, bool finish) {
    if(finish) {
        startNewUndoRedoSet();
    }
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setStrokeWidth(strokeWidth, finish);
    }
    if(finish) {
        finishUndoRedoSet();
    }
}

void BoxesGroup::startSelectedStrokeWidthTransform()
{
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->startStrokeWidthTransform();
    }
}

void BoxesGroup::startSelectedStrokeColorTransform()
{
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->startStrokeColorTransform();
    }
}

void BoxesGroup::startSelectedFillColorTransform()
{
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->startFillColorTransform();
    }
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
    if(mVisible) {
        p->save();

        p->setOpacity(p->opacity()*mTransformAnimator.getOpacity()*0.01 );
        foreach(BoundingBox *box, mChildren) {
            box->draw(p);
        }

        p->restore();
    }
}

void BoxesGroup::render(QPainter *p)
{
    if(mVisible) {
        p->save();

        p->setOpacity(p->opacity()*mTransformAnimator.getOpacity()*0.01 );
        foreach(BoundingBox *box, mChildren){
            box->render(p);
        }

        p->restore();
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
    if(!bT) {
        if(mChildren.isEmpty() && mParent != NULL) {
            mParent->removeChild(this);
        }
    }
    
}

bool BoxesGroup::isCurrentGroup() {
    return mIsCurrentGroup;
}

Edge *BoxesGroup::getPressedEdge(QPointF absPos) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        if(box->isSelected() ) {
            Edge *pathEdge = box->getEgde(absPos);
            if(pathEdge == NULL) continue;
            return pathEdge;
        }
    }
    return NULL;
}

BoundingBox *BoxesGroup::getPathAtFromAllAncestors(QPointF absPos)
{
    BoundingBox *boxAtPos = NULL;
    //foreachBoxInListInverted(mChildren) {
    BoundingBox *box;
    foreachInverted(box, mChildren) {
        if(box->isVisibleAndUnlocked()) {
            boxAtPos = box->getPathAtFromAllAncestors(absPos);
            if(boxAtPos != NULL) {
                break;
            }
        }
    }
    return boxAtPos;
}

Bone *BoxesGroup::getBoneAt(QPointF absPos)
{
    Bone *boxAtPos = NULL;
    BoundingBox *box;
    foreachInverted(box, mChildren) {
        if(box->isVisibleAndUnlocked() && box->isBone()) {
            if(box->pointInsidePath(absPos) ) {
                boxAtPos = (Bone*)box;
                break;
            }
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
    foreach(BoundingBox *box, mChildren) {
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
    foreach(BoundingBox *box, mChildren) {
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
    foreach(BoundingBox *box, mSelectedBoxes) {
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
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setStrokeSettings(strokeSettings, saveUndoRedo);
    }
    if(saveUndoRedo) {
        finishUndoRedoSet();
    }
}

void BoxesGroup::rotateSelectedBy(qreal rotBy, QPointF absOrigin,
                                  bool startTrans)
{
    if(mSelectedBoxes.count() == 1) {
        if(startTrans) {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->startRotTransform();
                box->rotateBy(rotBy);
            }
        } else {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->rotateBy(rotBy);
            }
        }
    } else {
        if(startTrans) {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->startRotTransform();
                box->startPosTransform();
                box->saveTransformPivot(absOrigin);
                box->rotateRelativeToSavedPivot(rotBy);
            }
        } else {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->rotateRelativeToSavedPivot(rotBy);
            }
        }
    }
}

void BoxesGroup::scaleSelectedBy(qreal scaleBy, QPointF absOrigin,
                                 bool startTrans) {
    if(mSelectedBoxes.count() == 1) {
        if(startTrans) {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->startScaleTransform();
                box->scale(scaleBy);
            }
        } else {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->scale(scaleBy);
            }
        }
    } else {
        if(startTrans) {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->startScaleTransform();
                box->startPosTransform();
                box->saveTransformPivot(absOrigin);
                box->scaleRelativeToSavedPivot(scaleBy);
            }
        } else {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->scaleRelativeToSavedPivot(scaleBy);
            }
        }
    }
}

void BoxesGroup::attachToBone(Bone *parentBone, CanvasMode currentCanvasMode) {
    startNewUndoRedoSet();
    if(currentCanvasMode == MOVE_POINT) {
        foreach(MovablePoint *point, mSelectedPoints) {
            point->setBone(parentBone);
        }
    } else if(currentCanvasMode == MOVE_PATH) {
        foreach(BoundingBox *box, mSelectedBoxes) {
            box->setBone(parentBone);
        }
    }
    finishUndoRedoSet();
}

void BoxesGroup::detachFromBone(CanvasMode currentCanvasMode) {
    startNewUndoRedoSet();
    if(currentCanvasMode == MOVE_POINT) {
        foreach(MovablePoint *point, mSelectedPoints) {
            point->setBone(NULL);
        }
    } else if(currentCanvasMode == MOVE_PATH) {
        foreach(BoundingBox *box, mSelectedBoxes) {
            box->setBone(NULL);
        }
    }
    finishUndoRedoSet();
}

QPointF BoxesGroup::getSelectedPivotPos()
{
    if(mSelectedBoxes.isEmpty()) return QPointF(0.f, 0.f);
    QPointF posSum = QPointF(0.f, 0.f);
    int count = mSelectedBoxes.length();
    foreach(BoundingBox *box, mSelectedBoxes) {
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
    BoundingBox *box;
    foreachInverted(box, mChildren) {
        box->applyTransformation(&mTransformAnimator);
        removeChild(box);
        parentGroup->addChild(box);
    }
    mParent->removeChild(this);
    finishUndoRedoSet();
}

void BoxesGroup::ungroupSelected()
{
    foreach(BoundingBox *box, mSelectedBoxes) {
        if(box->isGroup()) {
            ((BoxesGroup*) box)->ungroup();
        }
    }
}

void BoxesGroup::drawSelected(QPainter *p, CanvasMode currentCanvasMode)
{
    if(mVisible) {
        foreach(BoundingBox *box, mSelectedBoxes) {
            box->drawSelected(p, currentCanvasMode);
        }
        drawBoundingRect(p);
    }
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

    foreach(BoundingBox *box, mSelectedBoxes) {
        removeChild(box);
        box->deselect();
    }
    mSelectedBoxes.clear(); schedulePivotUpdate();

    finishUndoRedoSet();
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
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->deselect();
    }
    mSelectedBoxes.clear(); schedulePivotUpdate();
}

void BoxesGroup::bringSelectedBoxesToFront() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->bringToFront();
    }
}

void BoxesGroup::bringSelectedBoxesToEnd() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->bringToEnd();
    }
}

void BoxesGroup::moveSelectedBoxesDown() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->moveDown();
    }
}

void BoxesGroup::moveSelectedBoxesUp() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->moveUp();
    }
}

void BoxesGroup::deselectAllBoxes() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        removeBoxFromSelection(box);
    }
}

void BoxesGroup::selectAllBoxes() {
    foreach(BoundingBox *box, mChildren) {
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
    

    callUpdateSchedulers();
}

BoundingBox *BoxesGroup::getBoxAt(QPointF absPos) {
    BoundingBox *boxAtPos = NULL;

    BoundingBox *box;
    foreachInverted(box, mChildren) {
        if(box->isVisibleAndUnlocked()) {
            if(box->pointInsidePath(absPos)) {
                boxAtPos = box;
                break;
            }
        }
    }
    return boxAtPos;
}

MovablePoint *BoxesGroup::getPointAt(QPointF absPos, CanvasMode currentMode) {
    MovablePoint *pointAtPos = NULL;
    foreach(BoundingBox *box, mSelectedBoxes) {
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
    if(isRecordingAllPoints() ) {
        QList<BoundingBox*> parentBoxes;
        foreach(MovablePoint *point, mSelectedPoints) {
            BoundingBox *parentBox = point->getParent();
            if(parentBoxes.contains(parentBox) ) continue;
            parentBoxes << parentBox;
        }
        foreach(BoundingBox *parentBox, parentBoxes) {
            parentBox->finishAllPointsTransform();
        }
    } else {
        foreach(MovablePoint *point, mSelectedPoints) {
            point->finishTransform();
        }
    }
    finishUndoRedoSet();
}

void BoxesGroup::addContainedBoxesToSelection(QRectF rect)
{
    foreach(BoundingBox *box, mChildren) {
        if(box->isVisibleAndUnlocked()) {
            if(box->isContainedIn(rect) ) {
                addBoxToSelection(box);
            }
        }
    }
}

void BoxesGroup::finishSelectedBoxesTransform()
{
    startNewUndoRedoSet();
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->finishTransform();
    }
    finishUndoRedoSet();
}

void BoxesGroup::cancelSelectedBoxesTransform() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->cancelTransform();
    }
}

void BoxesGroup::cancelSelectedPointsTransform() {
    foreach(MovablePoint *point, mSelectedPoints) {
        point->cancelTransform();
    }
}

Bone *BoxesGroup::boneFromZIndex(int index) {
    if(index >= 0 && index < mChildren.count()) {
        BoundingBox *box = mChildren.at(index);
        if(box->isBone()) {
            return (Bone*) box;
        }
    }
    return NULL;
}

void BoxesGroup::attachToBoneFromSqlZId()
{
    BoundingBox::attachToBoneFromSqlZId();
    foreach (BoundingBox *box, mChildren) {
        box->attachToBoneFromSqlZId();
    }
}

BoxesGroup *BoxesGroup::loadChildrenFromSql(QString thisBoundingBoxId,
                                            bool loadInBox) {
    if(loadInBox) {
        BoxesGroup *newGroup = new BoxesGroup(mFillStrokeSettingsWidget, this);
        newGroup->loadChildrenFromSql(thisBoundingBoxId, false);
        return newGroup;
    }
    QSqlQuery query;
    QString queryStr;
    if(thisBoundingBoxId == "NULL") {
        queryStr = "SELECT id, boxtype FROM boundingbox WHERE parentboundingboxid IS " + thisBoundingBoxId;
    } else {
        queryStr = "SELECT id, boxtype FROM boundingbox WHERE parentboundingboxid = " + thisBoundingBoxId;
    }
    if(query.exec(queryStr) ) {
        int idId = query.record().indexOf("id");
        int idBoxType = query.record().indexOf("boxtype");
        while(query.next() ) {
            if(static_cast<BoundingBoxType>(
                        query.value(idBoxType).toInt()) == TYPE_BONE ) {
                new Bone(query.value(idId).toInt(), this);
            } else if(static_cast<BoundingBoxType>(
                        query.value(idBoxType).toInt()) == TYPE_VECTOR_PATH ) {
                new VectorPath(query.value(idId).toInt(), this);
            } else if(static_cast<BoundingBoxType>(
                          query.value(idBoxType).toInt()) == TYPE_GROUP ) {
                new BoxesGroup(query.value(idId).toInt(),
                               mFillStrokeSettingsWidget, this);
            }
        }
    } else {
        qDebug() << "Could not load children for boxesgroup with id " + thisBoundingBoxId;
    }
    return this;
}

int BoxesGroup::saveToSql(int parentId)
{
    QSqlQuery query;
    int boundingBoxId = BoundingBox::saveToSql(parentId);
    query.exec(QString("INSERT INTO boxesgroup (boundingboxid) VALUES (%1)").
                arg(boundingBoxId));
    foreach(BoundingBox *box, mChildren) {
        box->saveToSql(boundingBoxId);
    }
    return boundingBoxId;
}

void BoxesGroup::saveSelectedToSql()
{
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->saveToSql(0);
    }
}

void BoxesGroup::moveSelectedPointsBy(QPointF by, bool startTransform)
{
    if(startTransform) {
        if(isRecordingAllPoints() ) {
            QList<BoundingBox*> parentBoxes;
            foreach(MovablePoint *point, mSelectedPoints) {
                BoundingBox *parentBox = point->getParent();
                if(parentBoxes.contains(parentBox) ) continue;
                parentBoxes << parentBox;
            }
            foreach(BoundingBox *parentBox, parentBoxes) {
                parentBox->startAllPointsTransform();
            }

            foreach(MovablePoint *point, mSelectedPoints) {
                point->moveBy(by);
            }
        } else {
            foreach(MovablePoint *point, mSelectedPoints) {
                point->startTransform();
                point->moveBy(by);
            }
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
        foreach(BoundingBox *box, mSelectedBoxes) {
            box->startPosTransform();
            box->moveBy(by);
        }
    } else {
        foreach(BoundingBox *box, mSelectedBoxes) {
            box->moveBy(by);
        }
    }
}

void BoxesGroup::selectAndAddContainedPointsToSelection(QRectF absRect)
{
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->selectAndAddContainedPointsToList(absRect, &mSelectedPoints);
    }
}

void BoxesGroup::centerPivotPosition() {
    if(!mPivotChanged) {
        if(mChildren.isEmpty()) return;
        QPointF posSum = QPointF(0.f, 0.f);
        int count = mChildren.length();
        foreach(BoundingBox *box, mChildren) {
            posSum += box->getPivotAbsPos();
        }
        setPivotAbsPos(posSum/count, false, false);
    }
}

BoxesGroup* BoxesGroup::groupSelectedBoxes() {
    if(mSelectedBoxes.count() == 0) {
        return NULL;
    }
    startNewUndoRedoSet();
    BoxesGroup *newGroup = new BoxesGroup(mFillStrokeSettingsWidget, this);
    BoundingBox *box;
    foreachInverted(box, mSelectedBoxes) {
        removeChild(box);
        box->deselect();
        newGroup->addChild(box);
    }
    newGroup->selectAllBoxes();
    mSelectedBoxes.clear(); schedulePivotUpdate();
    finishUndoRedoSet();
    return newGroup;
}

void BoxesGroup::addChild(BoundingBox *child)
{
    startNewUndoRedoSet();
    child->setParent(this);
    addChildToListAt(mChildren.count(), child);
    centerPivotPosition();
    finishUndoRedoSet();
}

void BoxesGroup::addChildToListAt(int index, BoundingBox *child, bool saveUndoRedo) {
    mChildren.insert(index, child);
    updateChildrenId(index, saveUndoRedo);
    if(saveUndoRedo) {
        addUndoRedo(new AddChildToListUndoRedo(this, index, child));
    }
    child->incNumberPointers();
}

void BoxesGroup::updateChildrenId(int firstId, bool saveUndoRedo) {
    updateChildrenId(firstId, mChildren.length() - 1, saveUndoRedo);
}

void BoxesGroup::updateChildrenId(int firstId, int lastId, bool saveUndoRedo) {
    if(saveUndoRedo) startNewUndoRedoSet();
    for(int i = firstId; i <= lastId; i++) {
        mChildren.at(i)->setZListIndex(i, saveUndoRedo);
    }
    if(saveUndoRedo) finishUndoRedoSet();
}

void BoxesGroup::removeChildFromList(int id, bool saveUndoRedo)
{
    BoundingBox *box = mChildren.at(id);
    if(box->isSelected()) {
        removeBoxFromSelection(box);
    }
    if(saveUndoRedo) {
        addUndoRedo(new RemoveChildFromListUndoRedo(this, id, mChildren.at(id)) );
    }
    mChildren.removeAt(id);
    if(box->isGroup()) {
        BoxesGroup *group = (BoxesGroup*) box;
        if(group->isCurrentGroup()) {
            mMainWindow->getCanvas()->setCurrentBoxesGroup(group->getParent());
        }
    }
    updateChildrenId(id, saveUndoRedo);

    box->decNumberPointers();
}

void BoxesGroup::removeChild(BoundingBox *child)
{
    int index = mChildren.indexOf(child);
    if(index < 0) {
        return;
    }
    startNewUndoRedoSet();
    removeChildFromList(index);
    child->setParent(NULL);
    centerPivotPosition();
    finishUndoRedoSet();
}


void BoxesGroup::increaseChildZInList(BoundingBox *child)
{
    int index = mChildren.indexOf(child);
    if(index == mChildren.count() - 1) {
        return;
    }
    startNewUndoRedoSet();
    moveChildInList(index, index + 1);
    finishUndoRedoSet();
}

void BoxesGroup::decreaseChildZInList(BoundingBox *child)
{
    int index = mChildren.indexOf(child);
    if(index == 0) {
        return;
    }
    startNewUndoRedoSet();
    moveChildInList(index, index - 1);
    finishUndoRedoSet();
}

void BoxesGroup::bringChildToEndList(BoundingBox *child)
{
    int index = mChildren.indexOf(child);
    if(index == mChildren.count() - 1) {
        return;
    }
    startNewUndoRedoSet();
    moveChildInList(index, mChildren.length() - 1);
    finishUndoRedoSet();
}

void BoxesGroup::bringChildToFrontList(BoundingBox *child)
{
    int index = mChildren.indexOf(child);
    if(index == 0) {
        return;
    }
    startNewUndoRedoSet();
    moveChildInList(index, 0);
    finishUndoRedoSet();
}

void BoxesGroup::moveChildInList(int from, int to, bool saveUndoRedo) {
    mChildren.move(from, to);
    updateChildrenId(qMin(from, to), qMax(from, to), saveUndoRedo);
    if(saveUndoRedo) {
        addUndoRedo(new MoveChildInListUndoRedo(from, to, this) );
    }
}

void BoxesGroup::updateAfterCombinedTransformationChanged()
{
    foreach(BoundingBox *child, mChildren) {
        child->updateCombinedTransform();
    }
}

void BoxesGroup::clearAll()
{
    foreach(BoundingBox *box, mChildren) {
        box->clearAll();
        box->decNumberPointers();
    }
    mChildren.clear();
}
