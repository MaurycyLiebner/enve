#include "Boxes/pathbox.h"
#include "updatescheduler.h"
#include "mainwindow.h"

PathBox::PathBox(BoxesGroup *parent,
                 const BoundingBoxType &type) :
    BoundingBox(parent, type) {
    mFillSettings->setTargetPathBox(this);
    mStrokeSettings->setTargetPathBox(this);

    ca_addChildAnimator(mFillSettings.data());
    ca_addChildAnimator(mStrokeSettings.data());

    mFillGradientPoints->initialize(this);
    mStrokeGradientPoints->initialize(this);

    mFillGradientPoints->prp_setUpdater(new GradientPointsUpdater(true, this));
    mFillGradientPoints->prp_blockUpdater();
    mStrokeGradientPoints->prp_setUpdater(new GradientPointsUpdater(false, this));
    mStrokeGradientPoints->prp_blockUpdater();

    mFillSettings->setGradientPoints(mFillGradientPoints.data());
    mStrokeSettings->setGradientPoints(mStrokeGradientPoints.data());

    mStrokeSettings->setLineWidthUpdaterTarget(this);
    mFillSettings->setPaintPathTarget(this);

    schedulePathUpdate();
}

PathBox::~PathBox()
{
    if(mFillSettings->getGradient() != NULL) {
        mFillSettings->getGradient()->removePath(this);
    }
    if(mStrokeSettings->getGradient() != NULL) {
        mStrokeSettings->getGradient()->removePath(this);
    }
}

