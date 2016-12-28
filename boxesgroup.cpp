#include "boxesgroup.h"
#include "undoredo.h"
#include <QApplication>
#include "mainwindow.h"
#include "ctrlpoint.h"
#include "circle.h"
#include "rectangle.h"

bool BoxesGroup::mCtrlsAlwaysVisible = false;

bool zLessThan(BoundingBox *box1, BoundingBox *box2)
{
    return box1->getZIndex() > box2->getZIndex();
}

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
    foreach(BoundingBox *child, mChildren) {
        child->updateAllBoxes();
    }
}

BoxesGroup::BoxesGroup(FillStrokeSettingsWidget *fillStrokeSetting) :
    BoundingBox(BoundingBoxType::TYPE_CANVAS)
{
    mFillStrokeSettingsWidget = fillStrokeSetting;
}

BoxesGroup::~BoxesGroup()
{
    clearBoxesSelection();
    clearPointsSelection();
    foreach(BoundingBox *box, mChildren) {
        box->decNumberPointers();
    }
}

void BoxesGroup::loadFromSql(int boundingBoxId) {
    BoundingBox::loadFromSql(boundingBoxId);
    loadChildrenFromSql(boundingBoxId, false);
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

void BoxesGroup::convertSelectedBoxesToPath() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->objectToPath();
    }
}

void BoxesGroup::setSelectedFontFamilyAndStyle(QString family, QString style)
{
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setFontFamilyAndStyle(family, style);
    }
}

void BoxesGroup::setSelectedFontSize(qreal size)
{
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setFontSize(size);
    }
}

void BoxesGroup::applyBlurToSelected() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new BlurEffect());
    }
}

void BoxesGroup::applyBrushEffectToSelected() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new BrushEffect());
    }
}

void BoxesGroup::applyLinesEffectToSelected() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new LinesEffect());
    }
}

void BoxesGroup::applyCirclesEffectToSelected() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new CirclesEffect());
    }
}

void BoxesGroup::resetSelectedTranslation() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->resetTranslation();
    }
}

void BoxesGroup::resetSelectedScale() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->resetScale();
    }
}

void BoxesGroup::resetSelectedRotation() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->resetRotation();
    }
}

PathPoint *BoxesGroup::createNewPointOnLineNearSelected(QPointF absPos,
                                                        bool adjust) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        PathPoint *point = box->createNewPointOnLineNear(absPos, adjust);
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

bool BoxesGroup::absPointInsidePath(QPointF absPos)
{
    foreach(BoundingBox *box, mChildren) {
        if(box->absPointInsidePath(absPos)) {
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

void BoxesGroup::clearPointsSelectionOrDeselect() {
    if(mSelectedPoints.isEmpty() ) {
        deselectAllBoxes();
    } else {
        clearPointsSelection();
    }
}

void BoxesGroup::setSelectedFillGradient(Gradient *gradient, bool finish) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setFillGradient(gradient, finish);
    }
}

void BoxesGroup::setSelectedStrokeGradient(Gradient *gradient, bool finish) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setStrokeGradient(gradient, finish);
    }
}

void BoxesGroup::setSelectedFillFlatColor(Color color, bool finish) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setFillFlatColor(color, finish);
    }
}

void BoxesGroup::setSelectedStrokeFlatColor(Color color, bool finish) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setStrokeFlatColor(color, finish);
    }
}

void BoxesGroup::setSelectedFillPaintType(PaintType paintType,
                                          Color color, Gradient *gradient) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setFillPaintType(paintType, color, gradient);
    }
}

void BoxesGroup::setSelectedStrokePaintType(PaintType paintType,
                                            Color color, Gradient *gradient) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setStrokePaintType(paintType, color, gradient);
    }
}

void BoxesGroup::setSelectedCapStyle(Qt::PenCapStyle capStyle) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setStrokeCapStyle(capStyle);
    }
}

void BoxesGroup::setSelectedJoinStyle(Qt::PenJoinStyle joinStyle) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setStrokeJoinStyle(joinStyle);
    }
}

void BoxesGroup::setFillGradient(Gradient *gradient, bool finish)
{
    foreach(BoundingBox *box, mChildren) {
        box->setFillGradient(gradient, finish);
    }
}

void BoxesGroup::setStrokeGradient(Gradient *gradient, bool finish)
{
    foreach(BoundingBox *box, mChildren) {
        box->setStrokeGradient(gradient, finish);
    }
}

