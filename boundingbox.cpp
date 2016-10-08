#include "boundingbox.h"
#include "canvas.h"
#include "undoredo.h"
#include "boxesgroup.h"
#include <QDebug>
#include "mainwindow.h"
#include "keysview.h"

BoundingBox::BoundingBox(BoxesGroup *parent, BoundingBoxType type) :
    Transformable()
{
    addActiveAnimator(&mTransformAnimator);
    mAnimatorsCollection.addAnimator(&mTransformAnimator);

    mBoxesList = getMainWindow()->getBoxesList();
    mKeysView = getMainWindow()->getKeysView();
    mTransformAnimator.setUpdater(new TransUpdater(this) );
    mType = type;
    if(type == TYPE_VECTOR_PATH) {
        mName = "Path";
    } else {
        mName = "Group";
    }
    parent->addChild(this);
    mTransformAnimator.reset();
    mCombinedTransformMatrix.reset();
    updateCombinedTransform();
}

BoundingBox::BoundingBox(BoundingBoxType type) :
    Transformable() {
    mType = type;
    mTransformAnimator.reset();
    mCombinedTransformMatrix.reset();
}

BoundingBox::BoundingBox(int boundingBoxId,
                         BoxesGroup *parent, BoundingBoxType type) :
    Transformable() {
    addActiveAnimator(&mTransformAnimator);
    mAnimatorsCollection.addAnimator(&mTransformAnimator);

    QSqlQuery query;
    mType = type;

    QString queryStr = "SELECT * FROM boundingbox WHERE id = " +
            QString::number(boundingBoxId);
    if(query.exec(queryStr)) {
        query.next();
        int idName = query.record().indexOf("name");
        int idSx = query.record().indexOf("sx");
        int idSy = query.record().indexOf("sy");
        int idRot = query.record().indexOf("rot");
        int idDx = query.record().indexOf("dx");
        int idDy = query.record().indexOf("dy");
        int idPivotx = query.record().indexOf("pivotx");
        int idPivoty = query.record().indexOf("pivoty");
        int idPivotChanged = query.record().indexOf("pivotchanged");
        int idVisible = query.record().indexOf("visible");
        int idLocked = query.record().indexOf("locked");
        int idBoneZId = query.record().indexOf("bonezid");
        qreal sx = query.value(idSx).toReal();
        qreal sy = query.value(idSy).toReal();
        qreal rot = query.value(idRot).toReal();
        qreal dx = query.value(idDx).toReal();
        qreal dy = query.value(idDy).toReal();
        qreal pivotX = query.value(idPivotx).toReal();
        qreal pivotY = query.value(idPivoty).toReal();
        bool pivotChanged = query.value(idPivotChanged).toBool();
        bool visible = query.value(idVisible).toBool();
        bool locked = query.value(idLocked).toBool();
        mTransformAnimator.setScale(sx, sy);
        mTransformAnimator.setRotation(rot);
        mTransformAnimator.setPosition(dx, dy);
        mTransformAnimator.setPivot(pivotX, pivotY);
        mPivotChanged = pivotChanged;
        mLocked = locked;
        mVisible = visible;
        mSqlLoadBoneZId = query.value(idBoneZId).toInt();
        mName = query.value(idName).toString();
    } else {
        qDebug() << "Could not load boundingbox with id " << boundingBoxId;
    }

    parent->addChild(this);
}

void BoundingBox::updateAfterFrameChanged(int currentFrame) {
    mTransformAnimator.setFrame(currentFrame);
    mAnimatorsCollection.setFrame(currentFrame);
}

void BoundingBox::setParent(BoxesGroup *parent, bool saveUndoRedo) {
    if(saveUndoRedo) {
        addUndoRedo(new SetBoxParentUndoRedo(this, mParent, parent));
    }
    mParent = parent;

    updateCombinedTransform();
}

BoxesGroup *BoundingBox::getParent() {
    return mParent;
}

bool BoundingBox::isBone() {
    return mType == TYPE_BONE;
}

bool BoundingBox::isGroup() {
    return mType == TYPE_GROUP;
}

bool BoundingBox::isPath() {
    return mType == TYPE_VECTOR_PATH;
}

void BoundingBox::setPivotRelPos(QPointF relPos, bool saveUndoRedo,
                                 bool pivotChanged) {
    if(saveUndoRedo) {
        addUndoRedo(new SetPivotRelPosUndoRedo(this,
                        mTransformAnimator.getPivot(), relPos,
                        mPivotChanged, pivotChanged));
    }
    mPivotChanged = pivotChanged;
    mTransformAnimator.setPivot(relPos, saveUndoRedo);
    schedulePivotUpdate();
}

void BoundingBox::setPivotAbsPos(QPointF absPos, bool saveUndoRedo, bool pivotChanged) {
    QPointF newPos = getCombinedTransform().inverted().map(absPos);
    setPivotRelPos(newPos, saveUndoRedo, pivotChanged);
    updateCombinedTransform();
}

QPointF BoundingBox::getPivotAbsPos() {
    return getCombinedTransform().map(mTransformAnimator.getPivot());
}

void BoundingBox::select() {
    mSelected = true;
}

void BoundingBox::deselect() {
    mSelected = false;
}

bool BoundingBox::isContainedIn(QRectF absRect) {
    return absRect.contains(getBoundingRect());
}

BoundingBox *BoundingBox::getPathAtFromAllAncestors(QPointF absPos) {
    if(pointInsidePath(absPos)) {
        return this;
    } else {
        return NULL;
    }
}

const PaintSettings *BoundingBox::getFillSettings() {
    return NULL;
}

const StrokeSettings *BoundingBox::getStrokeSettings() {
    return NULL;
}

