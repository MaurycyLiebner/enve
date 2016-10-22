#include "pathbox.h"
#include "updatescheduler.h"
#include "mainwindow.h"

PathBox::PathBox(BoxesGroup *parent, BoundingBoxType type) :
    BoundingBox(parent, type)
{
    addActiveAnimator(&mFillPaintSettings);
    addActiveAnimator(&mStrokeSettings);
    mAnimatorsCollection.addAnimator(&mFillPaintSettings);
    mAnimatorsCollection.addAnimator(&mStrokeSettings);

    mFillPaintSettings.blockPointer();
    mStrokeSettings.blockPointer();

    mFillGradientPoints.initialize(this);
    mFillGradientPoints.blockPointer();
    mStrokeGradientPoints.initialize(this);
    mStrokeGradientPoints.blockPointer();

    mFillPaintSettings.setGradientPoints(&mFillGradientPoints);
    mStrokeSettings.setGradientPoints(&mStrokeGradientPoints);

    mStrokeSettings.setLineWidthUpdaterTarget(this);
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
int PathBox::saveToSql(int parentId)
{
    int boundingBoxId = BoundingBox::saveToSql(parentId);
    QSqlQuery query;

    int fillStartPt = mFillGradientPoints.startPoint->saveToSql();
    int fillEndPt = mFillGradientPoints.endPoint->saveToSql();
    int strokeStartPt = mStrokeGradientPoints.startPoint->saveToSql();
    int strokeEndPt = mStrokeGradientPoints.endPoint->saveToSql();

    int fillSettingsId = mFillPaintSettings.saveToSql();
    int strokeSettingsId = mStrokeSettings.saveToSql();
    if(!query.exec(
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
        qDebug() << query.lastError() << endl << query.lastQuery();
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
        int idId = query.record().indexOf("id");
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

void PathBox::schedulePathUpdate()
{
    if(mPathUpdateNeeded) {
        return;
    }
    addUpdateScheduler(new PathUpdateScheduler(this));
    mPathUpdateNeeded = true;
    mMappedPathUpdateNeeded = false;
    mOutlinePathUpdateNeeded = false;
}

void PathBox::updatePathIfNeeded()
{
    if(mPathUpdateNeeded) {
        updatePath();
        if(!mAnimatorsCollection.hasKeys() && !mPivotChanged ) centerPivotPosition();
        mPathUpdateNeeded = false;
        mMappedPathUpdateNeeded = false;
        mOutlinePathUpdateNeeded = false;
    }
}

void PathBox::scheduleOutlinePathUpdate()
{
    if(mOutlinePathUpdateNeeded) {
        return;
    }
    addUpdateScheduler(new OutlineUpdateScheduler(this));
    mOutlinePathUpdateNeeded = true;
}

void PathBox::updateOutlinePathIfNeeded() {
    if(mOutlinePathUpdateNeeded) {
        updateOutlinePath();
        mOutlinePathUpdateNeeded = false;
    }
}

void PathBox::updateMappedPathIfNeeded()
{
    if(mMappedPathUpdateNeeded) {
        if(mParent != NULL) {
            updateMappedPath();
        }
        mMappedPathUpdateNeeded = false;
    }
}

void PathBox::scheduleMappedPathUpdate()
{
    if(mMappedPathUpdateNeeded || mPathUpdateNeeded || mParent == NULL) {
        return;
    }
    addUpdateScheduler(new MappedPathUpdateScheduler(this));
    mMappedPathUpdateNeeded = true;

}

void PathBox::updateAfterCombinedTransformationChanged()
{
    scheduleMappedPathUpdate();
}

void PathBox::updateOutlinePath() {
    mStrokeSettings.setStrokerSettings(&mPathStroker);
    if(mOutlineAffectedByScale) {
        mMappedOutlinePath = mCombinedTransformMatrix.map(
                                            mPathStroker.createStroke(mPath));
    } else {
        QPainterPathStroker stroker;
        stroker.setCapStyle(mPathStroker.capStyle());
        stroker.setJoinStyle(mPathStroker.joinStyle());
        stroker.setMiterLimit(mPathStroker.miterLimit());
        stroker.setWidth(mPathStroker.width()*getCurrentCanvasScale() );
        mMappedOutlinePath = stroker.createStroke(mMappedPath);
    }
    updateWholePath();
}

void PathBox::updateWholePath() {
    mMappedWhole = QPainterPath();
    if(mStrokeSettings.getPaintType() != NOPAINT) {
        mMappedWhole += mMappedOutlinePath;
    }
    if(mFillPaintSettings.getPaintType() != NOPAINT ||
            mStrokeSettings.getPaintType() == NOPAINT) {
        mMappedWhole += mMappedPath;
    }
}

void PathBox::setRenderCombinedTransform() {
    BoundingBox::setRenderCombinedTransform();
    updateMappedPath();
}

void PathBox::updateMappedPath()
{
    mMappedPath = mCombinedTransformMatrix.map(mPath);

    updateOutlinePath();
    updateDrawGradients();
}


void PathBox::updateDrawGradients()
{
    if(mFillPaintSettings.getPaintType() == GRADIENTPAINT) {
        Gradient *gradient = mFillPaintSettings.getGradient();
        if(!gradient->isInPaths(this)) {
            gradient->addPath(this);
        }
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
    if(mStrokeSettings.getPaintType() == GRADIENTPAINT) {
        Gradient *gradient = mStrokeSettings.getGradient();
        if(!gradient->isInPaths(this)) {
            gradient->addPath(this);
        }
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

QRectF PathBox::getBoundingRect()
{
    return mMappedWhole.boundingRect();
}

void PathBox::draw(QPainter *p)
{
    if(mVisible) {
        p->save();

        p->setOpacity(p->opacity()*mTransformAnimator.getOpacity()*0.01);
        p->setPen(Qt::NoPen);
        if(mFillPaintSettings.getPaintType() == GRADIENTPAINT) {
            p->setBrush(mDrawFillGradient);
        } else if(mFillPaintSettings.getPaintType() == FLATPAINT) {
            p->setBrush(mFillPaintSettings.getCurrentColor().qcol);
        } else{
            p->setBrush(Qt::NoBrush);
        }
        p->drawPath(mMappedPath);
        if(mStrokeSettings.getPaintType() == GRADIENTPAINT) {
            p->setBrush(mDrawStrokeGradient);
        } else if(mStrokeSettings.getPaintType() == FLATPAINT) {
            p->setBrush(mStrokeSettings.getCurrentColor().qcol);
        } else{
            p->setBrush(Qt::NoBrush);
        }

        p->drawPath(mMappedOutlinePath);

        p->restore();
    }
}

bool PathBox::pointInsidePath(QPointF point)
{
    return mMappedWhole.contains(point);
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

const PaintSettings *PathBox::getFillSettings()
{
    return &mFillPaintSettings;
}

const StrokeSettings *PathBox::getStrokeSettings()
{
    return &mStrokeSettings;
}
