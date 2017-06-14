#include "Boxes/pathbox.h"
#include "updatescheduler.h"
#include "mainwindow.h"
#include "gradientpoint.h"
#include "Animators/animatorupdater.h"
#include "gradientpoints.h"
#include "skiaincludes.h"
#include "PathEffects/patheffect.h"
#include "PathEffects/patheffectanimators.h"

PathBox::PathBox(BoxesGroup *parent,
                 const BoundingBoxType &type) :
    BoundingBox(parent, type) {
    mPathEffectsAnimators =
            (new PathEffectAnimators())->ref<PathEffectAnimators>();
    mPathEffectsAnimators->prp_setName("path effects");
    mPathEffectsAnimators->prp_setBlockedUpdater(
                new PathPointUpdater(this));

    mOutlinePathEffectsAnimators =
            (new PathEffectAnimators())->ref<PathEffectAnimators>();
    mOutlinePathEffectsAnimators->prp_setName("outline effects");
    mOutlinePathEffectsAnimators->prp_setBlockedUpdater(
                new PathPointUpdater(this));

//    mPathEffectsAnimators->prp_setName("path effects");
//    mPathEffectsAnimators->prp_setBlockedUpdater(
//                new PixmapEffectUpdater(this));

//    mOutlinePathEffectsAnimators->prp_setName("outline path effects");
//    mOutlinePathEffectsAnimators->prp_setBlockedUpdater(
//                new PixmapEffectUpdater(this));

    mStrokeGradientPoints =
            (new GradientPoints)->ref<GradientPoints>();
    mFillGradientPoints =
            (new GradientPoints)->ref<GradientPoints>();
    mFillSettings->setTargetPathBox(this);
    mStrokeSettings->setTargetPathBox(this);

    ca_addChildAnimator(mFillSettings.data());
    ca_addChildAnimator(mStrokeSettings.data());

    mFillGradientPoints->initialize(this);
    mStrokeGradientPoints->initialize(this);

    mFillGradientPoints->prp_setBlockedUpdater(
                new GradientPointsUpdater(true, this));
    mStrokeGradientPoints->prp_setBlockedUpdater(
                new GradientPointsUpdater(false, this));

    mFillSettings->setGradientPoints(mFillGradientPoints.data());
    mStrokeSettings->setGradientPoints(mStrokeGradientPoints.data());

    mStrokeSettings->setLineWidthUpdaterTarget(this);
    mFillSettings->setPaintPathTarget(this);

    schedulePathUpdate();
}

PathBox::~PathBox() {
    if(mFillSettings->getGradient() != NULL) {
        mFillSettings->getGradient()->removePath(this);
    }
    if(mStrokeSettings->getGradient() != NULL) {
        mStrokeSettings->getGradient()->removePath(this);
    }
}

void PathBox::updateEffectsMargin() {
    mEffectsMargin = mEffectsAnimators->getEffectsMargin();/* +
                        mPathEffectsAnimators->getEffectsMargin() +
                            mOutlinePathEffectsAnimators->getEffectsMargin();*/
}

#include <QSqlError>
int PathBox::saveToSql(QSqlQuery *query, const int &parentId) {
    int boundingBoxId = BoundingBox::saveToSql(query, parentId);

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
        int idfillgradientpointsid =
                query.record().indexOf("fillgradientpointsid");
        int idstrokegradientpointsid =
                query.record().indexOf("strokegradientpointsid");
        int idfillsettingsid =
                query.record().indexOf("fillsettingsid");
        int idstrokesettingsid =
                query.record().indexOf("strokesettingsid");

        int fillGradientPointsId =
                query.value(idfillgradientpointsid).toInt();
        int strokeGradientPointsId =
                query.value(idstrokegradientpointsid).toInt();
        int fillSettingsId =
                query.value(idfillsettingsid).toInt();
        int strokeSettingsId =
                query.value(idstrokesettingsid).toInt();


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
        mPathEffectsAnimators->filterPath(&mPathSk);
        updateOutlinePathSk();
        if(!prp_hasKeys() &&
           !mPivotChanged ) {
            centerPivotPosition();
        }
        mPathUpdateNeeded = false;
        mOutlinePathUpdateNeeded = false;
    }
}

