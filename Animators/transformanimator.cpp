#include "Animators/transformanimator.h"
#include "undoredo.h"
#include <QDebug>
#include <QSqlRecord>
#include "boxpathpoint.h"
#include "animatorupdater.h"
#include "qrealanimator.h"

BasicTransformAnimator::BasicTransformAnimator() :
    ComplexAnimator() {
    mRotAnimator = (new QrealAnimator)->ref<QrealAnimator>();
    mTransformUpdater = (new TransformUpdater(this))->ref<TransformUpdater>();

    prp_setName("transformation");
    mScaleAnimator->prp_setName("scale");
    mScaleAnimator->setCurrentPointValue(QPointF(1., 1.));
    mScaleAnimator->setPrefferedValueStep(0.05);
    mScaleAnimator->prp_setBlockedUpdater(mTransformUpdater.get());

    mRotAnimator->prp_setName("rot");
    mRotAnimator->qra_setCurrentValue(0.);
    mRotAnimator->prp_setBlockedUpdater(mTransformUpdater.get());

    mPosAnimator->prp_setName("trans");
    mPosAnimator->setCurrentPointValue(QPointF(0., 0.) );
    mPosAnimator->prp_setBlockedUpdater(mTransformUpdater.get());

    ca_addChildAnimator(mPosAnimator.data());
    ca_addChildAnimator(mRotAnimator.data());
    ca_addChildAnimator(mScaleAnimator.data());
}

void BasicTransformAnimator::resetScale(const bool &finish) {
    mScaleAnimator->setCurrentPointValue(QPointF(1., 1.), finish);
}

void BasicTransformAnimator::resetTranslation(const bool &finish) {
    mPosAnimator->setCurrentPointValue(QPointF(0., 0.), finish);
}

void BasicTransformAnimator::resetRotation(const bool &finish) {
    mRotAnimator->qra_setCurrentValue(0., finish);
}

void BasicTransformAnimator::reset(const bool &finish) {
    resetScale(finish);
    resetTranslation(finish);
    resetRotation(finish);
}

void BasicTransformAnimator::setScale(const qreal &sx, const qreal &sy) {
    mScaleAnimator->setCurrentPointValue(QPointF(sx, sy) );
}

void BasicTransformAnimator::setPosition(const qreal &x, const qreal &y,
                                         const bool &saveUndoRedo) {
    mPosAnimator->setCurrentPointValue(QPointF(x, y), saveUndoRedo);
}

void BasicTransformAnimator::setRotation(const qreal &rot) {
    mRotAnimator->qra_setCurrentValue(rot);
}

void BasicTransformAnimator::startRotTransform() {
    mRotAnimator->prp_startTransform();
}

void BasicTransformAnimator::startPosTransform() {
    mPosAnimator->prp_startTransform();
}

void BasicTransformAnimator::startScaleTransform() {
    mScaleAnimator->prp_startTransform();
}

qreal BasicTransformAnimator::getYScale() {
    return mScaleAnimator->getYValue();
}

qreal BasicTransformAnimator::getXScale() {
    return mScaleAnimator->getXValue();
}

void BasicTransformAnimator::rotateRelativeToSavedValue(const qreal &rotRel) {
    mRotAnimator->incSavedValueToCurrentValue(rotRel);
}

void BasicTransformAnimator::moveRelativeToSavedValue(const qreal &dX,
                                                      const qreal &dY) {
    mPosAnimator->incSavedValueToCurrentValue(dX, dY);
}

void BasicTransformAnimator::translate(const qreal &dX, const qreal &dY) {
    mPosAnimator->incCurrentValues(dX, dY);
}

void BasicTransformAnimator::scale(const qreal &sx, const qreal &sy) {
    mScaleAnimator->multSavedValueToCurrentValue(sx, sy);
}

qreal BasicTransformAnimator::dx() {
    return mPosAnimator->getXValue();
}

qreal BasicTransformAnimator::dy() {
    return mPosAnimator->getYValue();
}

