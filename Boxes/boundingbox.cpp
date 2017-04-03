#include "Boxes/boundingbox.h"
#include "canvas.h"
#include "undoredo.h"
#include "Boxes/boxesgroup.h"
#include <QDebug>
#include "mainwindow.h"
#include "keysview.h"
#include "BoxesList/boxscrollwidget.h"
#include "BoxesList/OptimalScrollArea/singlewidgetabstraction.h"
#include "durationrectangle.h"

BoundingBox::BoundingBox(BoxesGroup *parent, BoundingBoxType type) :
    ComplexAnimator(), Transformable()
{
    mParent = parent;

    mEffectsAnimators.blockPointer();
    mEffectsAnimators.prp_setName("effects");
    mEffectsAnimators.setParentBox(this);
    mEffectsAnimators.prp_setUpdater(new PixmapEffectUpdater(this));
    mEffectsAnimators.prp_blockUpdater();

    ca_addChildAnimator(&mTransformAnimator);
    mTransformAnimator.blockPointer();

    mTransformAnimator.prp_setUpdater(new TransUpdater(this) );
    mTransformAnimator.prp_blockUpdater();
    mType = type;

    mTransformAnimator.reset();
    if(parent == NULL) return;
    parent->addChild(this);
    updateCombinedTransform();
}

BoundingBox::BoundingBox(BoundingBoxType type) :
    ComplexAnimator(), Transformable() {
    mType = type;
    mTransformAnimator.reset();
}

BoundingBox::~BoundingBox() {
    delete mUpdateRenderContainer;
}

void BoundingBox::prp_updateAfterChangedAbsFrameRange(const int &minFrame,
                                                      const int &maxFrame) {
    int minRelFrame;
    if(minFrame == INT_MIN) {
        minRelFrame = INT_MIN;
    } else {
        minRelFrame = prp_absFrameToRelFrame(minFrame);
    }
    int maxRelFrame;
    if(maxFrame == INT_MAX) {
        maxRelFrame = INT_MAX;
    } else {
        maxRelFrame = prp_absFrameToRelFrame(maxFrame);
    }
    mRenderCacheHandler.addRangeNeedingUpdate(minRelFrame, maxRelFrame);
    scheduleRenderCacheChange();
}

void BoundingBox::ca_childAnimatorIsRecordingChanged() {
    ComplexAnimator::ca_childAnimatorIsRecordingChanged();
    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Animated);
    SWT_scheduleWidgetsContentUpdateWithRule(SWT_NotAnimated);
}

void BoundingBox::ca_addDescendantsKey(Key *key) {
    if(mNoCache) {
        ComplexAnimator::ca_addDescendantsKey(key);
    } else {
        ComplexKey *collection =
                ca_getKeyCollectionAtAbsFrame(key->getAbsFrame());
        if(collection == NULL) {
            collection = new ComplexKey(this);
            collection->setAbsFrame(key->getAbsFrame());
            anim_appendKey(collection);
            mRenderCacheHandler.addAddedKey(collection);
        } else {
            mRenderCacheHandler.addChangedKey(collection);
        }
        collection->addAnimatorKey(key);
        scheduleRenderCacheChange();
    }
}

void BoundingBox::ca_removeDescendantsKey(Key *key) {
    if(mNoCache) {
        ComplexAnimator::ca_removeDescendantsKey(key);
    } else {
        ComplexKey *collection = key->getParentKey();
        if(collection == NULL) return;
        collection->removeAnimatorKey(key);
        if(collection->isEmpty() ) {
            mRenderCacheHandler.addRemovedKey(collection);
            anim_removeKey(collection);
        } else {
            mRenderCacheHandler.addChangedKey(collection);
        }
        scheduleRenderCacheChange();
    }
}

SingleWidgetAbstraction* BoundingBox::SWT_getAbstractionForWidget(
            ScrollWidgetVisiblePart *visiblePartWidget) {
    foreach(SingleWidgetAbstraction *abs, mSWT_allAbstractions) {
        if(abs->getParentVisiblePartWidget() == visiblePartWidget) {
            return abs;
        }
    }
    return SWT_createAbstraction(visiblePartWidget);
}

#include "linkbox.h"
BoundingBox *BoundingBox::createLink(BoxesGroup *parent) {
    InternalLinkBox *linkBox = new InternalLinkBox(this, parent);
    BoundingBox::prp_makeDuplicate(linkBox);
    return linkBox;
}