void BoxesGroup::setFillFlatColor(Color color, bool finish)
{
    foreach(BoundingBox *box, mChildren) {
        box->setFillFlatColor(color, finish);
    }
}

void BoxesGroup::setStrokeFlatColor(Color color, bool finish)
{
    foreach(BoundingBox *box, mChildren) {
        box->setStrokeFlatColor(color, finish);
    }
}

void BoxesGroup::setFillPaintType(PaintType paintType,
                                  Color color, Gradient *gradient)
{
    foreach(BoundingBox *box, mChildren) {
        box->setFillPaintType(paintType, color, gradient);
    }
}

void BoxesGroup::setStrokePaintType(PaintType paintType,
                                    Color color, Gradient *gradient)
{
    foreach(BoundingBox *box, mChildren) {
        box->setStrokePaintType(paintType, color, gradient);
    }
}

void BoxesGroup::setStrokeCapStyle(Qt::PenCapStyle capStyle)
{
    foreach(BoundingBox *box, mChildren) {
        box->setStrokeCapStyle(capStyle);
    }
}

void BoxesGroup::setStrokeJoinStyle(Qt::PenJoinStyle joinStyle)
{
    foreach(BoundingBox *box, mChildren) {
        box->setStrokeJoinStyle(joinStyle);
    }
}

void BoxesGroup::setStrokeWidth(qreal strokeWidth, bool finish)
{
    foreach(BoundingBox *box, mChildren) {
        box->setStrokeWidth(strokeWidth, finish);
    }
}

void BoxesGroup::startStrokeWidthTransform()
{
    foreach(BoundingBox *box, mChildren) {
        box->startStrokeWidthTransform();
    }
}

void BoxesGroup::startStrokeColorTransform()
{
    foreach(BoundingBox *box, mChildren) {
        box->startStrokeColorTransform();
    }
}

void BoxesGroup::startFillColorTransform()
{
    foreach(BoundingBox *box, mChildren) {
        box->startFillColorTransform();
    }
}

void BoxesGroup::setSelectedAnimated(bool animated) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setAnimated(animated);
    }
}

void BoxesGroup::setSelectedStrokeWidth(qreal strokeWidth, bool finish) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setStrokeWidth(strokeWidth, finish);
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

void BoxesGroup::updateBoundingRect() {
    QPainterPath boundingPaths = QPainterPath();
    foreach(BoundingBox *child, mChildren) {
        boundingPaths.addPath(
                    child->getRelativeTransform().
                    map(child->getBoundingRectPath()));
    }
    QRectF boundingPathsRect = boundingPaths.boundingRect().
                    adjusted(-mEffectsMargin, -mEffectsMargin,
                             mEffectsMargin, mEffectsMargin);
    mBoundingRect = QPainterPath();
    mBoundingRect.addRect(boundingPathsRect);

    mPixBoundingRect = mUpdateTransform.mapRect(boundingPathsRect);
    mMappedBoundingRect = mUpdateTransform.map(mBoundingRect);
    updatePixBoundingRectClippedToView();
}

void BoxesGroup::draw(QPainter *p)
{
    if(mVisible) {
        p->save();
        p->setTransform(QTransform(mCombinedTransformMatrix.inverted()), true);
        foreach(BoundingBox *box, mChildren) {
            //box->draw(p);
            box->drawPixmap(p);
        }

        p->restore();
    }
}

void BoxesGroup::render(QPainter *p)
{
    if(mVisible) {
        p->save();

        foreach(BoundingBox *box, mChildren){
            box->render(p);
        }

        p->restore();
    }
}

