#include "Boxes/pathbox.h"
#include "updatescheduler.h"
#include "mainwindow.h"

PathBox::PathBox(BoxesGroup *parent, BoundingBoxType type) :
    BoundingBox(parent, type)
{
    mFillPaintSettings.setTargetPathBox(this);
    mStrokeSettings.setTargetPathBox(this);

    addActiveAnimator(&mFillPaintSettings);
    addActiveAnimator(&mStrokeSettings);

    mFillPaintSettings.blockPointer();
    mStrokeSettings.blockPointer();

    mFillGradientPoints.initialize(this);
    mFillGradientPoints.blockPointer();
    mStrokeGradientPoints.initialize(this);
    mStrokeGradientPoints.blockPointer();

    mFillGradientPoints.setUpdater(new GradientPointsUpdater(true, this));
    mFillGradientPoints.blockUpdater();
    mStrokeGradientPoints.setUpdater(new GradientPointsUpdater(false, this));
    mStrokeGradientPoints.blockUpdater();

    mFillPaintSettings.setGradientPoints(&mFillGradientPoints);
    mStrokeSettings.setGradientPoints(&mStrokeGradientPoints);

    mStrokeSettings.setLineWidthUpdaterTarget(this);
    mFillPaintSettings.setPaintPathTarget(this);
}

PathBox::~PathBox()
{
    if(mFillPaintSettings.getGradient() != NULL) {
        mFillPaintSettings.getGradient()->removePath(this);
    }
    if(mStrokeSettings.getGradient() != NULL) {
        mStrokeSettings.getGradient()->removePath(this);
    }
}

