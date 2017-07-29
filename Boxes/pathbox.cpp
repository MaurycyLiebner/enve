#include "Boxes/pathbox.h"
#include "updatescheduler.h"
#include "mainwindow.h"
#include "gradientpoint.h"
#include "Animators/animatorupdater.h"
#include "gradientpoints.h"
#include "skiaincludes.h"
#include "PathEffects/patheffect.h"
#include "PathEffects/patheffectanimators.h"
#include "canvas.h"
#include "vectorpath.h"

PathBox::PathBox(const BoundingBoxType &type) :
    BoundingBox(type) {
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
}

PathBox::~PathBox() {
    if(mFillSettings->getGradient() != NULL) {
        mFillSettings->getGradient()->removePath(this);
    }
    if(mStrokeSettings->getGradient() != NULL) {
        mStrokeSettings->getGradient()->removePath(this);
    }
}

void PathBox::setupBoundingBoxRenderDataForRelFrame(
                            const int &relFrame,
                            BoundingBoxRenderData *data) {
    BoundingBox::setupBoundingBoxRenderDataForRelFrame(relFrame,
                                                       data);
    PathBoxRenderData *pathData = (PathBoxRenderData*)data;
    pathData->path = getPathAtRelFrame(relFrame);
    mPathEffectsAnimators->filterPathForRelFrame(relFrame, &pathData->path);
    //prp_thisRelFrameToParentRelFrame(relFrame);

    SkPath outline;
    if(mStrokeSettings->nonZeroLineWidth()) {
        SkStroke strokerSk;
        mStrokeSettings->setStrokerSettingsForRelFrameSk(relFrame, &strokerSk);
        outline = SkPath();
        strokerSk.strokePath(pathData->path, &outline);
    } else {
        outline = SkPath();
    }
    mOutlinePathEffectsAnimators->filterPathForRelFrame(relFrame, &outline);
    pathData->outlinePath = outline;
    outline.addPath(pathData->path);

    UpdatePaintSettings *fillSettings = &pathData->paintSettings;

    fillSettings->paintColor = mFillSettings->
            getColorAtRelFrame(relFrame).qcol;
    fillSettings->paintType = mFillSettings->getPaintType();
    Gradient *grad = mFillSettings->getGradient();
    if(grad != NULL) {
        fillSettings->updateGradient(
                    grad->getQGradientStopsAtAbsFrame(
                        prp_relFrameToAbsFrame(relFrame)),
                    mFillGradientPoints->getStartPointAtRelFrame(relFrame),
                    mFillGradientPoints->getEndPointAtRelFrame(relFrame));
    }

    UpdateStrokeSettings *strokeSettings = &pathData->strokeSettings;
    strokeSettings->paintColor = mStrokeSettings->
            getColorAtRelFrame(relFrame).qcol;
    strokeSettings->paintType = mStrokeSettings->getPaintType();
    grad = mStrokeSettings->getGradient();
    if(grad != NULL) {
        strokeSettings->updateGradient(
                    grad->getQGradientStopsAtAbsFrame(
                        prp_relFrameToAbsFrame(relFrame)),
                    mStrokeGradientPoints->getStartPointAtRelFrame(relFrame),
                    mStrokeGradientPoints->getEndPointAtRelFrame(relFrame));
    }
}