void BoundingBox::prp_makeDuplicate(Property *property) {
    BoundingBox *targetBox = (BoundingBox*)property;
    targetBox->duplicateTransformAnimatorFrom(&mTransformAnimator);
    int effectsCount = mEffectsAnimators.ca_getNumberOfChildren();
    for(int i = 0; i < effectsCount; i++) {
        targetBox->addEffect((PixmapEffect*)((PixmapEffect*)mEffectsAnimators.
                           ca_getChildAt(i))->prp_makeDuplicate() );
    }
}

Property *BoundingBox::prp_makeDuplicate() {
    return createDuplicate(mParent);
}

BoundingBox *BoundingBox::createDuplicate(BoxesGroup *parent) {
    BoundingBox *target = createNewDuplicate(parent);
    prp_makeDuplicate(target);
    return target;
}

BoundingBox *BoundingBox::createSameTransformationLink(BoxesGroup *parent) {
    return new SameTransformInternalLink(this, parent);
}

bool BoundingBox::isAncestor(BoxesGroup *box) const {
    if(mParent == box) return true;
    if(mParent == NULL) return false;
    return mParent->isAncestor(box);
}

bool BoundingBox::isAncestor(BoundingBox *box) const {
    if(box->isGroup()) {
        return isAncestor((BoxesGroup*)box);
    }
    return false;
}

void BoundingBox::applyEffects(QImage *im,
                               qreal scale) {
    if(mEffectsAnimators.hasChildAnimators()) {
        fmt_filters::image img(im->bits(), im->width(), im->height());
        mEffectsAnimators.applyEffects(this,
                                       im,
                                       img,
                                       scale);
    }
}

BoundingBoxRenderContainer *BoundingBox::getRenderContainerAtFrame(
                                                const int &frame) {
    return mRenderCacheHandler.getRenderContainerAtRelFrame(frame);
}


#include <QSqlError>
int BoundingBox::prp_saveToSql(QSqlQuery *query, const int &parentId) {
    Q_UNUSED(parentId);
    int transfromAnimatorId = mTransformAnimator.prp_saveToSql(query);
    if(!query->exec(
                QString("INSERT INTO boundingbox (name, boxtype, transformanimatorid, "
                        "pivotchanged, visible, locked, "
                "parentboundingboxid) "
                "VALUES ('%1', %2, %3, %4, %5, %6, %7)").
                arg(prp_mName).
                arg(mType).
                arg(transfromAnimatorId).
                arg(boolToSql(mPivotChanged)).
                arg(boolToSql(mVisible) ).
                arg(boolToSql(mLocked) ).
                arg(parentId)
                ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }

    int boxId = query->lastInsertId().toInt();
    if(mEffectsAnimators.hasChildAnimators()) {
        mEffectsAnimators.prp_saveToSql(query, boxId);
    }
    return boxId;
}

void BoundingBox::prp_loadFromSql(const int &boundingBoxId) {
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
        mTransformAnimator.prp_loadFromSql(transformAnimatorId);
        mEffectsAnimators.prp_loadFromSql(boundingBoxId);
        mPivotChanged = pivotChanged;
        mLocked = locked;
        mVisible = visible;
        prp_mName = query.value(idName).toString();
    } else {
        qDebug() << "Could not load boundingbox with id " << boundingBoxId;
    }
}

void BoundingBox::preUpdatePixmapsUpdates() {
    updateEffectsMarginIfNeeded();
    //updateBoundingRect();
}

void BoundingBox::updatePixmaps() {
//    if(mRedoUpdate) {
//        mRedoUpdate = false;
//        updateUpdateTransform();
//    }

    preUpdatePixmapsUpdates();
    updateAllUglyPixmap();
}

//void BoundingBox::setAwaitingUpdate(bool bT) {
//    mAwaitingUpdate = bT;
//    if(bT) {
//        setUpdateVars();
//        if(!mUpdateReplaceCache) {
//            BoundingBoxRenderContainer *cont = getRenderContainerAtFrame(
//                                                    mUpdateFrame);
//            if(cont != NULL) {
//                mOldRenderContainer = cont;
//                mOldRenderContainer->updatePaintTransformGivenNewCombinedTransform(
//                            mCombinedTransformMatrix);
//            }
//        }
//    } else {
//        afterSuccessfulUpdate();
//        //mReplaceCache = false;

//        if(mHighQualityPaint) {
//            mOldPixmap = mNewPixmap;
//            mOldPixBoundingRect = mPixBoundingRectClippedToView;
//            mOldTransform = mUpdateTransform;
//        }

////        mUpdateRenderContainer->updatePaintTransformGivenNewCombinedTransform(
////                                            mCombinedTransformMatrix);