#include <QSqlError>
int PathBox::prp_saveToSql(QSqlQuery *query, const int &parentId) {
    int boundingBoxId = BoundingBox::prp_saveToSql(query, parentId);

    int fillPts = mFillGradientPoints->startPoint->prp_saveToSql(query);
    int strokePts = mStrokeGradientPoints->startPoint->prp_saveToSql(query);

    int fillSettingsId = mFillSettings->prp_saveToSql(query);
    int strokeSettingsId = mStrokeSettings->prp_saveToSql(query);
    if(!query->exec(
            QString(
            "INSERT INTO pathbox (fillgradientpointsid, "
            "strokegradientpointsid, "
            "boundingboxid, fillsettingsid, strokesettingsid) "
            "VALUES (%1, %2, %3, %4, %5)").
            arg(fillPts).
            arg(strokePts).
            arg(boundingBoxId).
            arg(fillSettingsId).
            arg(strokeSettingsId) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }

    return boundingBoxId;
}

void PathBox::prp_loadFromSql(const int &boundingBoxId) {
    BoundingBox::prp_loadFromSql(boundingBoxId);
    QSqlQuery query;
    QString queryStr = "SELECT * FROM pathbox WHERE boundingboxid = " +
            QString::number(boundingBoxId);
    if(query.exec(queryStr) ) {
        query.next();
        int idfillgradientpointsid = query.record().indexOf("fillgradientpointsid");
        int idstrokegradientpointsid = query.record().indexOf("strokegradientpointsid");
        int idfillsettingsid = query.record().indexOf("fillsettingsid");
        int idstrokesettingsid = query.record().indexOf("strokesettingsid");

        int fillGradientPointsId = query.value(idfillgradientpointsid).toInt();
        int strokeGradientPointsId = query.value(idstrokegradientpointsid).toInt();
        int fillSettingsId = query.value(idfillsettingsid).toInt();
        int strokeSettingsId = query.value(idstrokesettingsid).toInt();


        mFillGradientPoints->prp_loadFromSql(fillGradientPointsId);
        mStrokeGradientPoints->prp_loadFromSql(strokeGradientPointsId);

        mFillSettings->prp_loadFromSql(fillSettingsId);
        mStrokeSettings->prp_loadFromSql(strokeSettingsId);
    } else {
        qDebug() << "Could not load vectorpath with id " << boundingBoxId;
    }
}

void PathBox::updatePathIfNeeded() {
    if(mPathUpdateNeeded) {
        updatePath();
        if(!prp_hasKeys() &&
           !mPivotChanged ) centerPivotPosition();
        mPathUpdateNeeded = false;
        mOutlinePathUpdateNeeded = false;
    }
}

void PathBox::resetStrokeGradientPointsPos(bool finish) {
    mStrokeGradientPoints->prp_setRecording(false);
    mStrokeGradientPoints->setPositions(mRelBoundingRect.topLeft(),
                                        mRelBoundingRect.bottomRight(),
                                        finish);
}

void PathBox::resetFillGradientPointsPos(bool finish) {
    mFillGradientPoints->prp_setRecording(false);
    mFillGradientPoints->setPositions(mRelBoundingRect.topLeft(),
                                      mRelBoundingRect.bottomRight(),
                                      finish);
}

void PathBox::setStrokeCapStyle(Qt::PenCapStyle capStyle) {
    mStrokeSettings->setCapStyle(capStyle);
    clearAllCache();
    scheduleOutlinePathUpdate();
}

void PathBox::setStrokeJoinStyle(Qt::PenJoinStyle joinStyle) {
    mStrokeSettings->setJoinStyle(joinStyle);
    clearAllCache();
    scheduleOutlinePathUpdate();
}

void PathBox::setStrokeWidth(qreal strokeWidth, bool finish) {
    mStrokeSettings->setCurrentStrokeWidth(strokeWidth);
    if(finish) {
        mStrokeSettings->getStrokeWidthAnimator()->prp_finishTransform();
    }
    //scheduleOutlinePathUpdate();
}

void PathBox::setOutlineCompositionMode(QPainter::CompositionMode compositionMode) {
    mStrokeSettings->setOutlineCompositionMode(compositionMode);
    clearAllCache();
    scheduleSoftUpdate();
}

void PathBox::startSelectedStrokeWidthTransform() {
    mStrokeSettings->getStrokeWidthAnimator()->prp_startTransform();
}

void PathBox::startSelectedStrokeColorTransform() {
    mStrokeSettings->getColorAnimator()->prp_startTransform();
}

void PathBox::startSelectedFillColorTransform() {
    mFillSettings->getColorAnimator()->prp_startTransform();
}

void PathBox::preUpdatePixmapsUpdates() {
    updateEffectsMarginIfNeeded();
    //    updatePathIfNeeded();
    //    updateOutlinePathIfNeeded();
    //updateBoundingRect();
}

void PathBox::duplicateGradientPointsFrom(GradientPoints *fillGradientPoints,
                                          GradientPoints *strokeGradientPoints) {
    if(fillGradientPoints != NULL) {
        fillGradientPoints->prp_makeDuplicate(mFillGradientPoints.data());
    }
    if(strokeGradientPoints != NULL) {
        strokeGradientPoints->prp_makeDuplicate(mStrokeGradientPoints.data());
    }
}

void PathBox::duplicatePaintSettingsFrom(PaintSettings *fillSettings,
                                         StrokeSettings *strokeSettings) {
    if(fillSettings == NULL) {
        mFillSettings->setPaintType(NOPAINT);
    } else {
        fillSettings->prp_makeDuplicate(mFillSettings.data());
    }
    if(strokeSettings == NULL) {
        mStrokeSettings->setPaintType(NOPAINT);
    } else {
        strokeSettings->prp_makeDuplicate(mStrokeSettings.data());
    }
}

void PathBox::prp_makeDuplicate(Property *targetBox) {
    PathBox *pathBoxTarget = ((PathBox*)targetBox);
    pathBoxTarget->duplicatePaintSettingsFrom(mFillSettings.data(),
                                              mStrokeSettings.data());
    pathBoxTarget->duplicateGradientPointsFrom(mFillGradientPoints.data(),
                                               mStrokeGradientPoints.data());
}

void PathBox::drawHovered(QPainter *p) {
    drawHoveredPath(p, mPath);
}

void PathBox::applyPaintSetting(const PaintSetting &setting) {
    setting.apply(this);
    replaceCurrentFrameCache();
    scheduleSoftUpdate();
}

void PathBox::setFillColorMode(const ColorMode &colorMode) {
    mFillSettings->getColorAnimator()->setColorMode(colorMode);
}

void PathBox::setStrokeColorMode(const ColorMode &colorMode) {
    mFillSettings->getColorAnimator()->setColorMode(colorMode);
}

void PathBox::schedulePathUpdate() {
    scheduleSoftUpdate();
    if(mPathUpdateNeeded) {
        return;
    }
    addUpdateScheduler(new PathUpdateScheduler(this));

    mPathUpdateNeeded = true;
    mOutlinePathUpdateNeeded = false;
}

void PathBox::scheduleOutlinePathUpdate() {
    scheduleSoftUpdate();
    if(mOutlinePathUpdateNeeded || mPathUpdateNeeded) {
        return;
    }
    addUpdateScheduler(new PathUpdateScheduler(this));

    mOutlinePathUpdateNeeded = true;
}

void PathBox::updateOutlinePathIfNeeded() {
    if(mOutlinePathUpdateNeeded) {
        updateOutlinePath();
        mOutlinePathUpdateNeeded = false;
    }
}

VectorPath *PathBox::objectToPath() {
    VectorPath *newPath = new VectorPath(mParent.data());
    newPath->loadPathFromQPainterPath(mPath);
    newPath->duplicateTransformAnimatorFrom(mTransformAnimator.data());
    newPath->duplicatePaintSettingsFrom(mFillSettings.data(),
                                        mStrokeSettings.data());
    newPath->duplicateGradientPointsFrom(mFillGradientPoints.data(),
                                         mStrokeGradientPoints.data());
    return newPath;
}

VectorPath *PathBox::strokeToPath() {
    if(mOutlinePath.isEmpty()) return NULL;
    VectorPath *newPath = new VectorPath(mParent.data());
    newPath->loadPathFromQPainterPath(mOutlinePath);
    newPath->duplicateTransformAnimatorFrom(mTransformAnimator.data());
    newPath->duplicatePaintSettingsFrom(mStrokeSettings.data(),
                                        NULL);
    newPath->duplicateGradientPointsFrom(mStrokeGradientPoints.data(),
                                         NULL);
    return newPath;
}

const QPainterPath &PathBox::getRelativePath() const { return mPath; }

void PathBox::updateOutlinePath() {
    if(mStrokeSettings->nonZeroLineWidth()) {
        QPainterPathStroker stroker;
        mStrokeSettings->setStrokerSettings(&stroker);
        mOutlinePath = stroker.createStroke(mPath);
    } else {
        mOutlinePath = QPainterPath();
    }
    updateWholePath();
}

void PathBox::updateWholePath() {
    mWholePath = QPainterPath();
    if(mStrokeSettings->getPaintType() != NOPAINT) {
        mWholePath += mOutlinePath;
    }
    if(mFillSettings->getPaintType() != NOPAINT ||
            mStrokeSettings->getPaintType() == NOPAINT) {
        mWholePath += mPath;
    }
    updateRelBoundingRect();
}

void PathBox::updateFillDrawGradient() {
    if(mFillSettings->getPaintType() == GRADIENTPAINT) {
        mFillSettingsGradientUpdateNeeded = true;
        Gradient *gradient = mFillSettings->getGradient();

        mFillGradientPoints->setColors(gradient->getFirstQGradientStopQColor(),
                                       gradient->getLastQGradientStopQColor());
        if(!mFillGradientPoints->enabled) {
            mFillGradientPoints->enable();
        }
    } else if(mFillGradientPoints->enabled) {
        mFillGradientPoints->disable();
    }
}

void PathBox::updatePath() {}

void PathBox::updateStrokeDrawGradient() {
    if(mStrokeSettings->getPaintType() == GRADIENTPAINT) {
        mStrokeSettingsGradientUpdateNeeded = true;
        Gradient *gradient = mStrokeSettings->getGradient();

        mStrokeGradientPoints->setColors(gradient->getFirstQGradientStopQColor(),
                                         gradient->getLastQGradientStopQColor() );

        if(!mStrokeGradientPoints->enabled) {
            mStrokeGradientPoints->enable();
        }
    } else if(mStrokeGradientPoints->enabled) {
        mStrokeGradientPoints->disable();
    }
}

void PathBox::updateDrawGradients() {
    updateFillDrawGradient();
    updateStrokeDrawGradient();
}

void PathBox::updateRelBoundingRect() {
    mRelBoundingRect = mWholePath.boundingRect();

    BoundingBox::updateRelBoundingRect();
}

void PathBox::setUpdateVars() {
    mUpdateFillSettings.paintColor = mFillSettings->getCurrentColor().qcol;
    mUpdateFillSettings.paintType = mFillSettings->getPaintType();
    if(mFillSettingsGradientUpdateNeeded) {
        mFillSettingsGradientUpdateNeeded = false;
        Gradient *grad = mFillSettings->getGradient();
        if(grad != NULL) {
            mUpdateFillSettings.updateGradient(
                        grad->getQGradientStops(),
                        mFillGradientPoints->getStartPoint(),
                        mFillGradientPoints->getEndPoint());
        }
    }
    mUpdateStrokeSettings.paintColor = mStrokeSettings->getCurrentColor().qcol;
    mUpdateStrokeSettings.paintType = mStrokeSettings->getPaintType();
    if(mStrokeSettingsGradientUpdateNeeded) {
        mStrokeSettingsGradientUpdateNeeded = false;
        Gradient *grad = mStrokeSettings->getGradient();
        if(grad != NULL) {
            mUpdateStrokeSettings.updateGradient(
                        grad->getQGradientStops(),
                        mStrokeGradientPoints->getStartPoint(),
                        mStrokeGradientPoints->getEndPoint());
        }
    }
    updatePathIfNeeded();
    updateOutlinePathIfNeeded();
    mUpdatePath = mPath;
    mUpdateOutlinePath = mOutlinePath;
    BoundingBox::setUpdateVars();
}

void PathBox::draw(QPainter *p) {
    p->save();

    p->setPen(Qt::NoPen);
    if(!mUpdatePath.isEmpty()) {
        mUpdateFillSettings.applyPainterSettings(p);

        p->drawPath(mUpdatePath);
    }
    if(!mUpdateOutlinePath.isEmpty()) {
        mUpdateStrokeSettings.applyPainterSettings(p);

        p->drawPath(mUpdateOutlinePath);
    }

    p->restore();
}

bool PathBox::relPointInsidePath(QPointF relPos) {
    if(mRelBoundingRectPath.contains(relPos) ) {
        return mWholePath.contains(relPos);
    } else {
        return false;
    }
}

void PathBox::setOutlineAffectedByScale(bool bT) {
    mOutlineAffectedByScale = bT;
    scheduleOutlinePathUpdate();
}

PaintSettings *PathBox::getFillSettings() {
    return mFillSettings.data();
}

StrokeSettings *PathBox::getStrokeSettings() {
    return mStrokeSettings.data();
}
