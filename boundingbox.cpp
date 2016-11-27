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
    mEffectsAnimators.blockPointer();
    mEffectsAnimators.setName("effects");
    addActiveAnimator(&mTransformAnimator);
    mAnimatorsCollection.addAnimator(&mTransformAnimator);
    mTransformAnimator.blockPointer();

    mBoxesList = getMainWindow()->getBoxesList();
    mKeysView = getMainWindow()->getKeysView();
    mTransformAnimator.setUpdater(new TransUpdater(this) );
    mType = type;

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

QPixmap BoundingBox::applyEffects(const QPixmap& pixmap, qreal scale) {
    if(mEffects.isEmpty() ) return pixmap;
    QImage im = pixmap.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);;
    fmt_filters::image img(im.bits(), im.width(), im.height());
    foreach(PixmapEffect *effect, mEffects) {
        effect->apply(&im, img, scale);
    }
    return QPixmap::fromImage(im);
}

#include <QSqlError>
int BoundingBox::saveToSql(int parentId) {
    int transfromAnimatorId = mTransformAnimator.saveToSql();
    QSqlQuery query;
    if(!query.exec(
        QString("INSERT INTO boundingbox (name, boxtype, transformanimatorid, "
                "pivotchanged, visible, locked, "
                "parentboundingboxid) "
                "VALUES ('%1', %2, %3, %4, %5, %6, %7)").
                arg(mName).
                arg(mType).
                arg(transfromAnimatorId).
                arg(boolToSql(mPivotChanged)).
                arg(boolToSql(mVisible) ).
                arg(boolToSql(mLocked) ).
                arg(parentId)
                ) ) {
        qDebug() << query.lastError() << endl << query.lastQuery();
    }

    return query.lastInsertId().toInt();
}

void BoundingBox::loadFromSql(int boundingBoxId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM boundingbox WHERE id = " +
            QString::number(boundingBoxId);
    if(query.exec(queryStr)) {
        query.next();
        int idName = query.record().indexOf("name");
        int idTransformAnimatorId = query.record().indexOf("transformanimatorid");
        int idPivotChanged = query.record().indexOf("pivotchanged");
        int idVisible = query.record().indexOf("visible");
        int idLocked = query.record().indexOf("locked");

        int transformAnimatorId = query.value(idTransformAnimatorId).toInt();
        bool pivotChanged = query.value(idPivotChanged).toBool();
        bool visible = query.value(idVisible).toBool();
        bool locked = query.value(idLocked).toBool();
        mTransformAnimator.loadFromSql(transformAnimatorId);
        mPivotChanged = pivotChanged;
        mLocked = locked;
        mVisible = visible;
        mName = query.value(idName).toString();
    } else {
        qDebug() << "Could not load boundingbox with id " << boundingBoxId;
    }
}

void BoundingBox::updateUpdateTransform() {
    mUpdateCanvasTransform = mMainWindow->getCanvas()->getCombinedTransform();
    mUpdateTransform = mCombinedTransformMatrix;
}

void BoundingBox::setAwaitingUpdate(bool bT) {
    mAwaitingUpdate = bT;
    if(bT) {
        updateUpdateTransform();
    } else {
        afterSuccessfulUpdate();

        if(mHighQualityPaint) {
            mOldPixmap = mNewPixmap;
            mOldPixBoundingRect = mPixBoundingRectClippedToView;
            mOldTransform = mUpdateTransform;
        }

        mOldAllUglyPixmap = mAllUglyPixmap;
        mOldAllUglyBoundingRect = mAllUglyBoundingRect;
        mOldAllUglyTransform = mAllUglyTransform;
        updateUglyPaintTransform();
    }
}

QRectF BoundingBox::getBoundingRectClippedToView() {
    return mPixBoundingRectClippedToView;
}