#include <QSqlError>
int PathBox::saveToSql(QSqlQuery *query, int parentId)
{
    int boundingBoxId = BoundingBox::saveToSql(query, parentId);

    int fillStartPt = mFillGradientPoints.startPoint->saveToSql(query);
    int fillEndPt = mFillGradientPoints.endPoint->saveToSql(query);
    int strokeStartPt = mStrokeGradientPoints.startPoint->saveToSql(query);
    int strokeEndPt = mStrokeGradientPoints.endPoint->saveToSql(query);

    int fillSettingsId = mFillPaintSettings.saveToSql(query);
    int strokeSettingsId = mStrokeSettings.saveToSql(query);
    if(!query->exec(
            QString(
            "INSERT INTO pathbox (fillgradientstartid, fillgradientendid, "
            "strokegradientstartid, strokegradientendid, "
            "boundingboxid, fillsettingsid, strokesettingsid) "
            "VALUES (%1, %2, %3, %4, %5, %6, %7)").
            arg(fillStartPt).
            arg(fillEndPt).
            arg(strokeStartPt).
            arg(strokeEndPt).
            arg(boundingBoxId).
            arg(fillSettingsId).
            arg(strokeSettingsId) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }

    return boundingBoxId;
}

void PathBox::loadFromSql(int boundingBoxId) {
    BoundingBox::loadFromSql(boundingBoxId);
    QSqlQuery query;
    QString queryStr = "SELECT * FROM pathbox WHERE boundingboxid = " +
            QString::number(boundingBoxId);
    if(query.exec(queryStr) ) {
        query.next();
        int idfillgradientstartid = query.record().indexOf("fillgradientstartid");
        int idfillgradientendid = query.record().indexOf("fillgradientendid");
        int idstrokegradientstartid = query.record().indexOf("strokegradientstartid");
        int idstrokegradientendid = query.record().indexOf("strokegradientendid");
        int idfillsettingsid = query.record().indexOf("fillsettingsid");
        int idstrokesettingsid = query.record().indexOf("strokesettingsid");

        int fillGradientStartId = query.value(idfillgradientstartid).toInt();
        int fillGradientEndId = query.value(idfillgradientendid).toInt();
        int strokeGradientStartId = query.value(idstrokegradientstartid).toInt();
        int strokeGradientEndId = query.value(idstrokegradientendid).toInt();
        int fillSettingsId = query.value(idfillsettingsid).toInt();
        int strokeSettingsId = query.value(idstrokesettingsid).toInt();


        mFillGradientPoints.loadFromSql(fillGradientStartId,
                                       fillGradientEndId);
        mStrokeGradientPoints.loadFromSql(strokeGradientStartId,
                                         strokeGradientEndId);

        GradientWidget *gradientWidget =
                mMainWindow->getFillStrokeSettings()->getGradientWidget();

        mFillPaintSettings.loadFromSql(fillSettingsId, gradientWidget);
        mStrokeSettings.loadFromSql(strokeSettingsId, gradientWidget);
    } else {
        qDebug() << "Could not load vectorpath with id " << boundingBoxId;
    }
}

void PathBox::updatePathIfNeeded()
{
    if(mUpdatePathUpdateNeeded) {
        updatePath();
        if(!mAnimatorsCollection.hasKeys() &&
           !mPivotChanged ) centerPivotPosition();
    }
}

void PathBox::preUpdatePixmapsUpdates() {
    updateEffectsMarginIfNeeded();
    updatePathIfNeeded();
    updateOutlinePathIfNeeded();
    updateBoundingRect();
}

void PathBox::duplicateGradientPointsFrom(GradientPoints *fillGradientPoints,
                                          GradientPoints *strokeGradientPoints) {
    fillGradientPoints->makeDuplicate(&mFillGradientPoints);
    strokeGradientPoints->makeDuplicate(&mStrokeGradientPoints);
}

void PathBox::duplicatePaintSettingsFrom(PaintSettings *fillSettings,
                                         StrokeSettings *strokeSettings) {
    fillSettings->makeDuplicate(&mFillPaintSettings);
    strokeSettings->makeDuplicate(&mStrokeSettings);
}

void PathBox::makeDuplicate(BoundingBox *targetBox) {
    PathBox *pathBoxTarget = ((PathBox*)targetBox);
    pathBoxTarget->duplicatePaintSettingsFrom(&mFillPaintSettings,
                                              &mStrokeSettings);
    pathBoxTarget->duplicateGradientPointsFrom(&mFillGradientPoints,
                                               &mStrokeGradientPoints);
}

void PathBox::schedulePathUpdate(const bool &replaceCache) {
    scheduleAwaitUpdate(replaceCache);
    if(mPathUpdateNeeded) {
        return;
    }

    mPathUpdateNeeded = true;
    mOutlinePathUpdateNeeded = false;
}

void PathBox::scheduleOutlinePathUpdate(const bool &replaceCache)
{
    scheduleAwaitUpdate(replaceCache);
    if(mOutlinePathUpdateNeeded || mPathUpdateNeeded) {
        return;
    }

    mOutlinePathUpdateNeeded = true;
}

void PathBox::updateOutlinePathIfNeeded() {
    if(mUpdateOutlinePathUpdateNeeded) {
        updateOutlinePath();
    }
}

void PathBox::updateAfterCombinedTransformationChanged() {
}

void PathBox::copyStrokeAndFillSettingsTo(PathBox *target) {

    target->setFillPaintType(mFillPaintSettings.getPaintType(),
                             mFillPaintSettings.getCurrentColor(),
                             mFillPaintSettings.getGradient());
    target->setStrokePaintType(mStrokeSettings.getPaintType(),
                               mStrokeSettings.getCurrentColor(),
                               mStrokeSettings.getGradient());
    target->setStrokeCapStyle(mStrokeSettings.getCapStyle());
    target->setStrokeJoinStyle(mStrokeSettings.getJoinStyle());
    target->setStrokeWidth(mStrokeSettings.getCurrentStrokeWidth(), false);
}

VectorPath *PathBox::objectToPath() {
    VectorPath *newPath = new VectorPath(mParent);
    newPath->loadPathFromQPainterPath(mPath);
    copyTransformationTo(newPath);
    copyStrokeAndFillSettingsTo(newPath);
    return newPath;
}

void PathBox::updateOutlinePath() {
    if(mStrokeSettings.nonZeroLineWidth()) {
        mStrokeSettings.setStrokerSettings(&mPathStroker);
        if(mOutlineAffectedByScale) {
            mOutlinePath = mPathStroker.createStroke(mPath);
        } else {
            QPainterPathStroker stroker;
            stroker.setCapStyle(mPathStroker.capStyle());
            stroker.setJoinStyle(mPathStroker.joinStyle());
            stroker.setMiterLimit(mPathStroker.miterLimit());
            stroker.setWidth(mPathStroker.width()*getCurrentCanvasScale() );
            mOutlinePath = stroker.createStroke(mPath);
        }
    } else {
        mOutlinePath = QPainterPath();
    }
    updateWholePath();
}

void PathBox::updateWholePath() {
    mWholePath = QPainterPath();
    if(mStrokeSettings.getPaintType() != NOPAINT) {
        mWholePath += mOutlinePath;
    }
    if(mFillPaintSettings.getPaintType() != NOPAINT ||
            mStrokeSettings.getPaintType() == NOPAINT) {
        mWholePath += mPath;
    }
    updateBoundingRect();
}

void PathBox::updateFillDrawGradient() {
    if(mFillPaintSettings.getPaintType() == GRADIENTPAINT) {
        Gradient *gradient = mFillPaintSettings.getGradient();

        mFillGradientPoints.setColors(gradient->getFirstQGradientStopQColor(),
                                      gradient->getLastQGradientStopQColor());
        if(!mFillGradientPoints.enabled) {
            mFillGradientPoints.enable();
        }

        mDrawFillGradient.setStops(gradient->getQGradientStops());
        mDrawFillGradient.setStart(mFillGradientPoints.getStartPoint() );
        mDrawFillGradient.setFinalStop(mFillGradientPoints.getEndPoint() );

    } else if(mFillGradientPoints.enabled) {
        mFillGradientPoints.disable();
    }
}

void PathBox::updateStrokeDrawGradient() {
    if(mStrokeSettings.getPaintType() == GRADIENTPAINT) {
        Gradient *gradient = mStrokeSettings.getGradient();

        mStrokeGradientPoints.setColors(gradient->getFirstQGradientStopQColor(),
                                      gradient->getLastQGradientStopQColor() );

        if(!mStrokeGradientPoints.enabled) {
            mStrokeGradientPoints.enable();
        }
        mDrawStrokeGradient.setStops(gradient->getQGradientStops());
        mDrawStrokeGradient.setStart(mStrokeGradientPoints.getStartPoint() );
        mDrawStrokeGradient.setFinalStop(mStrokeGradientPoints.getEndPoint() );
    } else if(mStrokeGradientPoints.enabled) {
        mStrokeGradientPoints.disable();
    }
}

void PathBox::updateDrawGradients() {
    updateFillDrawGradient();
    updateStrokeDrawGradient();
}

void PathBox::updateBoundingRect() {
    mRelBoundingRect = mWholePath.boundingRect();
    qreal effectsMargin = mEffectsMargin*mUpdateCanvasTransform.m11();
    mPixBoundingRect = mUpdateTransform.mapRect(mRelBoundingRect).
                        adjusted(-effectsMargin, -effectsMargin,
                                 effectsMargin, effectsMargin);
    BoundingBox::updateBoundingRect();
}

void PathBox::afterSuccessfulUpdate()
{
    mUpdatePathUpdateNeeded = false;
    mUpdateOutlinePathUpdateNeeded = false;
}

void PathBox::updateUpdateTransform()
{
    mUpdatePathUpdateNeeded = mPathUpdateNeeded ||
            mUpdatePathUpdateNeeded;
    mUpdateOutlinePathUpdateNeeded = mOutlinePathUpdateNeeded ||
            mUpdateOutlinePathUpdateNeeded;
    mPathUpdateNeeded = false;
    mOutlinePathUpdateNeeded = false;
    BoundingBox::updateUpdateTransform();
}

void PathBox::draw(QPainter *p)
{
    if(mVisible) {
        p->save();

        p->setPen(Qt::NoPen);
        if(mFillPaintSettings.getPaintType() == GRADIENTPAINT) {
            p->setBrush(mDrawFillGradient);
        } else if(mFillPaintSettings.getPaintType() == FLATPAINT) {
            p->setBrush(mFillPaintSettings.getCurrentColor().qcol);
        } else{
            p->setBrush(Qt::NoBrush);
        }
        p->drawPath(mPath);
        if(mStrokeSettings.getPaintType() == GRADIENTPAINT) {
            p->setBrush(mDrawStrokeGradient);
        } else if(mStrokeSettings.getPaintType() == FLATPAINT) {
            p->setBrush(mStrokeSettings.getCurrentColor().qcol);
        } else{
            p->setBrush(Qt::NoBrush);
        }

        p->setCompositionMode(mStrokeSettings.getOutlineCompositionMode());
        p->drawPath(mOutlinePath);

        p->restore();
    }
}

bool PathBox::relPointInsidePath(QPointF relPos) {
    if(mRelBoundingRectPath.contains(relPos) ) {
        return mWholePath.contains(relPos);
    } else {
        return false;
    }
}

void PathBox::updateAfterFrameChanged(int currentFrame)
{
    mFillPaintSettings.setFrame(currentFrame);
    mStrokeSettings.setFrame(currentFrame);
    BoundingBox::updateAfterFrameChanged(currentFrame);
}

void PathBox::setOutlineAffectedByScale(bool bT)
{
    mOutlineAffectedByScale = bT;
    scheduleOutlinePathUpdate();
}

PaintSettings *PathBox::getFillSettings() {
    return &mFillPaintSettings;
}

StrokeSettings *PathBox::getStrokeSettings() {
    return &mStrokeSettings;
}
