#include "boundingbox.h"
#include "canvas.h"
#include "undoredo.h"
#include "boxesgroup.h"
#include <QDebug>

BoundingBox::BoundingBox(BoxesGroup *parent, BoundingBoxType type) :
    Transformable(parent)
{
    mType = type;
    if(type == TYPE_VECTOR_PATH) {
        mName = "Path";
    } else {
        mName = "Group";
    }
    parent->addChild(this);
    mTransformMatrix.reset();
    mCombinedTransformMatrix.reset();
    updateCombinedTransform();
}

BoundingBox::BoundingBox(MainWindow *window, BoundingBoxType type) : Transformable(window)
{
    mType = type;
    mTransformMatrix.reset();
    mCombinedTransformMatrix.reset();
}

BoundingBox::BoundingBox(int boundingBoxId,
                         BoxesGroup *parent, BoundingBoxType type) :
    Transformable(parent) {
    QSqlQuery query;
    mType = type;

    QString queryStr = "SELECT * FROM boundingbox WHERE id = " +
            QString::number(boundingBoxId);
    if(query.exec(queryStr)) {
        query.next();
        int idName = query.record().indexOf("name");
        int idM11 = query.record().indexOf("m11_trans");
        int idM12 = query.record().indexOf("m12_trans");
        int idM21 = query.record().indexOf("m21_trans");
        int idM22 = query.record().indexOf("m22_trans");
        int idDx = query.record().indexOf("dx_trans");
        int idDy = query.record().indexOf("dy_trans");
        int idPivotx = query.record().indexOf("pivotx");
        int idPivoty = query.record().indexOf("pivoty");
        int idPivotChanged = query.record().indexOf("pivotchanged");
        int idVisible = query.record().indexOf("visible");
        int idLocked = query.record().indexOf("locked");
        int idBoneZId = query.record().indexOf("bonezid");
        qreal m11 = query.value(idM11).toReal();
        qreal m12 = query.value(idM12).toReal();
        qreal m21 = query.value(idM21).toReal();
        qreal m22 = query.value(idM22).toReal();
        qreal dx = query.value(idDx).toReal();
        qreal dy = query.value(idDy).toReal();
        qreal pivotX = query.value(idPivotx).toReal();
        qreal pivotY = query.value(idPivoty).toReal();
        bool pivotChanged = query.value(idPivotChanged).toBool();
        bool visible = query.value(idVisible).toBool();
        bool locked = query.value(idLocked).toBool();
        //mTransformMatrix.setMatrix(m11, m12, m21, m22, dx, dy);
        mTransformMatrix.setPivot(pivotX, pivotY, false);
        mPivotChanged = pivotChanged;
        mLocked = locked;
        mVisible = visible;
        mSqlLoadBoneZId = query.value(idBoneZId).toInt();
        mName = query.value(idName).toString();
    } else {
        qDebug() << "Could not load boundingbox with id " << boundingBoxId;
    }

    parent->addChild(this);
    updateCombinedTransform();
}

void BoundingBox::setParent(BoxesGroup *parent, bool saveUndoRedo) {
    if(saveUndoRedo) {
        addUndoRedo(new SetBoxParentUndoRedo(this, mParent, parent));
    }
    mParent = parent;
    if(mParent == NULL) {
        scheduleRemoval();
    } else {
        descheduleRemoval();
    }
    updateCombinedTransform();
}

BoxesGroup *BoundingBox::getParent()
{
    return mParent;
}

bool BoundingBox::isBone()
{
    return mType == TYPE_BONE;
}

bool BoundingBox::isGroup()
{
    return mType == TYPE_GROUP;
}

bool BoundingBox::isPath() {
    return mType == TYPE_VECTOR_PATH;
}

void BoundingBox::setPivotRelPos(QPointF relPos, bool saveUndoRedo, bool pivotChanged) {
    mPivotChanged = pivotChanged;
    mTransformMatrix.setPivot(relPos, saveUndoRedo);
    schedulePivotUpdate();
}

void BoundingBox::scheduleRemoval()
{
    mScheduledForRemove = true;
}

void BoundingBox::descheduleRemoval()
{
    mScheduledForRemove = false;
}

bool BoundingBox::isScheduldedForRemoval()
{
    return mScheduledForRemove;
}

void BoundingBox::setPivotAbsPos(QPointF absPos, bool saveUndoRedo, bool pivotChanged) {
    QPointF newPos = getCombinedTransform().inverted().map(absPos);
    setPivotRelPos(newPos, saveUndoRedo, pivotChanged);
    updateCombinedTransform();
}

QPointF BoundingBox::getPivotAbsPos()
{
    return getCombinedTransform().map(mTransformMatrix.getPivot());
}

void BoundingBox::select()
{
    mSelected = true;
    scheduleBoxesListRepaint();
    scheduleRepaint();
}

void BoundingBox::deselect()
{
    mSelected = false;
    scheduleBoxesListRepaint();
    scheduleRepaint();
}

bool BoundingBox::isContainedIn(QRectF absRect)
{
    return absRect.contains(getBoundingRect());
}

BoundingBox *BoundingBox::getPathAtFromAllAncestors(QPointF absPos) {
    if(pointInsidePath(absPos)) {
        return this;
    } else {
        return NULL;
    }
}

PaintSettings BoundingBox::getFillSettings()
{
    return PaintSettings();
}

StrokeSettings BoundingBox::getStrokeSettings()
{
    return StrokeSettings();
}

qreal BoundingBox::getCurrentCanvasScale()
{
     return mParent->getCurrentCanvasScale();
}