MovablePoint *PathBox::getPointAtAbsPos(const QPointF &absPtPos,
                                     const CanvasMode &currentCanvasMode,
                                     const qreal &canvasScaleInv) {
    MovablePoint *pointToReturn = NULL;
    if(currentCanvasMode == MOVE_POINT) {
        pointToReturn = mStrokeGradientPoints->qra_getPointAt(absPtPos,
                                                              canvasScaleInv);
        if(pointToReturn == NULL) {
            pointToReturn = mFillGradientPoints->qra_getPointAt(absPtPos,
                                                                canvasScaleInv);
        }
    } else if(currentCanvasMode == MOVE_PATH) {
        MovablePoint *pivotMovable = mTransformAnimator->getPivotMovablePoint();
        if(pivotMovable->isPointAtAbsPos(absPtPos, canvasScaleInv)) {
            return pivotMovable;
        }
    }
    return pointToReturn;
}

void PathBox::addPathEffect(PathEffect *effect) {
    //effect->setUpdater(new PixmapEffectUpdater(this));

    if(!mPathEffectsAnimators->hasChildAnimators()) {
        ca_addChildAnimator(mPathEffectsAnimators.data());
    }
    mPathEffectsAnimators->ca_addChildAnimator(effect);
    //effect->setParentEffectAnimators(mEffectsAnimators.data());

    //scheduleEffectsMarginUpdate();
    clearAllCache();
}

void PathBox::addOutlinePathEffect(PathEffect *effect) {
    //effect->setUpdater(new PixmapEffectUpdater(this));

    if(!mOutlinePathEffectsAnimators->hasChildAnimators()) {
        ca_addChildAnimator(mOutlinePathEffectsAnimators.data());
    }
    mOutlinePathEffectsAnimators->ca_addChildAnimator(effect);
    //effect->setParentEffectAnimators(mEffectsAnimators.data());

    //scheduleEffectsMarginUpdate();
    clearAllCache();
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

void PathBox::setOutlineCompositionMode(
        const QPainter::CompositionMode &compositionMode) {
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
        fillGradientPoints->makeDuplicate(mFillGradientPoints.data());
    }
    if(strokeGradientPoints != NULL) {
        strokeGradientPoints->makeDuplicate(mStrokeGradientPoints.data());
    }
}

void PathBox::duplicatePaintSettingsFrom(PaintSettings *fillSettings,
                                         StrokeSettings *strokeSettings) {
    if(fillSettings == NULL) {
        mFillSettings->setPaintType(NOPAINT);
    } else {
        fillSettings->makeDuplicate(mFillSettings.data());
    }
    if(strokeSettings == NULL) {
        mStrokeSettings->setPaintType(NOPAINT);
    } else {
        strokeSettings->makeDuplicate(mStrokeSettings.data());
    }
}

void PathBox::makeDuplicate(Property *targetBox) {
    BoundingBox::makeDuplicate(targetBox);

    PathBox *pathBoxTarget = ((PathBox*)targetBox);

    int effectsCount = mPathEffectsAnimators->ca_getNumberOfChildren();
    for(int i = 0; i < effectsCount; i++) {
        pathBoxTarget->addPathEffect(
                (PathEffect*)((PathEffect*)mPathEffectsAnimators->
                    ca_getChildAt(i))->makeDuplicate() );
    }

    int outlineEffectsCount =
            mOutlinePathEffectsAnimators->ca_getNumberOfChildren();
    for(int i = 0; i < outlineEffectsCount; i++) {
        pathBoxTarget->addOutlinePathEffect(
                (PathEffect*)((PathEffect*)mOutlinePathEffectsAnimators->
                    ca_getChildAt(i))->makeDuplicate() );
    }

    pathBoxTarget->duplicatePaintSettingsFrom(mFillSettings.data(),
                                              mStrokeSettings.data());
    pathBoxTarget->duplicateGradientPointsFrom(mFillGradientPoints.data(),
                                               mStrokeGradientPoints.data());
}