//        if(mUpdateReplaceCache) {
//            for(auto pair : mRenderContainers) {
//                delete pair.second;
//            }

//            mRenderContainers.clear();
//            mOldRenderContainer = getRenderContainerAtFrame(
//                                            mUpdateFrame);

//            if(mOldRenderContainer == NULL) {
//                mOldRenderContainer = new BoundingBoxRenderContainer();
//                mRenderContainers.insert({mUpdateFrame, mOldRenderContainer});
//            }
//            mOldRenderContainer->duplicateFrom(mUpdateRenderContainer);
//        } else {
//            mOldRenderContainer = getRenderContainerAtFrame(
//                                            mUpdateFrame);
//            if(mOldRenderContainer == NULL) {
//                mOldRenderContainer = new BoundingBoxRenderContainer();
//                mRenderContainers.insert({mUpdateFrame, mOldRenderContainer});
//                mOldRenderContainer->duplicateFrom(mUpdateRenderContainer);
//            }
//        }
//        updateUglyPaintTransform();

//        mParent->scheduleUpdate(mUpdateReplaceCache);
//        mUpdateReplaceCache = false;
//    }
//}

QImage BoundingBox::getPrettyPixmapProvidedTransform(
                                         const QMatrix &transform,
                                         QRectF *pixBoundingRectClippedToViewP) {
    QRectF pixBoundingRectClippedToView = *pixBoundingRectClippedToViewP;
    QSizeF sizeF = pixBoundingRectClippedToView.size();
    QImage newPixmap = QImage(QSize(ceil(sizeF.width()),
                                      ceil(sizeF.height())),
                              QImage::Format_ARGB32_Premultiplied);
    newPixmap.fill(Qt::transparent);

    QPainter p(&newPixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.translate(-pixBoundingRectClippedToView.topLeft());
    QPointF transF = pixBoundingRectClippedToView.topLeft() -
            QPointF(qRound(pixBoundingRectClippedToView.left()),
                    qRound(pixBoundingRectClippedToView.top()));
    pixBoundingRectClippedToView.translate(-transF);
    p.translate(transF);
    p.setTransform(QTransform(transform), true);

    draw(&p);
    p.end();

//    if(parentCanvas->effectsPaintEnabled()) {
//        newPixmap = applyEffects(newPixmap,
//                                 true,
//                                 mUpdateCanvasTransform.m11());
//    }

    *pixBoundingRectClippedToViewP = pixBoundingRectClippedToView;

    return newPixmap;
}

Canvas *BoundingBox::getParentCanvas() {
    return mParent->getParentCanvas();
}

void BoundingBox::duplicateTransformAnimatorFrom(
        TransformAnimator *source) {
    source->prp_makeDuplicate(&mTransformAnimator);
}

void BoundingBox::updateAllBoxes() {
    scheduleUpdate();
}

void BoundingBox::clearAllCache() {
    mRenderCacheHandler.clearAllCache();
}

void BoundingBox::replaceCurrentFrameCache() {
    mReplaceCache = true;

    if(mParent == NULL) return;
    mParent->BoundingBox::replaceCurrentFrameCache();
    //scheduleUpdate();
}

QImage BoundingBox::getAllUglyPixmapProvidedTransform(
        const qreal &effectsMargin,
        const QMatrix &allUglyTransform,
        QRectF *allUglyBoundingRectP) {
    QRectF allUglyBoundingRect = allUglyTransform.mapRect(mUpdateRelBoundingRect).
            adjusted(-effectsMargin, -effectsMargin,
                     effectsMargin, effectsMargin);

    QSizeF sizeF = allUglyBoundingRect.size();
    QImage allUglyPixmap = QImage(QSize(ceil(sizeF.width()),
                                   ceil(sizeF.height())),
                                  QImage::Format_ARGB32_Premultiplied);
    allUglyPixmap.fill(Qt::transparent);

    QPainter p(&allUglyPixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.translate(-allUglyBoundingRect.topLeft());
    QPointF transF = allUglyBoundingRect.topLeft() -
            QPointF(qRound(allUglyBoundingRect.left()),
                    qRound(allUglyBoundingRect.top()));
    allUglyBoundingRect.translate(-transF);
    p.translate(transF);
    p.setTransform(QTransform(allUglyTransform), true);

    draw(&p);
    p.end();

//    if(parentCanvas->effectsPaintEnabled()) {
//        allUglyPixmap = applyEffects(allUglyPixmap,
//                                     false,
//                                     parentCanvas->getResolutionPercent());
//    }

    *allUglyBoundingRectP = allUglyBoundingRect;
    return allUglyPixmap;
}

void BoundingBox::updateAllUglyPixmap() {
    Canvas *parentCanvas = getParentCanvas();

    mUpdateRenderContainer->updateVariables(
                mUpdateTransform,
                mEffectsMargin,
                parentCanvas->getResolutionPercent(),
                this);
    mUpdateRenderContainer->setFrame(mUpdateRelFrame);
}

void BoundingBox::drawPreviewPixmap(QPainter *p) {
    drawPixmap(p);
//    p->save();

//    p->setOpacity(mTransformAnimator.getOpacity()*0.01);

//    p->drawImage(mPreviewDrawPos, mRenderPixmap);
//    p->restore();
}

void BoundingBox::renderFinal(QPainter *p) {
    p->save();

    QMatrix renderTransform = getCombinedFinalRenderTransform();

    QRectF pixBoundingRect = renderTransform.mapRect(mRelBoundingRect).
                            adjusted(-mEffectsMargin, -mEffectsMargin,
                                     mEffectsMargin, mEffectsMargin);

    QSizeF sizeF = pixBoundingRect.size();
    QImage renderPixmap = QImage(QSize(ceil(sizeF.width()),
                                         ceil(sizeF.height())),
                                 QImage::Format_ARGB32_Premultiplied);
    renderPixmap.fill(Qt::transparent);

    QPainter pixP(&renderPixmap);
    pixP.setRenderHint(QPainter::Antialiasing);
    pixP.setRenderHint(QPainter::SmoothPixmapTransform);
    pixP.translate(-pixBoundingRect.topLeft());
    pixP.setTransform(QTransform(renderTransform), true);

    draw(&pixP);
    pixP.end();

    applyEffects(&renderPixmap, true);

    p->setOpacity(mTransformAnimator.getOpacity()*0.01);
    p->drawImage(pixBoundingRect.topLeft(), renderPixmap);

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

void BoundingBox::drawUpdatePixmap(QPainter *p) {
    if(mUpdateDrawOnParentBox) {
        p->save();
        p->setCompositionMode(mCompositionMode);
        p->setOpacity(mUpdateOpacity);
        mUpdateRenderContainer->draw(p);
        p->restore();
    }
}

void BoundingBox::drawUpdatePixmapForEffect(QPainter *p) {
    p->save();
    p->setOpacity(mUpdateOpacity);
    mUpdateRenderContainer->drawWithoutTransform(p);
    p->restore();
}

QRectF BoundingBox::getUpdateRenderRect() {
    return mUpdateRenderContainer->getBoundingRect();
}

QMatrix BoundingBox::getUpdatePaintTransform() {
    return mUpdateRenderContainer->getPaintTransform();
}

void BoundingBox::drawPixmap(QPainter *p) {
    if(isVisibleAndInVisibleDurationRect()) {
        p->save();
        p->setCompositionMode(mCompositionMode);
        p->setOpacity(mTransformAnimator.getOpacity()*0.01 );
        mRenderCacheHandler.drawCurrentRenderContainer(p);
        p->restore();
    }
}

void BoundingBox::setCompositionMode(
        QPainter::CompositionMode compositionMode) {
    mCompositionMode = compositionMode;
    scheduleUpdate();
}

QPainter::CompositionMode BoundingBox::getCompositionMode() {
    return mCompositionMode;
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
    scheduleUpdate();
    mEffectsMarginUpdateNeeded = true;
    if(mParent == NULL) return;
    mParent->scheduleEffectsMarginUpdate();
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
    mCurrentAbsFrame = currentFrame;
    mCurrentRelFrame = mCurrentAbsFrame - prp_getFrameShift();
    prp_setAbsFrame(currentFrame);
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

bool BoundingBox::isParticleBox() {
    return mType == TYPE_PARTICLES;
}

bool BoundingBox::isInternalLink() {
    return mType == TYPE_INTERNAL_LINK;
}

bool BoundingBox::isExternalLink() {
    return mType == TYPE_EXTERNAL_LINK;
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
    mTransformAnimator.
            setPivotWithoutChangingTransformation(relPos,
                                                  saveUndoRedo);//setPivot(relPos, saveUndoRedo);//setPivotWithoutChangingTransformation(relPos, saveUndoRedo);
    schedulePivotUpdate();
}

void BoundingBox::setPivotAbsPos(QPointF absPos,
                                 bool saveUndoRedo, bool pivotChanged) {
    QPointF newPos = mapAbsPosToRel(absPos);
    setPivotRelPos(newPos, saveUndoRedo, pivotChanged);
    updateCombinedTransform();
}

QPointF BoundingBox::getPivotAbsPos() {
    return mCombinedTransformMatrix.map(mTransformAnimator.getPivot());
}

void BoundingBox::select() {
    mSelected = true;

    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Selected);
}

void BoundingBox::scheduleCenterPivot() {
    mCenterPivotScheduled = true;
}

void BoundingBox::updateRelBoundingRect() {
    mRelBoundingRectPath = QPainterPath();
    mRelBoundingRectPath.addRect(mRelBoundingRect);

    if(mCenterPivotScheduled) {
        mCenterPivotScheduled = false;
        centerPivotPosition();
    }
}

void BoundingBox::deselect() {
    mSelected = false;

    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Selected);
}

bool BoundingBox::isContainedIn(QRectF absRect) {
    return absRect.contains(mCombinedTransformMatrix.mapRect(mRelBoundingRect));
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

PaintSettings *BoundingBox::getFillSettings() {
    return NULL;
}

StrokeSettings *BoundingBox::getStrokeSettings() {
    return NULL;
}

void BoundingBox::drawAsBoundingRect(QPainter *p, QPainterPath path) {
    p->save();
    QPen pen = QPen(QColor(0, 0, 0, 125), 1., Qt::DashLine);
    pen.setCosmetic(true);
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);
    p->setTransform(QTransform(mCombinedTransformMatrix), true);
    p->drawPath(path);
    p->restore();
}

void BoundingBox::drawBoundingRect(QPainter *p) {
    drawAsBoundingRect(p, mRelBoundingRectPath);
}

const QPainterPath &BoundingBox::getRelBoundingRectPath() {
    return mRelBoundingRectPath;
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

void BoundingBox::moveByAbs(QPointF trans) {
    mTransformAnimator.moveByAbs(mParent->getCombinedTransform(), trans);
//    QPointF by = mParent->mapAbsPosToRel(trans) -
//                 mParent->mapAbsPosToRel(QPointF(0., 0.));
// //    QPointF by = mapAbsPosToRel(
// //                trans - mapRelativeToAbsolute(QPointF(0., 0.)));

//    moveByRel(by);
}

void BoundingBox::moveByRel(QPointF trans) {
    mTransformAnimator.moveRelativeToSavedValue(trans.x(), trans.y());
}

void BoundingBox::setAbsolutePos(QPointF pos, bool saveUndoRedo) {
    QMatrix combinedM = mParent->getCombinedTransform();
    QPointF newPos = combinedM.inverted().map(pos);
    setRelativePos(newPos, saveUndoRedo );
}

void BoundingBox::setRelativePos(QPointF relPos, bool saveUndoRedo) {
    mTransformAnimator.setPosition(relPos.x(), relPos.y() );
}

void BoundingBox::saveTransformPivotAbsPos(QPointF absPivot) {
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
    mTransformAnimator.prp_startTransform();
}

void BoundingBox::finishTransform() {
    mTransformAnimator.prp_finishTransform();
    //updateCombinedTransform();
}

bool BoundingBox::absPointInsidePath(QPointF absPoint) {
    return relPointInsidePath(mapAbsPosToRel(absPoint));
}

void BoundingBox::cancelTransform() {
    mTransformAnimator.prp_cancelTransform();
    //updateCombinedTransform();
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

void BoundingBox::updateRelativeTransformTmp() {
    mRelativeTransformMatrix = mTransformAnimator.qra_getCurrentValue();
    updateCombinedTransformTmp();
    //updateCombinedTransform(replaceCache);
}

void BoundingBox::updateRelativeTransformAfterFrameChange() {
    mRelativeTransformMatrix = mTransformAnimator.qra_getCurrentValue();
    updateCombinedTransform();
}

void BoundingBox::updateCombinedTransform() {
    if(mParent == NULL) return;
    mCombinedTransformMatrix = mRelativeTransformMatrix*
                               mParent->getCombinedTransform();

    updateAfterCombinedTransformationChanged();
    scheduleUpdate();
}

void BoundingBox::updateCombinedTransformTmp() {
    if(mAwaitingUpdate) {
        mCombinedTransformMatrix = mRelativeTransformMatrix*
                                   mParent->getCombinedTransform();
        mRenderCacheHandler.updateCurrentRenderContainerTransform(
                                    mCombinedTransformMatrix);
    } else {
        updateCombinedTransform();
    }
}

TransformAnimator *BoundingBox::getTransformAnimator() {
    return &mTransformAnimator;
}

QMatrix BoundingBox::getCombinedRenderTransform() {
    return mTransformAnimator.qra_getCurrentValue()*
            mParent->getCombinedRenderTransform();
}

QMatrix BoundingBox::getCombinedFinalRenderTransform() {
    return mTransformAnimator.qra_getCurrentValue()*
            mParent->getCombinedFinalRenderTransform();
}

void BoundingBox::selectionChangeTriggered(bool shiftPressed) {
    Canvas *parentCanvas = getParentCanvas();
    if(shiftPressed) {
        if(mSelected) {
            parentCanvas->removeBoxFromSelection(this);
        } else {
            parentCanvas->addBoxToSelection(this);
        }
    } else {
        parentCanvas->clearBoxesSelection();
        parentCanvas->addBoxToSelection(this);
    }
}

void BoundingBox::addEffect(PixmapEffect *effect) {
    //effect->setUpdater(new PixmapEffectUpdater(this));

    if(!mEffectsAnimators.hasChildAnimators()) {
        ca_addChildAnimator(&mEffectsAnimators);
    }
    mEffectsAnimators.ca_addChildAnimator(effect);

    scheduleEffectsMarginUpdate();
    scheduleUpdate();
}

void BoundingBox::removeEffect(PixmapEffect *effect) {
    mEffectsAnimators.ca_removeChildAnimator(effect);
    if(!mEffectsAnimators.hasChildAnimators()) {
        ca_removeChildAnimator(&mEffectsAnimators);
    }
    effect->decNumberPointers();

    scheduleEffectsMarginUpdate();
    scheduleUpdate();
}

void BoundingBox::getKeysInRect(const QRectF &selectionRect,
                                const qreal &pixelsPerFrame,
                                QList<Key*> *keysList) {
    prp_getKeysInRect(selectionRect,
                                       pixelsPerFrame,
                                       keysList);
}

Key *BoundingBox::getKeyAtPos(const qreal &relX,
                              const int &minViewedFrame,
                              const qreal &pixelsPerFrame) {
    return prp_getKeyAtPos(
                                     relX,
                                     minViewedFrame,
                                     pixelsPerFrame);
}

int BoundingBox::prp_getParentFrameShift() const {
    if(mParent == NULL) {
        return 0;
    } else {
        return mParent->prp_getFrameShift();
    }
}

int BoundingBox::prp_getFrameShift() const {
    if(mDurationRectangle == NULL) {
        return prp_getParentFrameShift();
    } else {
        return mDurationRectangle->getFramePos() + prp_getParentFrameShift();
    }
}

void BoundingBox::setDurationRectangle(DurationRectangle *durationRect) {
    if(mDurationRectangle != NULL) {
        disconnect(mDurationRectangle, SIGNAL(changed()),
                   this, SLOT(updateAfterDurationRectangleChanged()));
    }
    mDurationRectangle = durationRect;
    if(mDurationRectangle == NULL) return;
    connect(mDurationRectangle, SIGNAL(changed()),
            this, SLOT(updateAfterDurationRectangleChanged()));
}

void BoundingBox::drawKeys(QPainter *p,
                           qreal pixelsPerFrame,
                           qreal drawY,
                           int startFrame, int endFrame) {
    mRenderCacheHandler.drawCacheOnTimeline(p, pixelsPerFrame,
                                            drawY,
                                            prp_absFrameToRelFrame(startFrame),
                                            prp_absFrameToRelFrame(endFrame));
    prp_drawKeys(p,
                 pixelsPerFrame, drawY,
                 startFrame, endFrame);
}

void BoundingBox::setName(QString name)
{
    prp_mName = name;
}

QString BoundingBox::getName()
{
    return prp_mName;
}

bool BoundingBox::isInVisibleDurationRect() {
    if(mDurationRectangle == NULL) return true;
    return mCurrentAbsFrame < mDurationRectangle->getMaxFrame() &&
           mCurrentAbsFrame >= mDurationRectangle->getMinFrame();
}

bool BoundingBox::isVisibleAndInVisibleDurationRect() {
    return isInVisibleDurationRect() && mVisible;
}

void BoundingBox::setVisibile(bool visible, bool saveUndoRedo) {
    if(mVisible == visible) return;
    if(mSelected) {
        removeFromSelection();
    }
    if(saveUndoRedo) {
        addUndoRedo(new SetBoxVisibleUndoRedo(this, mVisible, visible));
    }
    mVisible = visible;

    scheduleUpdate();

    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Visible);
    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Invisible);
}

void BoundingBox::switchVisible() {
    setVisibile(!mVisible);
}

void BoundingBox::switchLocked() {
    setLocked(!mLocked);
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
        getParentCanvas()->removeBoxFromSelection(this);
    }
    mLocked = bt;
    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Locked);
    SWT_scheduleWidgetsContentUpdateWithRule(SWT_Unlocked);
}

