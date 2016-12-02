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
        if(!mAnimatorsCollection.hasKeys() && !mPivotChanged ) centerPivotPosition();
    }
}

void PathBox::updatePrettyPixmap() {
    if(mRedoUpdate) {
        mRedoUpdate = false;
        updateUpdateTransform();
    }

    updateEffectsMarginIfNeeded();
    updatePathIfNeeded();
    updateOutlinePathIfNeeded();
    updateBoundingRect();

    BoundingBox::updateAllUglyPixmap();
    if(mMainWindow->getCanvas()->highQualityPaint()) {
        BoundingBox::updatePrettyPixmap();
        mHighQualityPaint = true;
    } else {
        mHighQualityPaint = false;
    }
}

void PathBox::schedulePathUpdate()
{
    scheduleAwaitUpdate();
    if(mPathUpdateNeeded) {
        return;
    }

    mPathUpdateNeeded = true;
    mOutlinePathUpdateNeeded = false;
}

void PathBox::scheduleOutlinePathUpdate()
{
    scheduleAwaitUpdate();
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

void PathBox::updateAfterCombinedTransformationChanged()
{
}

void PathBox::updateOutlinePath() {
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

void PathBox::updatePixBoundingRectClippedToView() {
    mPixBoundingRectClippedToView = mPixBoundingRect.intersected(
                mMainWindow->getCanvas()->rect());
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

void PathBox::updateBoundingRect() {
    QRectF relBoundingRect = mWholePath.boundingRect().adjusted(-mEffectsMargin, -mEffectsMargin,
                                                                mEffectsMargin, mEffectsMargin);
    mPixBoundingRect = mUpdateTransform.mapRect(relBoundingRect);
    mBoundingRect = QPainterPath();
    mBoundingRect.addRect(relBoundingRect);
    mMappedBoundingRect = mUpdateTransform.map(mBoundingRect);
    updatePixBoundingRectClippedToView();
}

QRectF PathBox::getPixBoundingRect()
{
    return mPixBoundingRect;
}

void PathBox::render(QPainter *p) {
    p->save();

    if(mEffects.isEmpty() ) {
        p->setTransform(QTransform(getCombinedRenderTransform()), false);
        draw(p);
    } else {
        QMatrix renderTransform = getCombinedRenderTransform();
        QRectF relBoundingRect = mWholePath.boundingRect().adjusted(-mEffectsMargin, -mEffectsMargin,
                                                                    mEffectsMargin, mEffectsMargin);
        QRectF pixBoundingRect = renderTransform.mapRect(relBoundingRect);

        QSizeF sizeF = pixBoundingRect.size();
        QPixmap renderPixmap = QPixmap(QSize(ceil(sizeF.width()),
                                             ceil(sizeF.height())) );
        renderPixmap.fill(Qt::transparent);

        QPainter pixP(&renderPixmap);
        pixP.setRenderHint(QPainter::Antialiasing);
        pixP.translate(-pixBoundingRect.topLeft());
        pixP.setTransform(QTransform(renderTransform), true);

        draw(&pixP);
        pixP.end();

        renderPixmap = applyEffects(renderPixmap, mHighQualityPaint);

        p->drawPixmap(pixBoundingRect.topLeft(), renderPixmap);
    }

    p->restore();
}

void PathBox::renderFinal(QPainter *p) {
    p->save();

    if(mEffects.isEmpty() ) {
        p->setTransform(QTransform(getCombinedFinalRenderTransform()), false);
        draw(p);
    } else {
        QMatrix renderTransform = getCombinedFinalRenderTransform();
        QRectF relBoundingRect = mWholePath.boundingRect().adjusted(-mEffectsMargin, -mEffectsMargin,
                                                                    mEffectsMargin, mEffectsMargin);
        QRectF pixBoundingRect = renderTransform.mapRect(relBoundingRect);

        QSizeF sizeF = pixBoundingRect.size();
        QPixmap renderPixmap = QPixmap(QSize(ceil(sizeF.width()),
                                             ceil(sizeF.height())) );
        renderPixmap.fill(Qt::transparent);

        QPainter pixP(&renderPixmap);
        pixP.setRenderHint(QPainter::Antialiasing);
        pixP.translate(-pixBoundingRect.topLeft());
        pixP.setTransform(QTransform(renderTransform), true);

        draw(&pixP);
        pixP.end();

        renderPixmap = applyEffects(renderPixmap, true);

        p->drawPixmap(pixBoundingRect.topLeft(), renderPixmap);
    }

    p->restore();
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

bool PathBox::absPointInsidePath(QPointF absPoint) {
    if(mMappedBoundingRect.contains(absPoint) ) {
        return mWholePath.contains(mapAbsPosToRel(absPoint));
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

const PaintSettings *PathBox::getFillSettings()
{
    return &mFillPaintSettings;
}

const StrokeSettings *PathBox::getStrokeSettings()
{
    return &mStrokeSettings;
}