void BoxesGroup::renderFinal(QPainter *p)
{
    if(mVisible) {
        p->save();

        foreach(BoundingBox *box, mChildren){
            box->renderFinal(p);
        }

        p->restore();
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
    p->drawPath(mBoundingRect);

    p->restore();
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

void BoxesGroup::rotateSelectedPointsBy(qreal rotBy, QPointF absOrigin,
                                        bool startTrans)
{
    if(mSelectedPoints.isEmpty()) return;
    if(startTrans) {
        foreach(MovablePoint *point, mSelectedPoints) {
            point->startTransform();
            point->saveTransformPivot(absOrigin);
            point->rotateRelativeToSavedPivot(rotBy);
        }
    } else {
        foreach(MovablePoint *point, mSelectedPoints) {
            point->rotateRelativeToSavedPivot(rotBy);
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

void BoxesGroup::scaleSelectedBy(qreal scaleXBy, qreal scaleYBy,
                                 QPointF absOrigin,
                                 bool startTrans) {
    if(mSelectedBoxes.count() == 1) {
        if(startTrans) {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->startScaleTransform();
                box->scale(scaleXBy, scaleYBy);
            }
        } else {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->scale(scaleXBy, scaleYBy);
            }
        }
    } else {
        if(startTrans) {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->startScaleTransform();
                box->startPosTransform();
                box->saveTransformPivot(absOrigin);
                box->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
            }
        } else {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
            }
        }
    }
}

void BoxesGroup::scaleSelectedPointsBy(qreal scaleXBy, qreal scaleYBy,
                                       QPointF absOrigin,
                                       bool startTrans) {
    if(mSelectedPoints.isEmpty()) return;
    if(startTrans) {
        foreach(MovablePoint *point, mSelectedPoints) {
            point->startTransform();
            point->saveTransformPivot(absOrigin);
            point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
        }
    } else {
        foreach(MovablePoint *point, mSelectedPoints) {
            point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
        }
    }
}

QPointF BoxesGroup::getSelectedBoxesAbsPivotPos()
{
    if(mSelectedBoxes.isEmpty()) return QPointF(0., 0.);
    QPointF posSum = QPointF(0., 0.);
    int count = mSelectedBoxes.length();
    foreach(BoundingBox *box, mSelectedBoxes) {
        posSum += box->getPivotAbsPos();
    }
    return posSum/count;
}

QPointF BoxesGroup::getSelectedPointsAbsPivotPos() {
    if(mSelectedPoints.isEmpty()) return QPointF(0., 0.);
    QPointF posSum = QPointF(0., 0.);
    int count = mSelectedPoints.length();
    foreach(MovablePoint *point, mSelectedPoints) {
        posSum += point->getAbsolutePos();
    }
    return posSum/count;
}

bool BoxesGroup::isPointsSelectionEmpty() {
    return mSelectedPoints.isEmpty();
}

int BoxesGroup::getPointsSelectionCount() {
    return mSelectedPoints.length();
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
    clearBoxesSelection();
    BoxesGroup *parentGroup = (BoxesGroup*) mParent;
    BoundingBox *box;
    foreachInverted(box, mChildren) {
        box->applyTransformation(&mTransformAnimator);
        removeChild(box);
        parentGroup->addChild(box);
    }
    mParent->removeChild(this);
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

void BoxesGroup::centerPivotForSelected() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->centerPivotPosition(true);
    }
}

void BoxesGroup::removeSelectedPointsApproximateAndClearList() {
    foreach(MovablePoint *point, mSelectedPoints) {
        point->deselect();
        point->removeApproximate();
    }
    mSelectedPoints.clear(); schedulePivotUpdate();
}

void BoxesGroup::removeSelectedPointsAndClearList()
{
    foreach(MovablePoint *point, mSelectedPoints) {
        point->deselect();
        point->remove();
    }
    mSelectedPoints.clear(); schedulePivotUpdate();
}

void BoxesGroup::removeSelectedBoxesAndClearList()
{
    foreach(BoundingBox *box, mSelectedBoxes) {
        removeChild(box);
        box->deselect();
    }
    mSelectedBoxes.clear(); schedulePivotUpdate();
}

void BoxesGroup::clearPointsSelection()
{
    foreach(MovablePoint *point, mSelectedPoints) {
        point->deselect();
    }
    mSelectedPoints.clear(); schedulePivotUpdate();
}

const PaintSettings *BoxesGroup::getFillSettings() {
    return mChildren.first()->getFillSettings();
}

const StrokeSettings *BoxesGroup::getStrokeSettings() {
    return mChildren.first()->getStrokeSettings();
}

void BoxesGroup::updateSelectedPointsAfterCtrlsVisiblityChanged() {
    if(!BoxesGroup::mCtrlsAlwaysVisible) {
        QList<MovablePoint*> pointsToDeselect;
        foreach(MovablePoint *point, mSelectedPoints) {
            pointsToDeselect << point;
        }
        foreach(MovablePoint *point, pointsToDeselect) {
            removePointFromSelection(point);
        }
    }
}

bool BoxesGroup::getCtrlsAlwaysVisible()
{
    return BoxesGroup::mCtrlsAlwaysVisible;
}