bool BoundingBox::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                      const bool &parentSatisfies,
                                      const bool &) {
    const SWT_Rule &rule = rules.rule;
    const SWT_Type &type = rules.type;
    bool satisfies;
    bool alwaysShowChildren = rules.alwaysShowChildren;
    if(type == SWT_SingleSound) return false;
    if(alwaysShowChildren) {
        if(rule == SWT_NoRule) {
            satisfies = parentSatisfies;
        } else if(rule == SWT_Selected) {
            satisfies = isSelected() || parentSatisfies;
        } else if(rule == SWT_Animated) {
            satisfies = isAnimated() || parentSatisfies;
        } else if(rule == SWT_NotAnimated) {
            satisfies = !isAnimated() || parentSatisfies;
        } else if(rule == SWT_Visible) {
            satisfies = isVisible() || parentSatisfies;
        } else if(rule == SWT_Invisible) {
            satisfies = !isVisible() || parentSatisfies;
        } else if(rule == SWT_Locked) {
            satisfies = isLocked() || parentSatisfies;
        } else if(rule == SWT_Unlocked) {
            satisfies = !isLocked() || parentSatisfies;
        }
    } else {
        if(rule == SWT_NoRule) {
            satisfies = parentSatisfies;
        } else if(rule == SWT_Selected) {
            satisfies = isSelected();
        } else if(rule == SWT_Animated) {
            satisfies = isAnimated();
        } else if(rule == SWT_NotAnimated) {
            satisfies = !isAnimated();
        } else if(rule == SWT_Visible) {
            satisfies = isVisible() && parentSatisfies;
        } else if(rule == SWT_Invisible) {
            satisfies = !isVisible() || parentSatisfies;
        } else if(rule == SWT_Locked) {
            satisfies = isLocked() || parentSatisfies;
        } else if(rule == SWT_Unlocked) {
            satisfies = !isLocked() && parentSatisfies;
        }
    }
    if(satisfies) {
        const QString &nameSearch = rules.searchString;
        if(!nameSearch.isEmpty()) {
            satisfies = prp_mName.contains(nameSearch);
        }
    }
    return satisfies;
}

