#include "Boxes/pathbox.h"
#include "updatescheduler.h"
#include "mainwindow.h"

PathBox::PathBox(BoxesGroup *parent, BoundingBoxType type) :
    BoundingBox(parent, type) {
    mFillPaintSettings.setTargetPathBox(this);
    mStrokeSettings.setTargetPathBox(this);

    ca_addChildAnimator(&mFillPaintSettings);
    ca_addChildAnimator(&mStrokeSettings);

    mFillPaintSettings.blockPointer();
    mStrokeSettings.blockPointer();

    mFillGradientPoints.initialize(this);
    mFillGradientPoints.blockPointer();
    mStrokeGradientPoints.initialize(this);
    mStrokeGradientPoints.blockPointer();

    mFillGradientPoints.prp_setUpdater(new GradientPointsUpdater(true, this));
    mFillGradientPoints.prp_blockUpdater();
    mStrokeGradientPoints.prp_setUpdater(new GradientPointsUpdater(false, this));
    mStrokeGradientPoints.prp_blockUpdater();

    mFillPaintSettings.setGradientPoints(&mFillGradientPoints);
    mStrokeSettings.setGradientPoints(&mStrokeGradientPoints);

    mStrokeSettings.setLineWidthUpdaterTarget(this);
    mFillPaintSettings.setPaintPathTarget(this);

    schedulePathUpdate();
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
int PathBox::prp_saveToSql(QSqlQuery *query, const int &parentId) {
    int boundingBoxId = BoundingBox::prp_saveToSql(query, parentId);

    int fillPts = mFillGradientPoints.startPoint->prp_saveToSql(query);
    int strokePts = mStrokeGradientPoints.startPoint->prp_saveToSql(query);

    int fillSettingsId = mFillPaintSettings.prp_saveToSql(query);
    int strokeSettingsId = mStrokeSettings.prp_saveToSql(query);
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


        mFillGradientPoints.prp_loadFromSql(fillGradientPointsId);
        mStrokeGradientPoints.prp_loadFromSql(strokeGradientPointsId);

        mFillPaintSettings.prp_loadFromSql(fillSettingsId);
        mStrokeSettings.prp_loadFromSql(strokeSettingsId);
    } else {
        qDebug() << "Could not load vectorpath with id " << boundingBoxId;
    }
}

void PathBox::updatePathIfNeeded() {
    if(mPathUpdateNeeded) {
        updatePath();
        mUpdatePath = mPath;
        mUpdateOutlinePath = mOutlinePath;
        if(!prp_hasKeys() &&
           !mPivotChanged ) centerPivotPosition();
        mPathUpdateNeeded = false;
    }
}

void PathBox::preUpdatePixmapsUpdates() {
    updateEffectsMarginIfNeeded();
//    updatePathIfNeeded();
//    updateOutlinePathIfNeeded();
    //updateBoundingRect();
}

void PathBox::duplicateGradientPointsFrom(GradientPoints *fillGradientPoints,
                                          GradientPoints *strokeGradientPoints) {
    fillGradientPoints->prp_makeDuplicate(&mFillGradientPoints);
    strokeGradientPoints->prp_makeDuplicate(&mStrokeGradientPoints);
}

void PathBox::duplicatePaintSettingsFrom(PaintSettings *fillSettings,
                                         StrokeSettings *strokeSettings) {
    fillSettings->prp_makeDuplicate(&mFillPaintSettings);
    strokeSettings->prp_makeDuplicate(&mStrokeSettings);
}

void PathBox::prp_makeDuplicate(Property *targetBox) {
    PathBox *pathBoxTarget = ((PathBox*)targetBox);
    pathBoxTarget->duplicatePaintSettingsFrom(&mFillPaintSettings,
                                              &mStrokeSettings);
    pathBoxTarget->duplicateGradientPointsFrom(&mFillGradientPoints,
                                               &mStrokeGradientPoints);
}

void PathBox::schedulePathUpdate() {
    scheduleUpdate();
    if(mPathUpdateNeeded) {
        return;
    }
    addUpdateScheduler(new PathUpdateScheduler(this));

    mPathUpdateNeeded = true;
    mOutlinePathUpdateNeeded = false;
}

void PathBox::scheduleOutlinePathUpdate()
{
    scheduleUpdate();
    if(mOutlinePathUpdateNeeded || mPathUpdateNeeded) {
        return;
    }

    mOutlinePathUpdateNeeded = true;
}

void PathBox::updateOutlinePathIfNeeded() {
    if(mOutlinePathUpdateNeeded) {
        updateOutlinePath();
        mUpdateOutlinePath = mOutlinePath;
        mOutlinePathUpdateNeeded = false;
    }
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
            stroker.setWidth(mPathStroker.width());
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
    updateRelBoundingRect();
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

void PathBox::updateRelBoundingRect() {
    mRelBoundingRect = mWholePath.boundingRect();

    BoundingBox::updateRelBoundingRect();
}

void PathBox::setUpdateVars() {
    updatePathIfNeeded();
    updateOutlinePathIfNeeded();
    BoundingBox::setUpdateVars();
}

void PathBox::draw(QPainter *p)
{
    if(shouldUpdateAndDraw()) {
        p->save();

        p->setPen(Qt::NoPen);
        if(!mUpdatePath.isEmpty()) {
            if(mFillPaintSettings.getPaintType() == GRADIENTPAINT) {
                p->setBrush(mDrawFillGradient);
            } else if(mFillPaintSettings.getPaintType() == FLATPAINT) {
                p->setBrush(mFillPaintSettings.getCurrentColor().qcol);
            } else {
                p->setBrush(Qt::NoBrush);
            }
            p->drawPath(mUpdatePath);
        }
        if(!mUpdateOutlinePath.isEmpty()) {
            if(mStrokeSettings.getPaintType() == GRADIENTPAINT) {
                p->setBrush(mDrawStrokeGradient);
            } else if(mStrokeSettings.getPaintType() == FLATPAINT) {
                p->setBrush(mStrokeSettings.getCurrentColor().qcol);
            } else{
                p->setBrush(Qt::NoBrush);
            }

            p->setCompositionMode(mStrokeSettings.getOutlineCompositionMode());
            p->drawPath(mUpdateOutlinePath);
        }

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

void PathBox::setOutlineAffectedByScale(bool bT) {
    mOutlineAffectedByScale = bT;
    scheduleOutlinePathUpdate();
}

PaintSettings *PathBox::getFillSettings() {
    return &mFillPaintSettings;
}

StrokeSettings *PathBox::getStrokeSettings() {
    return &mStrokeSettings;
}