qreal BasicTransformAnimator::rot() {
    return mRotAnimator->qra_getCurrentValue();
}

qreal BasicTransformAnimator::xScale() {
    return mScaleAnimator->getXValue();
}

qreal BasicTransformAnimator::yScale() {
    return mScaleAnimator->getYValue();
}

QPointF BasicTransformAnimator::pos() {
    return mPosAnimator->getCurrentPointValue();
}

QMatrix BasicTransformAnimator::getCurrentTransformationMatrix() {
    QMatrix matrix;

    matrix.translate(mPosAnimator->getXValue(),
                     mPosAnimator->getYValue());

    matrix.rotate(mRotAnimator->qra_getCurrentValue() );
    matrix.scale(mScaleAnimator->getXValue(),
                 mScaleAnimator->getYValue() );
    return matrix;
}

QMatrix BasicTransformAnimator::getTransformMatrixAtRelFrame(
                                    const int &relFrame) {
    QMatrix matrix;

    matrix.translate(mPosAnimator->getXValueAtRelFrame(relFrame),
                     mPosAnimator->getYValueAtRelFrame(relFrame));

    matrix.rotate(mRotAnimator->qra_getValueAtRelFrame(relFrame) );
    matrix.scale(mScaleAnimator->getXValueAtRelFrame(relFrame),
                 mScaleAnimator->getYValueAtRelFrame(relFrame) );
    return matrix;
}

void BasicTransformAnimator::duplicatePosAnimatorFrom(
                        QPointFAnimator *source) {
    source->makeDuplicate(mPosAnimator.data());
}

void BasicTransformAnimator::duplicateScaleAnimatorFrom(
                        QPointFAnimator *source) {
    source->makeDuplicate(mScaleAnimator.data());
}

void BasicTransformAnimator::duplicateRotAnimatorFrom(
                        QrealAnimator *source) {
    source->makeDuplicate(mRotAnimator.data());
}

void BasicTransformAnimator::moveByAbs(
                        const QMatrix &combinedTrans,
                        const QPointF &absTrans) {
    moveToAbs(combinedTrans,
              combinedTrans.map(mPosAnimator->getSavedPointValue()) +
              absTrans);
}

void BasicTransformAnimator::moveToAbs(
                        const QMatrix &combinedTrans,
                        const QPointF &absPos) {
    setAbsolutePos(combinedTrans, absPos, false);
}

void BasicTransformAnimator::setAbsolutePos(
                        const QMatrix &combinedTrans,
                        const QPointF &pos,
                        const bool &saveUndoRedo) {
    QPointF newPos = combinedTrans.inverted().map(pos);
    setRelativePos(newPos, saveUndoRedo);
}

void BasicTransformAnimator::setRelativePos(const QPointF &relPos,
                                            const bool &saveUndoRedo) {
    mPosAnimator->setCurrentPointValue(relPos, saveUndoRedo);
}

void BasicTransformAnimator::rotateRelativeToSavedValue(const qreal &rotRel,
                                                        const QPointF &pivot) {
    QMatrix matrix;
    matrix.translate(pivot.x(),
                     pivot.y());
    matrix.rotate(rotRel);
    matrix.translate(-pivot.x() + mPosAnimator->getSavedXValue(),
                     -pivot.y() + mPosAnimator->getSavedYValue() );
    rotateRelativeToSavedValue(rotRel);
    mPosAnimator->setCurrentPointValue(QPointF(matrix.dx(), matrix.dy()) );
}

QPointF BasicTransformAnimator::mapRelPosToAbs(const QPointF &relPos) const {
    return mCombinedTransform.map(relPos);
}

QPointF BasicTransformAnimator::mapAbsPosToRel(const QPointF &absPos) const {
    return mCombinedTransform.inverted().map(absPos);
}

void BasicTransformAnimator::updateRelativeTransform() {
    mRelTransform = getCurrentTransformationMatrix();
    updateCombinedTransform();
}