void BoundingBox::SWT_addToContextMenu(
        QMenu *menu) {
    menu->addAction("Apply Transformation");
    menu->addAction("Create Link");
    menu->addAction("Center Pivot");
    menu->addAction("Copy");
    menu->addAction("Cut");
    menu->addAction("Duplicate");
    menu->addAction("Group");
    menu->addAction("Ungroup");
    menu->addAction("Delete");

    QMenu *effectsMenu = menu->addMenu("Effects");
    effectsMenu->addAction("Blur");
    effectsMenu->addAction("Shadow");
//            effectsMenu->addAction("Brush");
    effectsMenu->addAction("Lines");
    effectsMenu->addAction("Circles");
    effectsMenu->addAction("Swirl");
    effectsMenu->addAction("Oil");
    effectsMenu->addAction("Implode");
    effectsMenu->addAction("Desaturate");
}

void BoundingBox::removeFromParent() {
    mParent->removeChild(this);
}

void BoundingBox::removeFromSelection() {
    if(mSelected) {
        getParentCanvas()->removeBoxFromSelection(this);
    }
}

bool BoundingBox::SWT_handleContextMenuActionSelected(
        QAction *selectedAction) {
    if(selectedAction != NULL) {
        if(selectedAction->text() == "Delete") {
            mParent->removeChild(this);
        } else if(selectedAction->text() == "Apply Transformation") {
            applyCurrentTransformation();
        } else if(selectedAction->text() == "Create Link") {
            createLink(mParent);
        } else if(selectedAction->text() == "Group") {
            getParentCanvas()->groupSelectedBoxes();
            return true;
//        } else if(selectedAction->text() == "Ungroup") {
//            ungroupSelected();
//        } else if(selectedAction->text() == "Center Pivot") {
//            mCurrentBoxesGroup->centerPivotForSelected();
//        } else if(selectedAction->text() == "Blur") {
//            mCurrentBoxesGroup->applyBlurToSelected();
//        } else if(selectedAction->text() == "Shadow") {
//            mCurrentBoxesGroup->applyShadowToSelected();
//        } else if(selectedAction->text() == "Brush") {
//            mCurrentBoxesGroup->applyBrushEffectToSelected();
//        } else if(selectedAction->text() == "Lines") {
//            mCurrentBoxesGroup->applyLinesEffectToSelected();
//        } else if(selectedAction->text() == "Circles") {
//            mCurrentBoxesGroup->applyCirclesEffectToSelected();
//        } else if(selectedAction->text() == "Swirl") {
//            mCurrentBoxesGroup->applySwirlEffectToSelected();
//        } else if(selectedAction->text() == "Oil") {
//            mCurrentBoxesGroup->applyOilEffectToSelected();
//        } else if(selectedAction->text() == "Implode") {
//            mCurrentBoxesGroup->applyImplodeEffectToSelected();
//        } else if(selectedAction->text() == "Desaturate") {
//            mCurrentBoxesGroup->applyDesaturateEffectToSelected();
        }
    } else {

    }
    return false;
}
#include "updatescheduler.h"
void BoundingBox::scheduleRenderCacheChange() {
    if(mRenderCacheChangeNeeded) return;
    mRenderCacheChangeNeeded = true;
    addUpdateScheduler(new ApplyRenderCacheChangesScheduler(this));
}