void BoundingBox::drawBoundingRect(QPainter *p) {
    QPen pen = p->pen();
    p->setPen(QPen(QColor(0, 0, 0, 125), 1.f, Qt::DashLine));
    p->setBrush(Qt::NoBrush);
    p->drawRect(getBoundingRect());
    p->setPen(pen);
}

QMatrix BoundingBox::getCombinedTransform()
{
    return mCombinedTransformMatrix;
}

void BoundingBox::applyTransformation(TransformAnimator *transAnimator)
{

}

void BoundingBox::scale(qreal scaleBy) {
    scale(scaleBy, scaleBy);
}

void BoundingBox::attachToBoneFromSqlZId()
{
    setBone(mParent->boneFromZIndex(mSqlLoadBoneZId), false);
}

void BoundingBox::scale(qreal scaleXBy, qreal scaleYBy)
{
    mTransformMatrix.scale(scaleXBy, scaleYBy);
    updateCombinedTransform();
}

#include <QSqlError>
int BoundingBox::saveToSql(int parentId)
{
    QSqlQuery query;
    if(!query.exec(
        QString("INSERT INTO boundingbox (name, boxtype, m11_trans, m12_trans, "
                "m21_trans, m22_trans, dx_trans, dy_trans, "
                "pivotx, pivoty, pivotchanged, visible, locked, "
                "bonezid, parentboundingboxid) "
                "VALUES ('%1', %2, %3, %4, %5, %6, %7, %8, %9, %10, %11, %12, %13, %14, %15)").
                arg(mName).
                arg(mType).
//                arg(mTransformMatrix.m11(), 0, 'f').
//                arg(mTransformMatrix.m12(), 0, 'f').
//                arg(mTransformMatrix.m21(), 0, 'f').
//                arg(mTransformMatrix.m22(), 0, 'f').
//                arg(mTransformMatrix.dx(), 0, 'f').
//                arg(mTransformMatrix.dy(), 0, 'f').
//                arg(mRelPivotPos.x(), 0, 'f').
//                arg(mRelPivotPos.y(), 0, 'f').
                arg(boolToSql(mPivotChanged)).
                arg(boolToSql(mVisible) ).
                arg(boolToSql(mLocked) ).
                arg((mBone == NULL) ? -1 : mBone->getZIndex() ).
                arg( (parentId == 0) ? "NULL" : QString::number(parentId) )
                ) ) {
        qDebug() << query.lastError() << endl << query.lastQuery();
    }

    return query.lastInsertId().toInt();
}

void BoundingBox::rotateBy(qreal rot)
{
    mTransformMatrix.rotateRelativeToSavedValue(rot);
    updateCombinedTransform();
}

void BoundingBox::rotateRelativeToSavedPivot(qreal rot) {
    mTransformMatrix.rotateRelativeToSavedValue(rot, mSavedTransformPivot);
    updateCombinedTransform();
}

void BoundingBox::scaleRelativeToSavedPivot(qreal scaleBy)
{
    mTransformMatrix.scale(scaleBy, scaleBy, mSavedTransformPivot);
    updateCombinedTransform();
}

void BoundingBox::moveBy(QPointF trans)
{
    trans /= getCurrentCanvasScale();

    mTransformMatrix.translate(trans.x(), trans.y());
    updateCombinedTransform();
    schedulePivotUpdate();
}

void BoundingBox::saveTransformPivot(QPointF absPivot) {
    mSavedTransformPivot =
            mParent->getCombinedTransform().inverted().map(absPivot) -
            mTransformMatrix.getPivot();
}

QPointF BoundingBox::getAbsBoneAttachPoint()
{
    return getPivotAbsPos();
}

void BoundingBox::startTransform()
{
    mTransformMatrix.saveCurrentValue();
}

void BoundingBox::finishTransform()
{
    startNewUndoRedoSet();

    mTransformMatrix.finishTransform();
//    TransformChildParentUndoRedo *undoRedo = new TransformChildParentUndoRedo(this,
//                                                           mSavedTransformMatrix,
//                                                           mTransformMatrix);
//    addUndoRedo(undoRedo);

    finishUndoRedoSet();
}

void BoundingBox::cancelTransform() {
    mTransformMatrix.retrieveSavedValue();
    updateCombinedTransform();
}

void BoundingBox::moveUp()
{
    mParent->increaseChildZInList(this);
}

void BoundingBox::moveDown()
{
    mParent->decreaseChildZInList(this);
}

void BoundingBox::bringToFront()
{
    mParent->bringChildToEndList(this);
}

void BoundingBox::bringToEnd()
{
    mParent->bringChildToFrontList(this);
}

void BoundingBox::setZListIndex(int z, bool saveUndoRedo)
{
    if(saveUndoRedo) {
        addUndoRedo(new SetBoundingBoxZListIndexUnoRedo(mZListIndex, z, this));
    }
    mZListIndex = z;
    scheduleRepaint();
}

int BoundingBox::getZIndex() {
    return mZListIndex;
}

void BoundingBox::setTransformation(QMatrix transMatrix)
{
    //mTransformMatrix = transMatrix;
    updateCombinedTransform();
    schedulePivotUpdate();
}

QPointF BoundingBox::getAbsolutePos()
{
    return QPointF(mCombinedTransformMatrix.dx(), mCombinedTransformMatrix.dy());
}

void BoundingBox::updateCombinedTransform()
{
    if(mParent != NULL) {
        mCombinedTransformMatrix = mTransformMatrix.getCurrentValue()*
                mParent->getCombinedTransform();
    }
    updateAfterCombinedTransformationChanged();
}