void BasicTransformAnimator::updateCombinedTransform() {
    if(mParentTransformAnimator.isNull()) {
        mCombinedTransform = mRelTransform;
    } else {
        mCombinedTransform = mRelTransform *
                             mParentTransformAnimator->getCombinedTransform();
    }
    emit combinedTransformChanged();
}

const QMatrix &BasicTransformAnimator::getCombinedTransform() const {
    return mCombinedTransform;
}

const QMatrix &BasicTransformAnimator::getRelativeTransform() const {
    return mRelTransform;
}

void BasicTransformAnimator::setParentTransformAnimator(
                                BasicTransformAnimator *parent) {
    if(!mParentTransformAnimator.isNull()) {
        disconnect(parent, SIGNAL(combinedTransformChanged()),
                   this, SLOT(updateCombinedTransform()));
    }
    if(parent == NULL) {
        mParentTransformAnimator.reset();
    } else {
        mParentTransformAnimator = parent->ref<BasicTransformAnimator>();
        connect(parent, SIGNAL(combinedTransformChanged()),
                this, SLOT(updateCombinedTransform()));
    }
    updateCombinedTransform();
}

void BasicTransformAnimator::scaleRelativeToSavedValue(const qreal &sx,
                                                      const qreal &sy,
                                                      const QPointF &pivot) {
    QMatrix matrix;

    matrix.translate(pivot.x(), pivot.y());
    matrix.rotate(mRotAnimator->qra_getCurrentValue());
    matrix.scale(sx, sy);
    matrix.rotate(-mRotAnimator->qra_getCurrentValue());
    matrix.translate(-pivot.x() + mPosAnimator->getSavedXValue(),
                     -pivot.y() + mPosAnimator->getSavedYValue());

    scale(sx, sy);
    mPosAnimator->setCurrentPointValue(QPointF(matrix.dx(), matrix.dy()) );
}

void BasicTransformAnimator::makeDuplicate(BasicTransformAnimator *target) {
    target->duplicatePosAnimatorFrom(mPosAnimator.data());
    target->duplicateScaleAnimatorFrom(mScaleAnimator.data());
    target->duplicateRotAnimatorFrom(mRotAnimator.data());
}

BoxTransformAnimator::BoxTransformAnimator(BoundingBox *parent) :
    BasicTransformAnimator() {
    mOpacityAnimator = (new QrealAnimator)->ref<QrealAnimator>();
    mPivotAnimator = (new BoxPathPoint(parent))->ref<PointAnimator>();
    mPivotAnimator->prp_setName("pivot");
    mPivotAnimator->setCurrentPointValue(QPointF(0., 0.) );
    mPivotAnimator->prp_setBlockedUpdater(mTransformUpdater.get());
    mOpacityAnimator->prp_setName("opacity");
    mOpacityAnimator->qra_setValueRange(0., 100.);
    mOpacityAnimator->setPrefferedValueStep(5.);
    mOpacityAnimator->qra_setCurrentValue(100.);
    mOpacityAnimator->freezeMinMaxValues();
    mOpacityAnimator->prp_setBlockedUpdater(mTransformUpdater.get());

    ca_addChildAnimator(mPivotAnimator.data());
    ca_addChildAnimator(mOpacityAnimator.data());

    prp_setBlockedUpdater(new TransUpdater(parent) );
}

MovablePoint *BoxTransformAnimator::getPivotMovablePoint() {
    return mPivotAnimator.data();
}