void BoxesGroup::setCtrlsAlwaysVisible(bool bT)
{
    BoxesGroup::mCtrlsAlwaysVisible = bT;
    MainWindow::getInstance()->getCanvas()->ctrlsVisiblityChanged();
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
    sortSelectedBoxesByZAscending();
    setCurrentFillStrokeSettingsFromBox(box);
    mMainWindow->setCurrentShapesMenuBox(box);
}

void BoxesGroup::addPointToSelection(MovablePoint *point)
{
    if(point->isSelected()) {
        return;
    }
    point->select();
    mSelectedPoints.append(point); schedulePivotUpdate();
}

void BoxesGroup::removeBoxFromSelection(BoundingBox *box) {
    box->deselect();
    mSelectedBoxes.removeOne(box); schedulePivotUpdate();
}

void BoxesGroup::removePointFromSelection(MovablePoint *point) {
    point->deselect();
    mSelectedPoints.removeOne(point); schedulePivotUpdate();
}

void BoxesGroup::clearBoxesSelection() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->deselect();
    }
    mSelectedBoxes.clear(); schedulePivotUpdate();
}

void BoxesGroup::applyCurrentTransformation() {
    QPointF absPivot = getPivotAbsPos();
    qreal rotation = mTransformAnimator.rot();
    qreal scaleX = mTransformAnimator.xScale();
    qreal scaleY = mTransformAnimator.yScale();
    foreach(BoundingBox *box, mChildren) {
        box->saveTransformPivot(absPivot);
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

void BoxesGroup::applyCurrentTransformationToSelected() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->applyCurrentTransformation();
    }
}

void BoxesGroup::sortSelectedBoxesByZAscending() {
    qSort(mSelectedBoxes.begin(), mSelectedBoxes.end(), zLessThan);
}

void BoxesGroup::raiseSelectedBoxesToTop() {
    BoundingBox *box;
    foreachInverted(box, mSelectedBoxes) {
        box->bringToFront();
    }
}

void BoxesGroup::lowerSelectedBoxesToBottom() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->bringToEnd();
    }
}

void BoxesGroup::lowerSelectedBoxes() {
    BoundingBox *box;
    int lastZ = -10000;
    bool lastBoxChanged = true;
    foreachInverted(box, mSelectedBoxes) {
        int boxZ = box->getZIndex();
        if(boxZ - 1 != lastZ || lastBoxChanged) {
            box->moveDown();
        }
        lastZ = boxZ;
        lastBoxChanged = boxZ - box->getZIndex() != 0;
    }
}

void BoxesGroup::raiseSelectedBoxes() {
    int lastZ = -10000;
    bool lastBoxChanged = true;
    foreach(BoundingBox *box, mSelectedBoxes) {
        int boxZ = box->getZIndex();
        if(boxZ + 1 != lastZ || lastBoxChanged) {
            box->moveUp();
        }
        lastZ = boxZ;
        lastBoxChanged = boxZ - box->getZIndex() != 0;
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

        PathPoint *firstPoint = selectedPathPoints.first();
        PathPoint *secondPoint = selectedPathPoints.last();
        if(firstPoint->isEndPoint() && secondPoint->isEndPoint()) {
            firstPoint->connectToPoint(secondPoint);
        }        
    }
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
        PathPoint *firstPoint = selectedPathPoints.first();
        PathPoint *secondPoint = selectedPathPoints.last();
        firstPoint->disconnectFromPoint(secondPoint);
    }
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
        PathPoint *firstPoint = selectedPathPoints.first();
        PathPoint *secondPoint = selectedPathPoints.last();
        QPointF sumPos = firstPoint->getAbsolutePos() + secondPoint->getAbsolutePos();
        firstPoint->remove();
        secondPoint->moveToAbs(sumPos/2);        
    }
}

void BoxesGroup::setPointCtrlsMode(CtrlsMode mode) {
    foreach(MovablePoint *point, mSelectedPoints) {
        if(point->isPathPoint()) {
            ( (PathPoint*)point)->setCtrlsMode(mode);
        }
    }
}

BoundingBox *BoxesGroup::getBoxAt(QPointF absPos) {
    BoundingBox *boxAtPos = NULL;

    BoundingBox *box;
    foreachInverted(box, mChildren) {
        if(box->isVisibleAndUnlocked()) {
            if(box->absPointInsidePath(absPos)) {
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
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->finishTransform();
    }
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
                point->moveByAbs(by);
            }
        } else {
            foreach(MovablePoint *point, mSelectedPoints) {
                point->startTransform();
                point->moveByAbs(by);
            }
        }
    } else {
        foreach(MovablePoint *point, mSelectedPoints) {
            point->moveByAbs(by);
        }
    }
}