void PathBox::drawHoveredSk(SkCanvas *canvas,
                            const SkScalar &invScale) {
    drawHoveredPathSk(canvas, mPathSk, invScale);
}

void PathBox::applyPaintSetting(const PaintSetting &setting) {
    setting.apply(this);
    replaceCurrentFrameCache();
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
        updateOutlinePathSk();
        mOutlinePathUpdateNeeded = false;
    }
}

VectorPath *PathBox::objectToPath() {
    VectorPath *newPath = new VectorPath(mParent.data());
    newPath->loadPathFromSkPath(mPathSk);
    newPath->duplicateTransformAnimatorFrom(mTransformAnimator.data());
    newPath->duplicatePaintSettingsFrom(mFillSettings.data(),
                                        mStrokeSettings.data());
    newPath->duplicateGradientPointsFrom(mFillGradientPoints.data(),
                                         mStrokeGradientPoints.data());
    return newPath;
}

VectorPath *PathBox::strokeToPath() {
    if(mOutlinePathSk.isEmpty()) return NULL;
    VectorPath *newPath = new VectorPath(mParent.data());
    newPath->loadPathFromSkPath(mOutlinePathSk);
    newPath->duplicateTransformAnimatorFrom(mTransformAnimator.data());
    newPath->duplicatePaintSettingsFrom(mStrokeSettings.data(),
                                        NULL);
    newPath->duplicateGradientPointsFrom(mStrokeGradientPoints.data(),
                                         NULL);
    return newPath;
}

const SkPath &PathBox::getRelativePath() const { return mPathSk; }

void PathBox::updateOutlinePathSk() {
    if(mStrokeSettings->nonZeroLineWidth()) {
        SkStroke strokerSk;
        mStrokeSettings->setStrokerSettingsSk(&strokerSk);
        mOutlinePathSk = SkPath();
        strokerSk.strokePath(mPathSk, &mOutlinePathSk);
    } else {
        mOutlinePathSk = SkPath();
    }
    mOutlinePathEffectsAnimators->filterPath(&mOutlinePathSk);
    updateWholePathSk();
}

void PathBox::updateWholePathSk() {
    mWholePathSk = SkPath();
    if(mStrokeSettings->getPaintType() != NOPAINT) {
        mWholePathSk.addPath(mOutlinePathSk);
    }
    if(mFillSettings->getPaintType() != NOPAINT ||
            mStrokeSettings->getPaintType() == NOPAINT) {
        mWholePathSk.addPath(mPathSk);
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
    mRelBoundingRectSk = mWholePathSk.computeTightBounds();//mWholePath.boundingRect();
    mRelBoundingRect = SkRectToQRectF(mRelBoundingRectSk);

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
    mUpdatePathSk = mPathSk;
    mUpdateOutlinePathSk = mOutlinePathSk;
    BoundingBox::setUpdateVars();
}

void PathBox::drawSk(SkCanvas *canvas) {
    canvas->save();

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kFill_Style);
    //mPathEffectsAnimators->applyEffectsSk(&paint);
    if(!mUpdatePathSk.isEmpty()) {
        mUpdateFillSettings.applyPainterSettingsSk(&paint);

        canvas->drawPath(mUpdatePathSk, paint);
    }
    if(!mUpdateOutlinePathSk.isEmpty()) {
        paint.setShader(NULL);
        mUpdateStrokeSettings.applyPainterSettingsSk(&paint);

        canvas->drawPath(mUpdateOutlinePathSk, paint);
    }

    canvas->restore();
}

bool PathBox::relPointInsidePath(const QPointF &relPos) {
    if(mSkRelBoundingRectPath.contains(relPos.x(), relPos.y()) ) {
        if(mPathSk.contains(relPos.x(), relPos.y())) {
            return true;
        }
        return mOutlinePathSk.contains(relPos.x(), relPos.y());
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