#include <QSqlError>
int PathBox::saveToSql(QSqlQuery *query, const int &parentId) {
    int boundingBoxId = BoundingBox::saveToSql(query, parentId);

    int fillPts = mFillGradientPoints->saveToSql(query);
    int strokePts = mStrokeGradientPoints->saveToSql(query);

    int fillSettingsId = mFillSettings->saveToSql(query);
    int strokeSettingsId = mStrokeSettings->saveToSql(query);
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

void PathBox::loadFromSql(const int &boundingBoxId) {
    BoundingBox::loadFromSql(boundingBoxId);
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


        mFillGradientPoints->loadFromSql(fillGradientPointsId);
        mStrokeGradientPoints->loadFromSql(strokeGradientPointsId);

        mFillSettings->loadFromSql(fillSettingsId);
        mStrokeSettings->loadFromSql(strokeSettingsId);
    } else {
        qDebug() << "Could not load vectorpath with id " << boundingBoxId;
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

void PathBox::drawBoundingRectSk(SkCanvas *canvas,
                                 const qreal &invScale) {
    BoundingBox::drawBoundingRectSk(canvas, invScale);
    drawAsBoundingRectSk(canvas, mPathSk, invScale, false);
}

void PathBox::addPathEffect(PathEffect *effect) {
    //effect->setUpdater(new PixmapEffectUpdater(this));

    if(!mPathEffectsAnimators->hasChildAnimators()) {
        ca_addChildAnimator(mPathEffectsAnimators.data());
    }
    mPathEffectsAnimators->ca_addChildAnimator(effect);
    //effect->setParentEffectAnimators(mEffectsAnimators.data());

    clearAllCache();
}

void PathBox::addOutlinePathEffect(PathEffect *effect) {
    //effect->setUpdater(new PixmapEffectUpdater(this));

    if(!mOutlinePathEffectsAnimators->hasChildAnimators()) {
        ca_addChildAnimator(mOutlinePathEffectsAnimators.data());
    }
    mOutlinePathEffectsAnimators->ca_addChildAnimator(effect);
    //effect->setParentEffectAnimators(mEffectsAnimators.data());

    clearAllCache();
}

void PathBox::resetStrokeGradientPointsPos(const bool &finish) {
    mStrokeGradientPoints->prp_setRecording(false);
    mStrokeGradientPoints->setPositions(mRelBoundingRect.topLeft(),
                                        mRelBoundingRect.bottomRight(),
                                        finish);
}

void PathBox::resetFillGradientPointsPos(const bool &finish) {
    mFillGradientPoints->prp_setRecording(false);
    mFillGradientPoints->setPositions(mRelBoundingRect.topLeft(),
                                      mRelBoundingRect.bottomRight(),
                                      finish);
}

void PathBox::setStrokeCapStyle(const Qt::PenCapStyle &capStyle) {
    mStrokeSettings->setCapStyle(capStyle);
    clearAllCache();
    scheduleUpdate();
}

void PathBox::setStrokeJoinStyle(const Qt::PenJoinStyle &joinStyle) {
    mStrokeSettings->setJoinStyle(joinStyle);
    clearAllCache();
    scheduleUpdate();
}

void PathBox::setStrokeWidth(const qreal &strokeWidth, const bool &finish) {
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
    scheduleUpdate();
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

VectorPath *PathBox::objectToPath() {
    VectorPath *newPath = new VectorPath();
    newPath->loadPathFromSkPath(mPathSk);
    newPath->duplicateTransformAnimatorFrom(mTransformAnimator.data());
    newPath->duplicatePaintSettingsFrom(mFillSettings.data(),
                                        mStrokeSettings.data());
    newPath->duplicateGradientPointsFrom(mFillGradientPoints.data(),
                                         mStrokeGradientPoints.data());
    mParent->addChild(newPath);
    return newPath;
}

VectorPath *PathBox::strokeToPath() {
    if(mOutlinePathSk.isEmpty()) return NULL;
    VectorPath *newPath = new VectorPath();
    newPath->loadPathFromSkPath(mOutlinePathSk);
    newPath->duplicateTransformAnimatorFrom(mTransformAnimator.data());
    newPath->duplicatePaintSettingsFrom(mStrokeSettings.data(),
                                        NULL);
    newPath->duplicateGradientPointsFrom(mStrokeGradientPoints.data(),
                                         NULL);
    mParent->addChild(newPath);
    return newPath;
}

const SkPath &PathBox::getRelativePath() const { return mPathSk; }

void PathBox::updateFillDrawGradient() {
    if(mFillSettings->getPaintType() == GRADIENTPAINT) {
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

QRectF PathBox::getRelBoundingRectAtRelFrame(const int &relFrame) {
    SkPath path = getPathAtRelFrame(relFrame);
    SkPath outline;
    if(mStrokeSettings->nonZeroLineWidth()) {
        SkStroke strokerSk;
        mStrokeSettings->setStrokerSettingsForRelFrameSk(relFrame, &strokerSk);
        outline = SkPath();
        strokerSk.strokePath(path, &outline);
    } else {
        outline = SkPath();
    }
    mOutlinePathEffectsAnimators->filterPathForRelFrame(relFrame, &outline);
    outline.addPath(path);
    return SkRectToQRectF(outline.computeTightBounds());
}

void PathBox::updateCurrentPreviewDataFromRenderData(
        BoundingBoxRenderData *renderData) {
    PathBoxRenderData *pathRenderData =
            ((PathBoxRenderData*)renderData);
    mPathSk = pathRenderData->path;
    mOutlinePathSk = pathRenderData->outlinePath;
    BoundingBox::updateCurrentPreviewDataFromRenderData(renderData);
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

void PathBox::setOutlineAffectedByScale(const bool &bT) {
    mOutlineAffectedByScale = bT;
    scheduleUpdate();
}

PaintSettings *PathBox::getFillSettings() {
    return mFillSettings.data();
}

StrokeSettings *PathBox::getStrokeSettings() {
    return mStrokeSettings.data();
}