void BoundingBox::updatePrettyPixmap() {
    QSizeF sizeF = mPixBoundingRectClippedToView.size();
    mNewPixmap = QPixmap(QSize(ceil(sizeF.width()), ceil(sizeF.height())) );
    mNewPixmap.fill(Qt::transparent);

    QPainter p(&mNewPixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.translate(-mPixBoundingRectClippedToView.topLeft());
    p.setTransform(QTransform(mUpdateTransform), true);

    draw(&p);
    p.end();

    mNewPixmap = applyEffects(mNewPixmap, mUpdateCanvasTransform.m11());
}

void BoundingBox::updateAllUglyPixmap() {
    QMatrix inverted = mUpdateCanvasTransform.inverted();
    mAllUglyTransform = inverted*mUpdateTransform;
    mAllUglyBoundingRect = (mUpdateTransform.inverted()*mAllUglyTransform).mapRect(mPixBoundingRect);
    QSizeF sizeF = mAllUglyBoundingRect.size();
    mAllUglyPixmap = QPixmap(QSize(ceil(sizeF.width()), ceil(sizeF.height())) );
    mAllUglyPixmap.fill(Qt::transparent);

    QPainter p(&mAllUglyPixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.translate(-mAllUglyBoundingRect.topLeft());
    p.setTransform(QTransform(mAllUglyTransform), true);

    draw(&p);
    p.end();

    mAllUglyPixmap = applyEffects(mAllUglyPixmap);
}

bool BoundingBox::shouldRedoUpdate() {
    return mRedoUpdate;
}

void BoundingBox::setRedoUpdateToFalse() {
    mRedoUpdate = false;
}

void BoundingBox::redoUpdate() {
    mRedoUpdate = true;
}

void BoundingBox::drawPixmap(QPainter *p) {
    p->save();
    p->setCompositionMode(mCompositionMode);
    if(mAwaitingUpdate || !mHighQualityPaint) {
        bool paintOld = mUglyPaintTransform.m11() < mOldAllUglyPaintTransform.m11()
                && mHighQualityPaint;

        if(paintOld) {
            p->save();
            QMatrix clipMatrix = mUglyPaintTransform;
            clipMatrix.translate(mOldPixBoundingRect.left(), mOldPixBoundingRect.top());
            QRegion clipRegion;
            clipRegion = clipMatrix.map(clipRegion.united(mOldPixmap.rect()));
            QRegion clipRegion2;
            clipRegion2 = clipRegion2.united(QRect(-100000, -100000, 200000, 200000));
            clipRegion = clipRegion2.subtracted(clipRegion);

            p->setClipRegion(clipRegion);
        }

        p->setTransform(QTransform(mOldAllUglyPaintTransform), true);
        p->drawPixmap(mOldAllUglyBoundingRect.topLeft(), mOldAllUglyPixmap);

        if(paintOld) {
            p->restore();

            p->setTransform(QTransform(mUglyPaintTransform), true);
            p->drawPixmap(mOldPixBoundingRect.topLeft(), mOldPixmap);
        }
    } else if(mHighQualityPaint) {
        p->drawPixmap(mPixBoundingRectClippedToView.topLeft(), mNewPixmap);
    }

    p->restore();

    if(mSelected) drawBoundingRect(p);
}

void BoundingBox::awaitUpdate() {
    if(mAwaitingUpdate) return;
    setAwaitingUpdate(true);
    mMainWindow->addBoxAwaitingUpdate(this);
}

#include "updatescheduler.h"
void BoundingBox::scheduleAwaitUpdate() {
    if(mAwaitUpdateScheduled) return;
    setAwaitUpdateScheduled(true);
    addUpdateScheduler(new AwaitUpdateUpdateScheduler(this));
}

void BoundingBox::setAwaitUpdateScheduled(bool bT) {
    mAwaitUpdateScheduled = bT;
}

void BoundingBox::setCompositionMode(QPainter::CompositionMode compositionMode)
{
    mCompositionMode = compositionMode;
    scheduleAwaitUpdate();
}

void BoundingBox::updateEffectsMarginIfNeeded() {
    if(mEffectsMarginUpdateNeeded) {
        mEffectsMarginUpdateNeeded = false;
        updateEffectsMargin();
    }
}

void BoundingBox::updateEffectsMargin() {
    qreal newMargin = 0.;
    foreach(PixmapEffect *effect, mEffects) {
        qreal effectMargin = effect->getMargin();
        if(effectMargin > newMargin) newMargin = effectMargin;
    }

    mEffectsMargin = newMargin;
}

void BoundingBox::scheduleEffectsMarginUpdate() {
    scheduleAwaitUpdate();
    mEffectsMarginUpdateNeeded = true;
}

void BoundingBox::resetScale() {
    mTransformAnimator.resetScale();
}

void BoundingBox::resetTranslation() {
    mTransformAnimator.resetTranslation();
}

void BoundingBox::resetRotation() {
    mTransformAnimator.resetRotation();
}

void BoundingBox::updateAfterFrameChanged(int currentFrame) {
    mTransformAnimator.setFrame(currentFrame);
    mAnimatorsCollection.setFrame(currentFrame);
    mEffectsAnimators.setFrame(currentFrame);
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

bool BoundingBox::isGroup() {
    return mType == TYPE_GROUP;
}

bool BoundingBox::isVectorPath() {
    return mType == TYPE_VECTOR_PATH;
}

bool BoundingBox::isCircle() {
    return mType == TYPE_CIRCLE;
}

bool BoundingBox::isRect() {
    return mType == TYPE_RECTANGLE;
}

bool BoundingBox::isText() {
    return mType == TYPE_TEXT;
}

void BoundingBox::copyTransformationTo(BoundingBox *box) {
    if(mPivotChanged) box->disablePivotAutoAdjust();

    mTransformAnimator.copyTransformationTo(box->getTransformAnimator());
}

void BoundingBox::disablePivotAutoAdjust() {
    mPivotChanged = true;
}

void BoundingBox::enablePivotAutoAdjust() {
    mPivotChanged = false;
}

void BoundingBox::setPivotRelPos(QPointF relPos, bool saveUndoRedo,
                                 bool pivotChanged) {
    if(saveUndoRedo) {
        addUndoRedo(new SetPivotRelPosUndoRedo(this,
                        mTransformAnimator.getPivot(), relPos,
                        mPivotChanged, pivotChanged));
    }
    mPivotChanged = pivotChanged;
    mTransformAnimator.setPivotWithoutChangingTransformation(relPos, saveUndoRedo);
    schedulePivotUpdate();
}

void BoundingBox::setPivotAbsPos(QPointF absPos, bool saveUndoRedo, bool pivotChanged) {
    QPointF newPos = mapAbsPosToRel(absPos);
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
    return absRect.contains(getPixBoundingRect());
}

BoundingBox *BoundingBox::getPathAtFromAllAncestors(QPointF absPos) {
    if(absPointInsidePath(absPos)) {
        return this;
    } else {
        return NULL;
    }
}

QPointF BoundingBox::mapAbsPosToRel(QPointF absPos) {
    return mCombinedTransformMatrix.inverted().map(absPos);
}

const PaintSettings *BoundingBox::getFillSettings() {
    return NULL;
}

const StrokeSettings *BoundingBox::getStrokeSettings() {
    return NULL;
}

qreal BoundingBox::getCurrentCanvasScale() {
     return mMainWindow->getCanvas()->getCurrentCanvasScale();
}

void BoundingBox::drawAsBoundingRect(QPainter *p, QPainterPath path) {
    p->save();
    QPen pen = QPen(QColor(0, 0, 0, 125), 1.f, Qt::DashLine);
    pen.setCosmetic(true);
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);
    p->setTransform(QTransform(mCombinedTransformMatrix), true);
    p->drawPath(path);
    p->restore();
}

void BoundingBox::drawBoundingRect(QPainter *p) {
    drawAsBoundingRect(p, mBoundingRect);
}

QMatrix BoundingBox::getCombinedTransform() const {
    return mCombinedTransformMatrix;
}

void BoundingBox::applyTransformation(TransformAnimator *transAnimator) {
    Q_UNUSED(transAnimator);
}

void BoundingBox::scale(qreal scaleBy) {
    scale(scaleBy, scaleBy);
}

void BoundingBox::scale(qreal scaleXBy, qreal scaleYBy) {
    mTransformAnimator.scale(scaleXBy, scaleYBy);
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

    mTransformAnimator.moveRelativeToSavedValue(trans.x(), trans.y());

    //mTransformAnimator.translate(trans.x(), trans.y());
}

void BoundingBox::setAbsolutePos(QPointF pos, bool saveUndoRedo) {
    QMatrix combinedM = mParent->getCombinedTransform();
    QPointF newPos = combinedM.inverted().map(pos);
    setRelativePos(newPos, saveUndoRedo );
}

void BoundingBox::setRelativePos(QPointF relPos, bool saveUndoRedo) {
    mTransformAnimator.setPosition(relPos.x(), relPos.y() );
}

void BoundingBox::saveTransformPivot(QPointF absPivot) {
    mSavedTransformPivot =
            mParent->mapAbsPosToRel(absPivot) -
            mTransformAnimator.getPivot();
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

void BoundingBox::updateRelativeTransform() {
    mRelativeTransformMatrix = mTransformAnimator.getCurrentValue();
    updateCombinedTransform();
}

void BoundingBox::updateCombinedTransform() {
    if(mParent == NULL) {
        updateAfterCombinedTransformationChanged();
    } else {
        mCombinedTransformMatrix = mRelativeTransformMatrix*
                mParent->getCombinedTransform();

        updateAfterCombinedTransformationChanged();

        if(mAwaitingUpdate) {
            redoUpdate();
        } else {
            scheduleAwaitUpdate();
        }
        updateUglyPaintTransform();
    }
}

void BoundingBox::updateUglyPaintTransform() {
    mUglyPaintTransform = mOldTransform.inverted()*mCombinedTransformMatrix;
    mOldAllUglyPaintTransform = mOldAllUglyTransform.inverted()*mCombinedTransformMatrix;
}

TransformAnimator *BoundingBox::getTransformAnimator() {
    return &mTransformAnimator;
}

QMatrix BoundingBox::getCombinedRenderTransform() {
    if(mParent == NULL) return QMatrix();
    return mTransformAnimator.getCurrentValue()*
            mParent->getCombinedRenderTransform();
}

QMatrix BoundingBox::getCombinedFinalRenderTransform() {
    if(mParent == NULL) return QMatrix();
    return mTransformAnimator.getCurrentValue()*
            mParent->getCombinedFinalRenderTransform();
}