void BoundingBox::applyRenderCacheChanges() {
    if(mRenderCacheChangeNeeded) {
        mRenderCacheChangeNeeded = false;
        mRenderCacheHandler.applyChanges();
    }
}

void BoundingBox::beforeUpdate() {
    qDebug() << "before update " + prp_mName;
    setUpdateVars();
//    if(!mUpdateReplaceCache) {
//        BoundingBoxRenderContainer *cont = getRenderContainerAtFrame(
//                                                mUpdateFrame);
//        if(cont != NULL) {
//            mOldRenderContainer = cont;
//            mOldRenderContainer->updatePaintTransformGivenNewCombinedTransform(
//                        mCombinedTransformMatrix);
//        }
//    }
    mAwaitingUpdate = false;
}

void BoundingBox::processUpdate() {
    qDebug() << "process update " + prp_mName;
    if(mUpdateReplaceCache) {
        updatePixmaps();
    }
}

void BoundingBox::setNoCache(const bool &bT) {
    mNoCache = bT;
    clearAllCache();
    mRenderCacheHandler.createNewRenderContainerAtRelFrame(mCurrentRelFrame);
    mRenderCacheHandler.updateCurrentRenderContainerFromFrame(mCurrentRelFrame);
    replaceCurrentFrameCache();
}

