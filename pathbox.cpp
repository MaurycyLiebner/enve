#include "pathbox.h"
#include "updatescheduler.h"

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

        p->setOpacity(p->opacity()*mTransformAnimator.getOpacity()*0.01 );
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
