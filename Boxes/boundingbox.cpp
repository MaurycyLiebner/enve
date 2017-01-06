#include "Boxes/boundingbox.h"
#include "canvas.h"
#include "undoredo.h"
#include "Boxes/boxesgroup.h"
#include <QDebug>
#include "mainwindow.h"
#include "keysview.h"
#include "ObjectSettings/effectssettingswidget.h"
#include "BoxesList/boxitemwidgetcontainer.h"

BoundingBox::BoundingBox(BoxesGroup *parent, BoundingBoxType type) :
    QObject(), Transformable()
{
    mEffectsAnimators.blockPointer();
    mEffectsAnimators.setName("effects");
    mEffectsAnimators.setUpdater(new PixmapEffectUpdater(this));

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

QPixmap BoundingBox::applyEffects(const QPixmap& pixmap,
                                  bool highQuality,
                                  qreal scale) {
    if(!mEffectsAnimators.hasChildAnimators()) return pixmap;
    QImage im = pixmap.toImage().convertToFormat(
                QImage::Format_ARGB32_Premultiplied);;
    fmt_filters::image img(im.bits(), im.width(), im.height());
    mEffectsAnimators.applyEffects(&im, img, scale, highQuality);
    return QPixmap::fromImage(im);
}

void BoundingBox::addAllEffectsToEffectsSettingsWidget(
        EffectsSettingsWidget *widget) {
    mEffectsAnimators.addAllEffectsToEffectsSettingsWidget(widget);
}

#include <QSqlError>
int BoundingBox::saveToSql(QSqlQuery *query, int parentId) {
    int transfromAnimatorId = mTransformAnimator.saveToSql();
    if(!query->exec(
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
        qDebug() << query->lastError() << endl << query->lastQuery();
    }

    return query->lastInsertId().toInt();
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

void BoundingBox::updatePixmaps() {
    if(mRedoUpdate) {
        mRedoUpdate = false;
        updateUpdateTransform();
    }

    updateEffectsMarginIfNeeded();
    updateBoundingRect();

    BoundingBox::updateAllUglyPixmap();
    if(mMainWindow->getCanvas()->highQualityPaint()) {
        updatePrettyPixmap();
        mHighQualityPaint = true;
    } else {
        mHighQualityPaint = false;
    }

    if(mParent != NULL) mParent->awaitUpdate();
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
    QPointF transF = mPixBoundingRectClippedToView.topLeft() -
            QPointF(qRound(mPixBoundingRectClippedToView.left()),
                    qRound(mPixBoundingRectClippedToView.top()));
    mPixBoundingRectClippedToView.translate(-transF);
    p.translate(transF);
    p.setTransform(QTransform(mUpdateTransform), true);

    draw(&p);
    p.end();

    if(Canvas::effectsPaintEnabled()) {
        mNewPixmap = applyEffects(mNewPixmap,
                                  mHighQualityPaint,
                                  mUpdateCanvasTransform.m11());
    }
}

void BoundingBox::updateAllBoxes() {
    scheduleAwaitUpdate();
}

QRectF BoundingBox::getPixBoundingRect()
{
    return mPixBoundingRect;
}

void BoundingBox::updatePixBoundingRectClippedToView() {
    mPixBoundingRectClippedToView = mPixBoundingRect.intersected(
                mMainWindow->getCanvasWidget()->rect());
}

void BoundingBox::updateAllUglyPixmap() {
    QMatrix inverted = mUpdateCanvasTransform.inverted().scale(Canvas::getResolutionPercent(),
                                                               Canvas::getResolutionPercent());
    mAllUglyTransform = inverted*mUpdateTransform;
    mAllUglyBoundingRect = (mUpdateTransform.inverted()*mAllUglyTransform).mapRect(mPixBoundingRect);
    QSizeF sizeF = mAllUglyBoundingRect.size();
    mAllUglyPixmap = QPixmap(QSize(ceil(sizeF.width()), ceil(sizeF.height())) );
    mAllUglyPixmap.fill(Qt::transparent);

    QPainter p(&mAllUglyPixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.translate(-mAllUglyBoundingRect.topLeft());
    QPointF transF = mAllUglyBoundingRect.topLeft() -
            QPointF(qRound(mAllUglyBoundingRect.left()),
                    qRound(mAllUglyBoundingRect.top()));
    mAllUglyBoundingRect.translate(-transF);
    p.translate(transF);
    p.setTransform(QTransform(mAllUglyTransform), true);

    draw(&p);
    p.end();

    if(Canvas::effectsPaintEnabled()) {
        mAllUglyPixmap = applyEffects(mAllUglyPixmap, false, Canvas::getResolutionPercent());
    }
}

void BoundingBox::render(QPainter *p) {
    p->save();

    if(mEffectsAnimators.hasChildAnimators()) {
        QMatrix renderTransform = getCombinedRenderTransform();

        QRectF pixBoundingRect = renderTransform.mapRect(mRelBoundingRect);

        QSizeF sizeF = pixBoundingRect.size();
        QPixmap renderPixmap = QPixmap(QSize(ceil(sizeF.width()),
                                             ceil(sizeF.height())) );
        renderPixmap.fill(Qt::transparent);

        QPainter pixP(&renderPixmap);
        pixP.setRenderHint(QPainter::Antialiasing);
        pixP.setRenderHint(QPainter::SmoothPixmapTransform);
        pixP.translate(-pixBoundingRect.topLeft());
        pixP.setTransform(QTransform(renderTransform), true);

        draw(&pixP);
        pixP.end();

        renderPixmap = applyEffects(renderPixmap, mHighQualityPaint,
                                    getCurrentCanvasScale()*
                                    Canvas::getResolutionPercent());

        p->setOpacity(mTransformAnimator.getOpacity()*0.01);
        p->drawPixmap(pixBoundingRect.topLeft(), renderPixmap);
    } else {
        p->setTransform(QTransform(getCombinedRenderTransform()), true);
        p->setOpacity(mTransformAnimator.getOpacity()*0.01);
        draw(p);
    }

    p->restore();
}

void BoundingBox::renderFinal(QPainter *p) {
    p->save();

    if(mEffectsAnimators.hasChildAnimators()) {
        QMatrix renderTransform = getCombinedFinalRenderTransform();

        QRectF pixBoundingRect = renderTransform.mapRect(mRelBoundingRect);

        QSizeF sizeF = pixBoundingRect.size();
        QPixmap renderPixmap = QPixmap(QSize(ceil(sizeF.width()),
                                             ceil(sizeF.height())) );
        renderPixmap.fill(Qt::transparent);

        QPainter pixP(&renderPixmap);
        pixP.setRenderHint(QPainter::Antialiasing);
        pixP.setRenderHint(QPainter::SmoothPixmapTransform);
        pixP.translate(-pixBoundingRect.topLeft());
        pixP.setTransform(QTransform(renderTransform), true);

        draw(&pixP);
        pixP.end();

        renderPixmap = applyEffects(renderPixmap, true);

        p->setOpacity(mTransformAnimator.getOpacity()*0.01);
        p->drawPixmap(pixBoundingRect.topLeft(), renderPixmap);
    } else {
        p->setTransform(QTransform(getCombinedFinalRenderTransform()), false);
        p->setOpacity(mTransformAnimator.getOpacity()*0.01);
        draw(p);
    }

    p->restore();
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
    p->setOpacity(mTransformAnimator.getOpacity()*0.01 );
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
            clipRegion2 = clipRegion2.united(QRect(-100000, -100000,
                                                   200000, 200000));
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

    //if(mSelected) drawBoundingRect(p);
}

void BoundingBox::awaitUpdate() {
    if(mAwaitingUpdate) return;
    setAwaitingUpdate(true);
    mMainWindow->addBoxAwaitingUpdate(this);
}

#include "updatescheduler.h"
void BoundingBox::scheduleAwaitUpdate() {
    if(mAwaitingUpdate) {
        redoUpdate();
    } else {
        if(mAwaitUpdateScheduled) return;
        setAwaitUpdateScheduled(true);
        addUpdateScheduler(new AwaitUpdateUpdateScheduler(this));
    }
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
    mEffectsMargin = mEffectsAnimators.getEffectsMargin();
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
    mTransformAnimator.setPivotWithoutChangingTransformation(relPos, saveUndoRedo);//setPivot(relPos, saveUndoRedo);//setPivotWithoutChangingTransformation(relPos, saveUndoRedo);
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

void BoundingBox::addAllAnimatorsToBoxItemWidgetContainer(BoxItemWidgetContainer *container) {
    foreach(QrealAnimator *animator, mActiveAnimators) {
        container->addAnimatorWidgetForAnimator(animator);
    }
}

void BoundingBox::setAnimated(bool bT) {
    mAnimated = bT;
    if(mParent != NULL) {
        if(mAnimated) {
            emit mParent->addAnimatedBoundingBoxSignal(this);
        } else {
            emit mParent->removeAnimatedBoundingBoxSignal(this);
        }
    }
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

const QPainterPath &BoundingBox::getBoundingRectPath() {
    return mBoundingRect;
}

QMatrix BoundingBox::getCombinedTransform() const {
    return mCombinedTransformMatrix;
}

QMatrix BoundingBox::getRelativeTransform() const {
    return mRelativeTransformMatrix;
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
    mTransformAnimator.rotateRelativeToSavedValue(rot,
                                                  mSavedTransformPivot);
}

void BoundingBox::scaleRelativeToSavedPivot(qreal scaleXBy, qreal scaleYBy) {
    mTransformAnimator.scaleRelativeToSavedValue(scaleXBy, scaleYBy,
                                                 mSavedTransformPivot);
}

void BoundingBox::scaleRelativeToSavedPivot(qreal scaleBy) {
    scaleRelativeToSavedPivot(scaleBy, scaleBy);
}

QPointF BoundingBox::mapRelativeToAbsolute(QPointF relPos) const {
    return getCombinedTransform().map(relPos);
}

void BoundingBox::moveBy(QPointF trans) {

    //trans /= getCurrentCanvasScale();

    mTransformAnimator.moveRelativeToSavedValue(trans.x(), trans.y());

    // //mTransformAnimator.translate(trans.x(), trans.y());
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
    mTransformAnimator.finishTransform();
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

        scheduleAwaitUpdate();
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

void BoundingBox::selectionChangeTriggered(bool shiftPressed) {
    if(shiftPressed) {
        if(mSelected) {
            mParent->removeBoxFromSelection(this);
        } else {
            mParent->addBoxToSelection(this);
        }
    } else {
        mParent->clearBoxesSelection();
        mParent->addBoxToSelection(this);
    }
}

void BoundingBox::addEffect(PixmapEffect *effect) {
    effect->setUpdater(new PixmapEffectUpdater(this));
    effect->incNumberPointers();

    if(!mEffectsAnimators.hasChildAnimators()) {
        mAnimatorsCollection.addAnimator(&mEffectsAnimators);
        addActiveAnimator(&mEffectsAnimators);
    }
    mEffectsAnimators.addChildAnimator(effect);

    scheduleEffectsMarginUpdate();
    scheduleAwaitUpdate();
}

void BoundingBox::removeEffect(PixmapEffect *effect) {

    removeActiveAnimator(effect);
    mEffectsAnimators.removeChildAnimator(effect);
    if(!mEffectsAnimators.hasChildAnimators()) {
        mAnimatorsCollection.removeAnimator(&mEffectsAnimators);
        removeActiveAnimator(&mEffectsAnimators);
    }
    effect->decNumberPointers();

    scheduleEffectsMarginUpdate();
    scheduleAwaitUpdate();
}

QrealAnimator *BoundingBox::getAnimatorsCollection() {
    return &mAnimatorsCollection;
}

void BoundingBox::addActiveAnimator(QrealAnimator *animator)
{
    mActiveAnimators << animator;
    emit addActiveAnimatorSignal(animator);
}

void BoundingBox::removeActiveAnimator(QrealAnimator *animator)
{
    mActiveAnimators.removeOne(animator);
    emit removeActiveAnimatorSignal(animator);
}

void BoundingBox::drawKeys(QPainter *p,
                           qreal pixelsPerFrame,
                           qreal drawY,
                           int startFrame, int endFrame) {
    mAnimatorsCollection.drawKeys(p,
                                  pixelsPerFrame, drawY,
                                  startFrame, endFrame);
}

void BoundingBox::setName(QString name)
{
    mName = name;
}

QString BoundingBox::getName()
{
    return mName;
}

void BoundingBox::setVisibile(bool visible, bool saveUndoRedo) {
    if(mVisible == visible) return;
    if(mSelected) {
        ((BoxesGroup*) mParent)->removeBoxFromSelection(this);
    }
    if(saveUndoRedo) {
        addUndoRedo(new SetBoxVisibleUndoRedo(this, mVisible, visible));
    }
    mVisible = visible;

    scheduleAwaitUpdate();
}

void BoundingBox::hide()
{
    setVisibile(false);
}

void BoundingBox::show()
{
    setVisibile(true);
}

bool BoundingBox::isVisibleAndUnlocked() {
    return mVisible && !mLocked;
}

bool BoundingBox::isVisible()
{
    return mVisible;
}

bool BoundingBox::isLocked() {
    return mLocked;
}

void BoundingBox::lock() {
    setLocked(true);
}

void BoundingBox::unlock() {
    setLocked(false);
}

void BoundingBox::setLocked(bool bt) {
    if(bt == mLocked) return;
    if(mSelected) {
        ((BoxesGroup*) mParent)->removeBoxFromSelection(this);
    }
    mLocked = bt;
}