void BoxesGroup::moveSelectedBoxesBy(QPointF by, bool startTransform)
{
    by = mapAbsPosToRel(by) - mapAbsPosToRel(QPointF(0., 0.));
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

void BoxesGroup::centerPivotPosition(bool finish) {
    if(mChildren.isEmpty()) return;
    QPointF posSum = QPointF(0.f, 0.f);
    int count = mChildren.length();
    foreach(BoundingBox *box, mChildren) {
        posSum += box->getPivotAbsPos();
    }
    setPivotAbsPos(posSum/count, finish, mPivotChanged);
}

BoxesGroup* BoxesGroup::groupSelectedBoxes() {
    if(mSelectedBoxes.count() == 0) {
        return NULL;
    }
    BoxesGroup *newGroup = new BoxesGroup(this);
    BoundingBox *box;
    foreachInverted(box, mSelectedBoxes) {
        removeChild(box);
        box->deselect();
        newGroup->addChild(box);
    }
    newGroup->selectAllBoxes();
    mSelectedBoxes.clear(); schedulePivotUpdate();
    return newGroup;
}

void BoxesGroup::addChild(BoundingBox *child)
{
    child->setParent(this);
    addChildToListAt(mChildren.count(), child);
    if(!mPivotChanged) centerPivotPosition();
}

void BoxesGroup::addChildToListAt(int index, BoundingBox *child, bool saveUndoRedo) {
    mChildren.insert(index, child);
    updateChildrenId(index, saveUndoRedo);
    if(saveUndoRedo) {
        addUndoRedo(new AddChildToListUndoRedo(this, index, child));
    }
    child->incNumberPointers();

    if(child->isAnimated()) {
        emit addAnimatedBoundingBoxSignal(child);
    }
}

void BoxesGroup::updateChildrenId(int firstId, bool saveUndoRedo) {
    updateChildrenId(firstId, mChildren.length() - 1, saveUndoRedo);
}

void BoxesGroup::updateChildrenId(int firstId, int lastId, bool saveUndoRedo) {
    for(int i = firstId; i <= lastId; i++) {
        mChildren.at(i)->setZListIndex(i, saveUndoRedo);
    }
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

    if(box->isAnimated()) {
        emit removeAnimatedBoundingBoxSignal(box);
    }
}

void BoxesGroup::removeChild(BoundingBox *child)
{
    int index = mChildren.indexOf(child);
    if(index < 0) {
        return;
    }
    removeChildFromList(index);
    child->setParent(NULL);
    if(!mPivotChanged) centerPivotPosition();
}


void BoxesGroup::increaseChildZInList(BoundingBox *child)
{
    int index = mChildren.indexOf(child);
    if(index == mChildren.count() - 1) {
        return;
    }
    moveChildInList(child, index, index + 1);
}

void BoxesGroup::decreaseChildZInList(BoundingBox *child)
{
    int index = mChildren.indexOf(child);
    if(index == 0) {
        return;
    }
    moveChildInList(child, index, index - 1);
}

void BoxesGroup::bringChildToEndList(BoundingBox *child)
{
    int index = mChildren.indexOf(child);
    if(index == mChildren.count() - 1) {
        return;
    }
    moveChildInList(child, index, mChildren.length() - 1);
}

void BoxesGroup::bringChildToFrontList(BoundingBox *child)
{
    int index = mChildren.indexOf(child);
    if(index == 0) {
        return;
    }
    moveChildInList(child, index, 0);
}

void BoxesGroup::moveChildInList(BoundingBox *child,
                                 int from, int to,
                                 bool saveUndoRedo) {
    mChildren.move(from, to);
    updateChildrenId(qMin(from, to), qMax(from, to), saveUndoRedo);
    if(saveUndoRedo) {
        addUndoRedo(new MoveChildInListUndoRedo(child, from, to, this) );
    }

    if(child->isAnimated()) {
        emit changeChildZSignal(from, to);
    }
}

void BoxesGroup::updateAfterCombinedTransformationChanged()
{
    foreach(BoundingBox *child, mChildren) {
        child->updateCombinedTransform();
    }
}