qreal BoundingBox::getCurrentCanvasScale() {
     return mParent->getCurrentCanvasScale();
}

void BoundingBox::drawBoundingRect(QPainter *p) {
    QPen pen = p->pen();
    p->setPen(QPen(QColor(0, 0, 0, 125), 1.f, Qt::DashLine));
    p->setBrush(Qt::NoBrush);
    p->drawRect(getBoundingRect());
    p->setPen(pen);
}

QMatrix BoundingBox::getCombinedTransform() {
    return mCombinedTransformMatrix;
}

void BoundingBox::applyTransformation(TransformAnimator *transAnimator) {
    Q_UNUSED(transAnimator);
}

void BoundingBox::scale(qreal scaleBy) {
    scale(scaleBy, scaleBy);
}

void BoundingBox::attachToBoneFromSqlZId() {
    setBone(mParent->boneFromZIndex(mSqlLoadBoneZId), false);
}

void BoundingBox::scale(qreal scaleXBy, qreal scaleYBy) {
    mTransformAnimator.scale(scaleXBy, scaleYBy);
}

#include <QSqlError>
int BoundingBox::saveToSql(int parentId) {
    QSqlQuery query;
    if(!query.exec(
        QString("INSERT INTO boundingbox (name, boxtype, sx, sy, "
                "rot, dx, dy, "
                "pivotx, pivoty, pivotchanged, visible, locked, "
                "bonezid, parentboundingboxid) "
                "VALUES ('%1', %2, %3, %4, %5, %6, %7, %8, %9, %10, %11, %12, %13, %14)").
                arg(mName).
                arg(mType).
                arg(mTransformAnimator.getXScale(), 0, 'f').
                arg(mTransformAnimator.getYScale(), 0, 'f').
                arg(mTransformAnimator.rot(), 0, 'f').
                arg(mTransformAnimator.dx(), 0, 'f').
                arg(mTransformAnimator.dy(), 0, 'f').
                arg(mTransformAnimator.getPivotX(), 0, 'f').
                arg(mTransformAnimator.getPivotY(), 0, 'f').
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

void BoundingBox::rotateBy(qreal rot) {
    mTransformAnimator.rotateRelativeToSavedValue(rot);
}

void BoundingBox::rotateRelativeToSavedPivot(qreal rot) {
    mTransformAnimator.rotateRelativeToSavedValue(rot, mSavedTransformPivot);
}

void BoundingBox::scaleRelativeToSavedPivot(qreal scaleXBy, qreal scaleYBy) {
    mTransformAnimator.scale(scaleXBy, scaleYBy, mSavedTransformPivot);
}

void BoundingBox::scaleRelativeToSavedPivot(qreal scaleBy) {
    scaleRelativeToSavedPivot(scaleBy, scaleBy);
}

void BoundingBox::moveBy(QPointF trans) {
    trans /= getCurrentCanvasScale();

    mTransformAnimator.translate(trans.x(), trans.y());
}

void BoundingBox::setAbsolutePos(QPointF pos, bool saveUndoRedo) {
    QMatrix combinedM = mParent->getCombinedTransform();
    QPointF newPos = combinedM.inverted().map(pos);
    setRelativePos(newPos, saveUndoRedo );
}

void BoundingBox::setRelativePos(QPointF relPos, bool saveUndoRedo) {
    mTransformAnimator.setPosition(relPos.x(), relPos.y() );
}

void BoundingBox::setRenderCombinedTransform() {
    mCombinedTransformMatrix = getCombinedRenderTransform();
}

void BoundingBox::saveTransformPivot(QPointF absPivot) {
    mSavedTransformPivot =
            mParent->getCombinedTransform().inverted().map(absPivot) -
            mTransformAnimator.getPivot();
}

QPointF BoundingBox::getAbsBoneAttachPoint() {
    return getPivotAbsPos();
}

void BoundingBox::startPosTransform() {
    mTransformAnimator.startPosTransform();
}

void BoundingBox::startRotTransform() {
    mTransformAnimator.startRotTransform();
}

void BoundingBox::startScaleTransform() {
    mTransformAnimator.startScaleTransform();
}

void BoundingBox::startTransform() {
    mTransformAnimator.startTransform();
}

void BoundingBox::finishTransform() {
    startNewUndoRedoSet();

    mTransformAnimator.finishTransform();

    finishUndoRedoSet();
}

void BoundingBox::cancelTransform() {
    mTransformAnimator.cancelTransform();
}

void BoundingBox::moveUp() {
    mParent->increaseChildZInList(this);
}

void BoundingBox::moveDown() {
    mParent->decreaseChildZInList(this);
}

void BoundingBox::bringToFront() {
    mParent->bringChildToEndList(this);
}

void BoundingBox::bringToEnd() {
    mParent->bringChildToFrontList(this);
}

void BoundingBox::setZListIndex(int z, bool saveUndoRedo) {
    if(saveUndoRedo) {
        addUndoRedo(new SetBoundingBoxZListIndexUnoRedo(mZListIndex, z, this));
    }
    mZListIndex = z;

}

int BoundingBox::getZIndex() {
    return mZListIndex;
}

QPointF BoundingBox::getAbsolutePos() {
    return QPointF(mCombinedTransformMatrix.dx(), mCombinedTransformMatrix.dy());
}

void BoundingBox::updateCombinedTransform() {
    if(mParent != NULL) {
        mCombinedTransformMatrix = mTransformAnimator.getCurrentValue()*
                mParent->getCombinedTransform();
    }
    updateAfterCombinedTransformationChanged();
}

QMatrix BoundingBox::getCombinedRenderTransform() {
    if(mParent == NULL) return QMatrix();
    return mTransformAnimator.getCurrentValue()*
            mParent->getCombinedRenderTransform();
}