void BoundingBox::afterUpdate() {
    afterSuccessfulUpdate();

    if(mNoCache) {
        mRenderCacheHandler.duplicateCurrentRenderContainerFrom(
                                            mUpdateRenderContainer);
    } else {
        mRenderCacheHandler.updateCurrentRenderContainerFromFrame(
                                                    mUpdateRelFrame);
        if(mUpdateReplaceCache) {
            mRenderCacheHandler.duplicateCurrentRenderContainerFrom(
                                            mUpdateRenderContainer);
        }
    }
    mRenderCacheHandler.updateCurrentRenderContainerTransform(
                                            mCombinedTransformMatrix);
}

void BoundingBox::setUpdateVars() {
    updateRelBoundingRect();

    mUpdateTransform = mCombinedTransformMatrix;
    mUpdateRelFrame = mCurrentRelFrame;
    mUpdateRelBoundingRect = mRelBoundingRect;
    mUpdateDrawOnParentBox = isVisibleAndInVisibleDurationRect();
    mUpdateReplaceCache = getRenderContainerAtFrame(mUpdateRelFrame) == NULL ||
                          mReplaceCache;
    mUpdateOpacity = mTransformAnimator.getOpacity()*0.01;
    mReplaceCache = false;
}

bool BoundingBox::isUsedAsTarget() {
    return mUsedAsTargetCount > 0;
}

void BoundingBox::incUsedAsTarget() {
    mUsedAsTargetCount++;
}

void BoundingBox::decUsedAsTarget() {
    mUsedAsTargetCount--;
}

bool BoundingBox::shouldUpdateAndDraw() {
    return isVisibleAndInVisibleDurationRect() ||
            (isInVisibleDurationRect() && isUsedAsTarget());
}

void BoundingBox::scheduleUpdate() {
    //if(mAwaitingUpdate) return;
    if(shouldUpdateAndDraw()) {
        qDebug() << "schedule update " + prp_mName;
        mAwaitingUpdate = true;
        mParent->addChildAwaitingUpdate(this);
        emit scheduledUpdate();
    }
}