#include <QSqlError>
int BoxTransformAnimator::saveToSql(QSqlQuery *query,
                                     const int &parentId) {
    Q_UNUSED(parentId);
    int posAnimatorId = mPosAnimator->saveToSql(query);
    int scaleAnimatorId = mScaleAnimator->saveToSql(query);
    int pivotAnimatorId = mPivotAnimator->saveToSql(query);
    int rotAnimatorId = mRotAnimator->saveToSql(query);
    int opacityAnimatorId = mOpacityAnimator->saveToSql(query);
    if(!query->exec(
        QString("INSERT INTO transformanimator (posanimatorid, scaleanimatorid, "
                "pivotanimatorid, rotanimatorid, opacityanimatorid ) "
                "VALUES (%1, %2, %3, %4, %5)").
                arg(posAnimatorId).
                arg(scaleAnimatorId).
                arg(pivotAnimatorId).
                arg(rotAnimatorId).
                arg(opacityAnimatorId) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }

    return query->lastInsertId().toInt();
}

void BoxTransformAnimator::loadFromSql(const int &transformAnimatorId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM transformanimator WHERE id = " +
            QString::number(transformAnimatorId);
    if(query.exec(queryStr)) {
        query.next();
        int posanimatorid = query.record().indexOf("posanimatorid");
        int scaleanimatorid = query.record().indexOf("scaleanimatorid");
        int pivotanimatorid = query.record().indexOf("pivotanimatorid");
        int rotanimatorid = query.record().indexOf("rotanimatorid");
        int opacityanimatorid = query.record().indexOf("opacityanimatorid");

        //loadKeysFromSql(qrealAnimatorId);

        mPosAnimator->loadFromSql(query.value(posanimatorid).toInt());
        mScaleAnimator->loadFromSql(query.value(scaleanimatorid).toInt());
        mPivotAnimator->loadFromSql(query.value(pivotanimatorid).toInt());
        mRotAnimator->loadFromSql(query.value(rotanimatorid).toInt());
        mOpacityAnimator->loadFromSql(query.value(opacityanimatorid).toInt());
    } else {
        qDebug() << "Could not load qpointfanimator with id " << transformAnimatorId;
    }
}

void BoxTransformAnimator::resetPivot(const bool &finish) {
    mPivotAnimator->setCurrentPointValue(QPointF(0., 0.), finish);
}

void BoxTransformAnimator::reset(const bool &finish) {
    BasicTransformAnimator::reset(finish);
    resetPivot(finish);
}

void BoxTransformAnimator::startOpacityTransform() {
    mOpacityAnimator->prp_startTransform();
}

void BoxTransformAnimator::setOpacity(const qreal &newOpacity) {
    mOpacityAnimator->qra_setCurrentValue(newOpacity);
}

void BoxTransformAnimator::pivotTransformStarted() {
    if(!mPosAnimator->prp_isDescendantRecording()) {
        mPosAnimator->prp_startTransform();
    }
}

void BoxTransformAnimator::pivotTransformFinished() {
    if(!mPosAnimator->prp_isDescendantRecording()) {
        mPosAnimator->prp_finishTransform();
    }
}

void BoxTransformAnimator::setPivotWithoutChangingTransformation(
                                const QPointF &point,
                                const bool &saveUndoRedo) {
    QMatrix currentMatrix;
    qreal pivotX = mPivotAnimator->getXValue();
    qreal pivotY = mPivotAnimator->getYValue();
    currentMatrix.translate(pivotX + mPosAnimator->getXValue(),
                            pivotY + mPosAnimator->getYValue());

    currentMatrix.rotate(mRotAnimator->qra_getCurrentValue() );
    currentMatrix.scale(mScaleAnimator->getXValue(),
                        mScaleAnimator->getYValue() );

    currentMatrix.translate(-pivotX,
                            -pivotY);

    QMatrix futureMatrix;
    futureMatrix.translate(point.x() + mPosAnimator->getXValue(),
                           point.y() + mPosAnimator->getYValue());

    futureMatrix.rotate(mRotAnimator->qra_getCurrentValue() );
    futureMatrix.scale(mScaleAnimator->getXValue(),
                       mScaleAnimator->getYValue() );

    futureMatrix.translate(-point.x(),
                           -point.y());


    mPosAnimator->incAllValues(currentMatrix.dx() - futureMatrix.dx(),
                               currentMatrix.dy() - futureMatrix.dy(),
                               saveUndoRedo,
                               false,
                               saveUndoRedo);

    mPivotAnimator->setCurrentPointValue(point,
                                         saveUndoRedo,
                                         false,
                                         saveUndoRedo);
}

void BoxTransformAnimator::setPivot(const QPointF &point,
                                 const bool &finish) {
    mPivotAnimator->setCurrentPointValue(point, finish);

    //callUpdater();
}

QPointF BoxTransformAnimator::getPivot() {
    return mPivotAnimator->getCurrentPointValue();
}

QPointF BoxTransformAnimator::getPivotAbs() {
    return mPivotAnimator->getAbsolutePos();
}

qreal BoxTransformAnimator::getOpacityAtRelFrame(const int &relFrame) {
    return mOpacityAnimator->qra_getValueAtRelFrame(relFrame);
}

bool BoxTransformAnimator::rotOrScaleOrPivotRecording() {
    return mRotAnimator->prp_isDescendantRecording() ||
           mScaleAnimator->prp_isDescendantRecording() ||
           mPivotAnimator->prp_isDescendantRecording();
}

qreal BoxTransformAnimator::getPivotX() {
    return mPivotAnimator->getXValue();
}

qreal BoxTransformAnimator::getPivotY() {
    return mPivotAnimator->getYValue();
}

qreal BoxTransformAnimator::getOpacity() {
    return mOpacityAnimator->qra_getCurrentValue();
}

QMatrix BoxTransformAnimator::getCurrentTransformationMatrix() {
    QMatrix matrix;
    qreal pivotX = mPivotAnimator->getXValue();
    qreal pivotY = mPivotAnimator->getYValue();

    matrix.translate(pivotX + mPosAnimator->getXValue(),
                     pivotY + mPosAnimator->getYValue());

    matrix.rotate(mRotAnimator->qra_getCurrentValue() );
    matrix.scale(mScaleAnimator->getXValue(),
                 mScaleAnimator->getYValue() );

    matrix.translate(-pivotX,
                     -pivotY);
    return matrix;
}

QMatrix BoxTransformAnimator::getTransformMatrixAtRelFrame(
                                    const int &relFrame) {
    QMatrix matrix;
    qreal pivotX = mPivotAnimator->getXValueAtRelFrame(relFrame);
    qreal pivotY = mPivotAnimator->getYValueAtRelFrame(relFrame);

    matrix.translate(pivotX + mPosAnimator->getXValueAtRelFrame(relFrame),
                     pivotY + mPosAnimator->getYValueAtRelFrame(relFrame));

    matrix.rotate(mRotAnimator->qra_getValueAtRelFrame(relFrame) );
    matrix.scale(mScaleAnimator->getXValueAtRelFrame(relFrame),
                 mScaleAnimator->getYValueAtRelFrame(relFrame) );

    matrix.translate(-pivotX,
                     -pivotY);
    return matrix;
}

QMatrix BasicTransformAnimator::getCombinedTransformMatrixAtRelFrame(
                                    const int &relFrame) {
    if(mParentTransformAnimator.data() == NULL) {
        return getTransformMatrixAtRelFrame(relFrame);
    } else {
        return getTransformMatrixAtRelFrame(relFrame)*
                mParentTransformAnimator->
                    getCombinedTransformMatrixAtRelFrame(relFrame);
    }
}

void BoxTransformAnimator::makeDuplicate(BoxTransformAnimator *target) {
    BasicTransformAnimator::makeDuplicate(target);
    target->duplicatePivotAnimatorFrom(mPivotAnimator.data());
    target->duplicateOpacityAnimatorFrom(mOpacityAnimator.data());
}

void BoxTransformAnimator::duplicatePivotAnimatorFrom(
        QPointFAnimator *source) {
    source->makeDuplicate(mPivotAnimator.data());
}

void BoxTransformAnimator::duplicateOpacityAnimatorFrom(
        QrealAnimator *source) {
    source->makeDuplicate(mOpacityAnimator.data());
}
