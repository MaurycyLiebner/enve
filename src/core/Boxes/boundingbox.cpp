#include "Boxes/boundingbox.h"
#include "Boxes/containerbox.h"
#include "canvas.h"
#include "singlewidgetabstraction.h"
#include "Timeline/durationrectangle.h"
#include "pointhelpers.h"
#include "skia/skqtconversions.h"
#include "GUI/global.h"
#include "MovablePoints/movablepoint.h"
#include "PropertyUpdaters/pixmapeffectupdater.h"
#include "Tasks/taskscheduler.h"
#include "Animators/rastereffectanimators.h"
#include "Animators/transformanimator.h"
#include "RasterEffects/rastereffect.h"
#include "RasterEffects/customrastereffectcreator.h"
#include "linkbox.h"
#include "PropertyUpdaters/transformupdater.h"
#include "PropertyUpdaters/boxpathpointupdater.h"
#include "Animators/qpointfanimator.h"
#include "MovablePoints/pathpointshandler.h"
#include "typemenu.h"
#include "patheffectsmenu.h"

SkFilterQuality BoundingBox::sDisplayFiltering = kLow_SkFilterQuality;
int BoundingBox::sNextDocumentId;
QList<BoundingBox*> BoundingBox::sDocumentBoxes;

QList<BoundingBox*> BoundingBox::sReadBoxes;
QList<WaitingForBoxLoad> BoundingBox::sFunctionsWaitingForBoxRead;

int BoundingBox::sNextWriteId;
QList<BoundingBox*> BoundingBox::sBoxesWithWriteIds;

BoundingBox::BoundingBox(const eBoxType type) : eBoxOrSound("box"),
    mDocumentId(sNextDocumentId++), mType(type),
    mTransformAnimator(enve::make_shared<BoxTransformAnimator>()),
    mRasterEffectsAnimators(enve::make_shared<RasterEffectAnimators>(this)) {
    sDocumentBoxes << this;
    ca_addChild(mTransformAnimator);
    mTransformAnimator->prp_setOwnUpdater(
                enve::make_shared<TransformUpdater>(mTransformAnimator.get()));
    const auto pivotAnim = mTransformAnimator->getPivotAnimator();
    const auto pivotUpdater = enve::make_shared<BoxPathPointUpdater>(
                mTransformAnimator.get(), this);
    pivotAnim->prp_setOwnUpdater(pivotUpdater);

    mRasterEffectsAnimators->prp_setOwnUpdater(
                enve::make_shared<PixmapEffectUpdater>(this));
    ca_addChild(mRasterEffectsAnimators);
    mRasterEffectsAnimators->SWT_hide();

    connect(mTransformAnimator.get(),
            &BoxTransformAnimator::totalTransformChanged,
            this, &BoundingBox::afterTotalTransformChanged);
    connect(this, &eBoxOrSound::parentChanged, this, [this]() {
        if(mParentGroup) {
            const auto trans = mParentGroup->getTransformAnimator();
            setParentTransform(trans);
        } else setParentTransform(nullptr);
    });
    connect(this, &eBoxOrSound::visibilityChanged,
            this, [this](const bool visible) {
        for(const auto& box : mLinkingBoxes) {
            if(box->isParentLinkBox()) box->setVisibile(visible);
        }
    });
}

BoundingBox::~BoundingBox() {
    sDocumentBoxes.removeOne(this);
}

void BoundingBox::writeBoundingBox(QIODevice * const dst) {
    if(mWriteId < 0) assignWriteId();
    StaticComplexAnimator::writeProperty(dst);

    gWrite(dst, prp_mName);
    dst->write(rcConstChar(&mWriteId), sizeof(int));
    dst->write(rcConstChar(&mVisible), sizeof(bool));
    dst->write(rcConstChar(&mLocked), sizeof(bool));
    dst->write(rcConstChar(&mBlendModeSk), sizeof(SkBlendMode));
    const bool hasDurRect = mDurationRectangle;
    dst->write(rcConstChar(&hasDurRect), sizeof(bool));

    if(hasDurRect) mDurationRectangle->writeDurationRectangle(dst);
}

void BoundingBox::readBoundingBox(QIODevice * const src) {
    StaticComplexAnimator::readProperty(src);
    prp_setName(gReadString(src));
    src->read(rcChar(&mReadId), sizeof(int));
    src->read(rcChar(&mVisible), sizeof(bool));
    src->read(rcChar(&mLocked), sizeof(bool));
    src->read(rcChar(&mBlendModeSk), sizeof(SkBlendMode));
    bool hasDurRect;
    src->read(rcChar(&hasDurRect), sizeof(bool));

    if(hasDurRect) {
        if(!mDurationRectangle) createDurationRectangle();
        mDurationRectangle->readDurationRectangle(src);
        updateAfterDurationRectangleShifted(0);
    }

    if(hasDurRect) anim_shiftAllKeys(prp_getFrameShift());

    BoundingBox::sAddReadBox(this);
}

BoundingBox *BoundingBox::sGetBoxByDocumentId(const int documentId) {
    for(const auto& box : sDocumentBoxes) {
        if(box->getDocumentId() == documentId) return box;
    }
    return nullptr;
}

void BoundingBox::prp_afterChangedAbsRange(const FrameRange &range) {
    const auto visRange = getVisibleAbsFrameRange();
    const auto croppedRange = visRange*range;
    Animator::prp_afterChangedAbsRange(croppedRange);
    if(croppedRange.inRange(anim_getCurrentAbsFrame())) {
        planScheduleUpdate(UpdateReason::userChange);
    }
}

void BoundingBox::ca_childAnimatorIsRecordingChanged() {
    ComplexAnimator::ca_childAnimatorIsRecordingChanged();
    SWT_scheduleContentUpdate(SWT_BR_ANIMATED);
    SWT_scheduleContentUpdate(SWT_BR_NOT_ANIMATED);
}

qsptr<BoundingBox> BoundingBox::createLink() {
    auto linkBox = enve::make_shared<InternalLinkBox>(this);
    copyBoundingBoxDataTo(linkBox.get());
    return std::move(linkBox);
}

qsptr<BoundingBox> BoundingBox::createLinkForLinkGroup() {
    qsptr<BoundingBox> box = createLink();
    box->clearRasterEffects();
    return box;
}

void BoundingBox::clearRasterEffects() {
    mRasterEffectsAnimators->ca_removeAllChildAnimators();
}

QPointF BoundingBox::getRelCenterPosition() {
    return mRelRect.center();
}

void BoundingBox::centerPivotPosition() {
    mTransformAnimator->setPivotFixedTransform(
                getRelCenterPosition());
}

void BoundingBox::planCenterPivotPosition() {
    mCenterPivotPlanned = true;
}

void BoundingBox::updateIfUsesProgram(
        const ShaderEffectProgram * const program) const {
    mRasterEffectsAnimators->updateIfUsesProgram(program);
}

template <typename T>
void transferData(const T& from, const T& to) {
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    from->writeProperty(&buffer);
    buffer.reset();
    to->readProperty(&buffer);
    buffer.close();
}

void BoundingBox::copyBoundingBoxDataTo(BoundingBox * const targetBox) {
    transferData(mTransformAnimator, targetBox->mTransformAnimator);
    transferData(mRasterEffectsAnimators, targetBox->mRasterEffectsAnimators);
}

void BoundingBox::drawHoveredSk(SkCanvas *canvas, const float invScale) {
    drawHoveredPathSk(canvas, mSkRelBoundingRectPath, invScale);
}

void BoundingBox::drawHoveredPathSk(SkCanvas *canvas,
                                    const SkPath &path,
                                    const float invScale) {
    canvas->save();
    SkPath mappedPath = path;
    mappedPath.transform(toSkMatrix(
                             mTransformAnimator->getTotalTransform()));
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);
    paint.setStrokeWidth(2*invScale);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(mappedPath, paint);

    paint.setColor(SK_ColorRED);
    paint.setStrokeWidth(invScale);
    canvas->drawPath(mappedPath, paint);
    canvas->restore();
}

void BoundingBox::setRasterEffectsEnabled(const bool enable) {
    mRasterEffectsAnimators->SWT_setEnabled(enable);
    mRasterEffectsAnimators->SWT_setVisible(
                mRasterEffectsAnimators->hasChildAnimators() || enable);
}

bool BoundingBox::getRasterEffectsEnabled() const {
    return mRasterEffectsAnimators->SWT_isEnabled();
}

void BoundingBox::applyPaintSetting(const PaintSettingsApplier &setting) {
    Q_UNUSED(setting);
}

bool BoundingBox::SWT_isBoundingBox() const { return true; }

void BoundingBox::updateAllBoxes(const UpdateReason reason) {
    planScheduleUpdate(reason);
}

void BoundingBox::drawAllCanvasControls(SkCanvas * const canvas,
                                        const CanvasMode mode,
                                        const float invScale,
                                        const bool ctrlPressed) {
    for(const auto& prop : mCanvasProps)
        prop->drawCanvasControls(canvas, mode, invScale, ctrlPressed);
}

MovablePoint *BoundingBox::getPointAtAbsPos(const QPointF &absPos,
                                            const CanvasMode mode,
                                            const qreal invScale) const {
    for(int i = mCanvasProps.count() - 1; i >= 0; i--) {
        const auto& prop = mCanvasProps.at(i);
        const auto handler = prop->getPointsHandler();
        if(!handler) continue;
        const auto pt = handler->getPointAtAbsPos(absPos, mode, invScale);
        if(pt) return pt;
    }
    return nullptr;
}

NormalSegment BoundingBox::getNormalSegment(const QPointF &absPos,
                                            const qreal invScale) const {
    for(int i = mCanvasProps.count() - 1; i >= 0; i--) {
        const auto& prop = mCanvasProps.at(i);
        const auto handler = prop->getPointsHandler();
        if(!handler) continue;
        const auto pathHandler = dynamic_cast<PathPointsHandler*>(handler);
        if(!pathHandler) continue;
        const auto seg = pathHandler->getNormalSegment(absPos, invScale);
        if(seg.isValid()) return seg;
    }
    return NormalSegment();
}

void BoundingBox::drawPixmapSk(SkCanvas * const canvas) {
    if(mTransformAnimator->getOpacity() < 0.001) return;
    SkPaint paint;
    const int intAlpha = qRound(mTransformAnimator->getOpacity()*2.55);
    paint.setAlpha(static_cast<U8CPU>(intAlpha));
    paint.setBlendMode(mBlendModeSk);
    paint.setFilterQuality(BoundingBox::sDisplayFiltering);
    drawPixmapSk(canvas, &paint);
}

void BoundingBox::drawPixmapSk(SkCanvas * const canvas,
                               SkPaint * const paint) {
    if(mTransformAnimator->getOpacity() < 0.001) return;
    paint->setFilterQuality(BoundingBox::sDisplayFiltering);
    mDrawRenderContainer.drawSk(canvas, paint);
}

void BoundingBox::setBlendModeSk(const SkBlendMode &blendMode) {
    mBlendModeSk = blendMode;
    prp_afterWholeInfluenceRangeChanged();
}

const SkBlendMode &BoundingBox::getBlendMode() {
    return mBlendModeSk;
}

void BoundingBox::resetScale() {
    mTransformAnimator->resetScale();
}

void BoundingBox::resetTranslation() {
    mTransformAnimator->resetTranslation();
}

void BoundingBox::resetRotation() {
    mTransformAnimator->resetRotation();
}

void BoundingBox::anim_setAbsFrame(const int frame) {
    const int oldRelFrame = anim_getCurrentRelFrame();
    ComplexAnimator::anim_setAbsFrame(frame);
    const int newRelFrame = anim_getCurrentRelFrame();

    if(prp_differencesBetweenRelFrames(oldRelFrame, newRelFrame)) {
        planScheduleUpdate(UpdateReason::frameChange);
    }
}

void BoundingBox::setStrokeCapStyle(const SkPaint::Cap capStyle) {
    Q_UNUSED(capStyle); }

void BoundingBox::setStrokeJoinStyle(const SkPaint::Join joinStyle) {
    Q_UNUSED(joinStyle); }

void BoundingBox::startSelectedStrokeColorTransform() {}

void BoundingBox::startSelectedFillColorTransform() {}

bool BoundingBox::diffsIncludingInherited(
        const int relFrame1, const int relFrame2) const {
    const bool diffThis = prp_differencesBetweenRelFrames(relFrame1, relFrame2);
    if(!mParentGroup || diffThis) return diffThis;
    const int absFrame1 = prp_relFrameToAbsFrame(relFrame1);
    const int absFrame2 = prp_relFrameToAbsFrame(relFrame2);
    const int parentRelFrame1 = mParentGroup->prp_absFrameToRelFrame(absFrame1);
    const int parentRelFrame2 = mParentGroup->prp_absFrameToRelFrame(absFrame2);

    const bool diffInherited = mParentGroup->diffsAffectingContainedBoxes(
                parentRelFrame1, parentRelFrame2);
    return diffThis || diffInherited;
}

bool BoundingBox::diffsIncludingInherited(const qreal relFrame1,
                                          const qreal relFrame2) const {
    const int prevFrame = qFloor(qMin(relFrame1, relFrame2));
    const int nextFrame = qCeil(qMax(relFrame1, relFrame2));
    return diffsIncludingInherited(prevFrame, nextFrame);
}

void BoundingBox::setParentTransform(BasicTransformAnimator *parent) {
    if(parent == mParentTransform) return;
    mParentTransform = parent;
    mTransformAnimator->setParentTransformAnimator(mParentTransform);
}

void BoundingBox::afterTotalTransformChanged(const UpdateReason reason) {
    updateDrawRenderContainerTransform();
    planScheduleUpdate(reason);
    requestGlobalPivotUpdateIfSelected();
}

void BoundingBox::clearParent() {
    setParentTransform(mParentGroup->getTransformAnimator());
}

void BoundingBox::setPivotRelPos(const QPointF &relPos) {
    mTransformAnimator->setPivotFixedTransform(relPos);
    requestGlobalPivotUpdateIfSelected();
}

void BoundingBox::startPivotTransform() {
    mTransformAnimator->startPivotTransform();
}

void BoundingBox::finishPivotTransform() {
    mTransformAnimator->finishPivotTransform();
}

void BoundingBox::setPivotAbsPos(const QPointF &absPos) {
    setPivotRelPos(mapAbsPosToRel(absPos));
}

QPointF BoundingBox::getPivotAbsPos() {
    return mTransformAnimator->getPivotAbs();
}

void BoundingBox::setRelBoundingRect(const QRectF& relRect) {
    mRelRect = relRect;
    mRelRectSk = toSkRect(mRelRect);
    mSkRelBoundingRectPath.reset();
    mSkRelBoundingRectPath.addRect(mRelRectSk);

    if(mCenterPivotPlanned) {
        mCenterPivotPlanned = false;
        setPivotRelPos(getRelCenterPosition());
    }
}

void BoundingBox::updateCurrentPreviewDataFromRenderData(
        BoxRenderData* renderData) {
    setRelBoundingRect(renderData->fRelBoundingRect);
}

void BoundingBox::planScheduleUpdate(const UpdateReason reason) {
    if(!isVisibleAndInVisibleDurationRect()) return;
    if(mParentGroup) mParentGroup->planScheduleUpdate(qMin(reason, reason));
    else if(!SWT_isCanvas()) return;
    if(reason != UpdateReason::frameChange) mStateId++;
    mDrawRenderContainer.setExpired(true);
    if(mSchedulePlanned) {
        mPlannedReason = qMax(reason, mPlannedReason);
        return;
    }
    mSchedulePlanned = true;
    mPlannedReason = reason;

    if(mParentScene && mParentScene->isPreviewingOrRendering()) {
        scheduleUpdate();
    }
}

void BoundingBox::scheduleUpdate() {
    if(!mSchedulePlanned) return;
    mSchedulePlanned = false;
    if(!shouldScheduleUpdate()) return;
    const int relFrame = anim_getCurrentRelFrame();
    if(hasCurrentRenderData(relFrame)) return;
    const auto currentRenderData = updateCurrentRenderData(relFrame, mPlannedReason);
    if(currentRenderData) currentRenderData->scheduleTask();
}

BoxRenderData *BoundingBox::updateCurrentRenderData(const qreal relFrame,
                                                    const UpdateReason reason) {
    const auto renderData = createRenderData();
    if(!renderData) {
        auto& tRef = *this;
        RuntimeThrow(typeid(tRef).name() + "::createRenderData returned a nullptr");
    }
    renderData->fRelFrame = relFrame;
    renderData->fReason = reason;
    mCurrentRenderDataHandler.addItemAtRelFrame(renderData);
    return renderData.get();
}

bool BoundingBox::hasCurrentRenderData(const qreal relFrame) const {
    const auto currentRenderData =
            mCurrentRenderDataHandler.getItemAtRelFrame(relFrame);
    if(currentRenderData) return true;
    if(mDrawRenderContainer.isExpired()) return false;
    const auto drawData = mDrawRenderContainer.getSrcRenderData();
    if(!drawData) return false;
    return !diffsIncludingInherited(drawData->fRelFrame, relFrame);
}

stdsptr<BoxRenderData> BoundingBox::getCurrentRenderData(const qreal relFrame) const {
    const auto currentRenderData =
            mCurrentRenderDataHandler.getItemAtRelFrame(relFrame);
    if(currentRenderData) return currentRenderData->ref<BoxRenderData>();
    if(mDrawRenderContainer.isExpired()) return nullptr;
    const auto drawData = mDrawRenderContainer.getSrcRenderData();
    if(!drawData) return nullptr;
    if(!diffsIncludingInherited(drawData->fRelFrame, relFrame)) {
        const auto copy = drawData->makeCopy();
        copy->fRelFrame = relFrame;
        return copy;
    }
    return nullptr;
}

bool BoundingBox::isContainedIn(const QRectF &absRect) const {
    return absRect.contains(getTotalTransform().mapRect(mRelRect));
}

BoundingBox *BoundingBox::getBoxAtFromAllDescendents(const QPointF &absPos) {
    if(absPointInsidePath(absPos)) return this;
    return nullptr;
}

QPointF BoundingBox::mapAbsPosToRel(const QPointF &absPos) {
    return mTransformAnimator->mapAbsPosToRel(absPos);
}

FillSettingsAnimator *BoundingBox::getFillSettings() const {
    return nullptr;
}

OutlineSettingsAnimator *BoundingBox::getStrokeSettings() const {
    return nullptr;
}

void BoundingBox::drawBoundingRect(SkCanvas * const canvas,
                                   const float invScale) {
    SkiaHelpers::drawOutlineOverlay(canvas, mSkRelBoundingRectPath,
                                    invScale, toSkMatrix(getTotalTransform()),
                                    true, MIN_WIDGET_DIM*0.25f);
}

const SkPath &BoundingBox::getRelBoundingRectPath() {
    return mSkRelBoundingRectPath;
}

QMatrix BoundingBox::getTotalTransform() const {
    return mTransformAnimator->getTotalTransform();
}

QMatrix BoundingBox::getRelativeTransformAtCurrentFrame() {
    return getRelativeTransformAtFrame(anim_getCurrentRelFrame());
}

void BoundingBox::scale(const qreal scaleBy) {
    scale(scaleBy, scaleBy);
}

void BoundingBox::scale(const qreal scaleXBy, const qreal scaleYBy) {
    mTransformAnimator->scale(scaleXBy, scaleYBy);
}

void BoundingBox::rotateBy(const qreal rot) {
    mTransformAnimator->rotateRelativeToSavedValue(rot);
}

void BoundingBox::rotateRelativeToSavedPivot(const qreal rot) {
    mTransformAnimator->rotateRelativeToSavedValue(rot,
                                                   mSavedTransformPivot);
}

void BoundingBox::scaleRelativeToSavedPivot(const qreal scaleXBy,
                                            const qreal scaleYBy) {
    mTransformAnimator->scaleRelativeToSavedValue(scaleXBy, scaleYBy,
                                                 mSavedTransformPivot);
}

void BoundingBox::scaleRelativeToSavedPivot(const qreal scaleBy) {
    scaleRelativeToSavedPivot(scaleBy, scaleBy);
}

QPointF BoundingBox::mapRelPosToAbs(const QPointF &relPos) const {
    return mTransformAnimator->mapRelPosToAbs(relPos);
}

QRectF BoundingBox::getRelBoundingRect() const {
    return mRelRect;
}

template <typename T>
void addEffectAction(const QString& text, PropertyMenu * const menu) {
    const PropertyMenu::PlainSelectedOp<BoundingBox> op = [](BoundingBox * box) {
        box->addEffect<T>();
    };
    menu->addPlainAction(text, op);
}

void BoundingBox::setupCanvasMenu(PropertyMenu * const menu) {
    Q_ASSERT(mParentScene);
    const auto parentScene = mParentScene;

    menu->addSection("Box");

    menu->addPlainAction("Create Link", [parentScene]() {
        parentScene->createLinkBoxForSelected();
    });
    menu->addPlainAction("Center Pivot", [parentScene]() {
        parentScene->centerPivotForSelected();
    });

    menu->addSeparator();

    menu->addPlainAction("Copy", [parentScene]() {
        parentScene->copyAction();
    })->setShortcut(Qt::CTRL + Qt::Key_C);

    menu->addPlainAction("Cut", [parentScene]() {
        parentScene->cutAction();
    })->setShortcut(Qt::CTRL + Qt::Key_X);

    menu->addPlainAction("Duplicate", [parentScene]() {
        parentScene->duplicateSelectedBoxes();
    })->setShortcut(Qt::CTRL + Qt::Key_D);

    menu->addPlainAction("Delete", [parentScene]() {
        parentScene->removeSelectedBoxesAndClearList();
    })->setShortcut(Qt::Key_Delete);

    menu->addSeparator();

    menu->addPlainAction("Group", [parentScene]() {
        parentScene->groupSelectedBoxes();
    })->setShortcut(Qt::CTRL + Qt::Key_G);

    menu->addPlainAction("Ungroup", [parentScene]() {
        parentScene->ungroupSelectedBoxes();
    })->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_G);

    menu->addSeparator();

    const auto RasterEffectsMenu = menu->addMenu("Raster Effects");
    CustomRasterEffectCreator::sAddToMenu(RasterEffectsMenu, &BoundingBox::addRasterEffect);
    if(!RasterEffectsMenu->isEmpty()) RasterEffectsMenu->addSeparator();
    for(const auto& creator : ShaderEffectCreator::sEffectCreators) {
        const PropertyMenu::PlainSelectedOp<BoundingBox> op =
        [creator](BoundingBox * box) {
            const auto effect = creator->create();
            box->addRasterEffect(qSharedPointerCast<RasterEffect>(effect));
        };
        RasterEffectsMenu->addPlainAction(creator->fName, op);
    }
}

void BoundingBox::moveByAbs(const QPointF &trans) {
    mTransformAnimator->moveByAbs(trans);
}

void BoundingBox::moveByRel(const QPointF &trans) {
    mTransformAnimator->moveRelativeToSavedValue(trans.x(), trans.y());
}

void BoundingBox::setAbsolutePos(const QPointF &pos) {
    setRelativePos(mParentTransform->mapAbsPosToRel(pos));
}

void BoundingBox::setRelativePos(const QPointF &relPos) {
    mTransformAnimator->setPosition(relPos.x(), relPos.y());
}

void BoundingBox::saveTransformPivotAbsPos(const QPointF &absPivot) {
    mSavedTransformPivot = mParentTransform->mapAbsPosToRel(absPivot) -
                           mTransformAnimator->getPivot();
}

void BoundingBox::startPosTransform() {
    mTransformAnimator->startPosTransform();
}

void BoundingBox::startRotTransform() {
    mTransformAnimator->startRotTransform();
}

void BoundingBox::startScaleTransform() {
    mTransformAnimator->startScaleTransform();
}

void BoundingBox::startTransform() {
    mTransformAnimator->prp_startTransform();
}

void BoundingBox::finishTransform() {
    mTransformAnimator->prp_finishTransform();
    //updateTotalTransform();
}

void BoundingBox::setupRenderData(const qreal relFrame,
                                  BoxRenderData * const data) {
    if(!mParentScene) return;
    data->fBoxStateId = mStateId;
    data->fRelFrame = qRound(relFrame);
    data->fRelTransform = getRelativeTransformAtFrame(relFrame);
    data->fTransform = getTotalTransformAtFrame(relFrame);
    data->fResolution = mParentScene->getResolutionFraction();
    data->fOpacity = mTransformAnimator->getOpacity(relFrame);
    const bool effectsVisible = mParentScene->getRasterEffectsVisible();
    data->fBaseMargin = QMargins() + 2;
    data->fBlendMode = getBlendMode();

    if(data->fOpacity > 0.001 && effectsVisible) {
        setupRasterEffectsF(relFrame, data);
    }

    bool unbound = false;
    if(mParentGroup) unbound = mParentGroup->unboundChildren();
    if(unbound) data->fMaxBoundsRect = mParentScene->getMaxBounds();
    else data->fMaxBoundsRect = mParentScene->getCurrentBounds();
}

void BoundingBox::setupRasterEffectsF(const qreal relFrame,
                                   BoxRenderData * const data) {
    mRasterEffectsAnimators->addEffects(relFrame, data);
}

void BoundingBox::addLinkingBox(BoundingBox *box) {
    mLinkingBoxes << box;
}

void BoundingBox::removeLinkingBox(BoundingBox *box) {
    mLinkingBoxes.removeOne(box);
}

const QList<BoundingBox*> &BoundingBox::getLinkingBoxes() const {
    return mLinkingBoxes;
}

void BoundingBox::incReasonsNotToApplyUglyTransform() {
    mNReasonsNotToApplyUglyTransform++;
}

void BoundingBox::decReasonsNotToApplyUglyTransform() {
    mNReasonsNotToApplyUglyTransform--;
}

bool BoundingBox::relPointInsidePath(const QPointF &relPos) const {
    return mRelRect.contains(relPos.toPoint());
}

bool BoundingBox::absPointInsidePath(const QPointF &absPoint) {
    return relPointInsidePath(mapAbsPosToRel(absPoint));
}

void BoundingBox::cancelTransform() {
    mTransformAnimator->prp_cancelTransform();
    //updateTotalTransform();
}

QPointF BoundingBox::getAbsolutePos() const {
    return QPointF(mTransformAnimator->getTotalTransform().dx(),
                   mTransformAnimator->getTotalTransform().dy());
}

void BoundingBox::updateDrawRenderContainerTransform() {
    if(mNReasonsNotToApplyUglyTransform == 0) {
        mDrawRenderContainer.updatePaintTransformGivenNewTotalTransform(
                    getTotalTransformAtFrame(anim_getCurrentRelFrame()));
    }
}

eBoxType BoundingBox::getBoxType() const { return mType; }

void BoundingBox::requestGlobalFillStrokeUpdateIfSelected() {
    if(isSelected()) emit fillStrokeSettingsChanged();
}

void BoundingBox::requestGlobalPivotUpdateIfSelected() {
    if(isSelected()) emit globalPivotInfluenced();
}

void BoundingBox::getMotionBlurProperties(QList<Property*> &list) const {
    list.append(mTransformAnimator->getScaleAnimator());
    list.append(mTransformAnimator->getPosAnimator());
    list.append(mTransformAnimator->getPivotAnimator());
    list.append(mTransformAnimator->getRotAnimator());
}

BasicTransformAnimator *BoundingBox::getTransformAnimator() const {
    return getBoxTransformAnimator();
}

BoxTransformAnimator *BoundingBox::getBoxTransformAnimator() const {
    return mTransformAnimator.get();
}

SmartVectorPath *BoundingBox::objectToVectorPathBox() { return nullptr; }

SmartVectorPath *BoundingBox::strokeToVectorPathBox() { return nullptr; }

bool BoundingBox::isAnimated() const {
    return anim_isDescendantRecording();
}

void BoundingBox::addRasterEffect(const qsptr<RasterEffect>& rasterEffect) {
    mRasterEffectsAnimators->addChild(rasterEffect);
}

void BoundingBox::removeRasterEffect(const qsptr<RasterEffect> &effect) {
    mRasterEffectsAnimators->removeChild(effect);
}

//int BoundingBox::prp_getParentFrameShift() const {
//    if(!mParentGroup) {
//        return 0;
//    } else {
//        return mParentGroup->prp_getFrameShift();
//    }
//}

QMatrix BoundingBox::getRelativeTransformAtFrame(const qreal relFrame) {
    if(isZero6Dec(relFrame - anim_getCurrentRelFrame()))
        return mTransformAnimator->getRelativeTransform();
    return mTransformAnimator->getRelativeTransformAtFrame(relFrame);
}

QMatrix BoundingBox::getTotalTransformAtFrame(const qreal relFrame) {
    if(isZero6Dec(relFrame - anim_getCurrentRelFrame()))
        return mTransformAnimator->getTotalTransform();
    return mTransformAnimator->getTotalTransformAtFrame(relFrame);
}

void BoundingBox::addPathEffect(const qsptr<PathEffect> &) {}

void BoundingBox::addFillPathEffect(const qsptr<PathEffect> &) {}

void BoundingBox::addOutlineBasePathEffect(const qsptr<PathEffect> &) {}

void BoundingBox::addOutlinePathEffect(const qsptr<PathEffect> &) {}

void BoundingBox::removePathEffect(const qsptr<PathEffect> &) {}

void BoundingBox::removeFillPathEffect(const qsptr<PathEffect> &) {}

void BoundingBox::removeOutlinePathEffect(const qsptr<PathEffect> &) {}

#include <QInputDialog>
void BoundingBox::setupTreeViewMenu(PropertyMenu * const menu) {
    const auto parentWidget = menu->getParentWidget();
    menu->addPlainAction("Rename", [this, parentWidget]() {
        bool ok;
        const QString text = QInputDialog::getText(parentWidget, tr("New name dialog"),
                                                   tr("Name:"), QLineEdit::Normal,
                                                   prp_getName(), &ok);
        if(ok) prp_setName(text);
    });

    const PropertyMenu::CheckSelectedOp<BoundingBox> visRangeOp =
    [](BoundingBox* const box, const bool checked) {
        if(box->mDurationRectangleLocked) return;
        const bool hasDur = box->hasDurationRectangle();
        if(hasDur == checked) return;
        if(checked) box->createDurationRectangle();
        else box->setDurationRectangle(nullptr);
    };

    menu->addCheckableAction("Visibility Range",
                             hasDurationRectangle(),
                             visRangeOp);
    menu->addPlainAction("Visibility Range Settings...",
                         [this, parentWidget]() {
        mDurationRectangle->openDurationSettingsDialog(parentWidget);
    })->setEnabled(mDurationRectangle);

    setupCanvasMenu(menu->addMenu("Actions"));
}



FrameRange BoundingBox::getFirstAndLastIdenticalForMotionBlur(
        const int relFrame, const bool takeAncestorsIntoAccount) {
    FrameRange range{FrameRange::EMIN, FrameRange::EMAX};
    if(mVisible) {
        if(isFrameInDurationRect(relFrame)) {
            QList<Property*> propertiesT;
            getMotionBlurProperties(propertiesT);
            for(const auto& child : propertiesT) {
                if(range.isUnary()) break;
                auto childRange = child->prp_getIdenticalRelRange(relFrame);
                range *= childRange;
            }

            range *= mDurationRectangle->getRelFrameRange();
        } else {
            if(relFrame > mDurationRectangle->getMaxFrameAsRelFrame()) {
                return mDurationRectangle->getAbsFrameRangeToTheRight();
            } else if(relFrame < mDurationRectangle->getMinFrameAsRelFrame()) {
                return mDurationRectangle->getAbsFrameRangeToTheLeft();
            }
        }
    } else {
        return {FrameRange::EMIN, FrameRange::EMAX};
    }
    if(!mParentGroup || takeAncestorsIntoAccount) return range;
    if(range.isUnary()) return range;
    const int parentRel = mParentGroup->prp_absFrameToRelFrame(prp_relFrameToAbsFrame(relFrame));
    auto parentRange = mParentGroup->BoundingBox::getFirstAndLastIdenticalForMotionBlur(parentRel);

    return range*parentRange;
}

void BoundingBox::cancelWaitingTasks() {
    for(const auto &task : mScheduledTasks) task->cancel();
    mScheduledTasks.clear();
}

void BoundingBox::queScheduledTasks() {
    scheduleUpdate();
    const auto taskScheduler = TaskScheduler::sGetInstance();
    for(const auto& task : mScheduledTasks)
        taskScheduler->queCPUTask(task);
    mScheduledTasks.clear();
}

void BoundingBox::writeIdentifier(QIODevice * const dst) const {
    dst->write(rcConstChar(&mType), sizeof(eBoxType));
}

int BoundingBox::getReadId() const {
    return mReadId;
}

int BoundingBox::getWriteId() const {
    return mWriteId;
}

int BoundingBox::assignWriteId() {
    mWriteId = sNextWriteId++;
    sBoxesWithWriteIds << this;
    return mWriteId;
}

void BoundingBox::clearReadId() {
    mReadId = -1;
}

void BoundingBox::clearWriteId() {
    mWriteId = -1;
}

BoundingBox *BoundingBox::sGetBoxByReadId(const int readId) {
    for(const auto& box : sReadBoxes) {
        if(box->getReadId() == readId) return box;
    }
    return nullptr;
}

void BoundingBox::sAddWaitingForBoxLoad(const WaitingForBoxLoad& func) {
    sFunctionsWaitingForBoxRead << func;
}

void BoundingBox::sAddReadBox(BoundingBox * const box) {
    sReadBoxes << box;
    for(int i = 0; i < sFunctionsWaitingForBoxRead.count(); i++) {
        auto funcT = sFunctionsWaitingForBoxRead.at(i);
        if(funcT.boxRead(box)) {
            sFunctionsWaitingForBoxRead.removeAt(i);
            i--;
        }
    }
}

void BoundingBox::sClearWriteBoxes() {
    for(const auto& box : sBoxesWithWriteIds) {
        box->clearWriteId();
    }
    sNextWriteId = 0;
    sBoxesWithWriteIds.clear();
}

void BoundingBox::sClearReadBoxes() {
    for(const auto& box : sReadBoxes) {
        box->clearReadId();
    }
    sReadBoxes.clear();
    for(const auto& func : sFunctionsWaitingForBoxRead) {
        func.boxNeverRead();
    }
    sFunctionsWaitingForBoxRead.clear();
}

void BoundingBox::selectAndAddContainedPointsToList(
        const QRectF &absRect, QList<MovablePoint*> &selection,
        const CanvasMode mode) {
    for(const auto& desc : mCanvasProps) {
        const auto handler = desc->getPointsHandler();
        if(!handler) continue;
        handler->addInRectForSelection(absRect, selection, mode);
    }
}

void BoundingBox::selectAllCanvasPts(QList<MovablePoint*> &selection,
                                     const CanvasMode mode) {
    for(const auto& desc : mCanvasProps) {
        const auto handler = desc->getPointsHandler();
        if(!handler) continue;
        handler->addAllPointsToSelection(selection, mode);
    }
}

void BoundingBox::scheduleTask(const stdsptr<BoxRenderData>& task) {
    mScheduledTasks << task;
}

bool BoundingBox::isParentLinkBox() {
    return mParentGroup->SWT_isLinkBox();
}

bool BoundingBox::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                      const bool parentSatisfies,
                                      const bool parentMainTarget) const {
    const SWT_BoxRule rule = rules.fRule;
    bool satisfies = false;
    const bool alwaysShowChildren = rules.fAlwaysShowChildren;
    if(rules.fType == SWT_TYPE_SOUND) return false;
    if(alwaysShowChildren) {
        if(rule == SWT_BR_ALL) {
            satisfies = parentSatisfies;
        } else if(rule == SWT_BR_SELECTED) {
            satisfies = isSelected() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_BR_ANIMATED) {
            satisfies = isAnimated() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_BR_NOT_ANIMATED) {
            satisfies = !isAnimated() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_BR_VISIBLE) {
            satisfies = isVisible() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_BR_HIDDEN) {
            satisfies = !isVisible() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_BR_LOCKED) {
            satisfies = isLocked() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_BR_UNLOCKED) {
            satisfies = !isLocked() ||
                    (parentSatisfies && !parentMainTarget);
        }
    } else {
        if(rule == SWT_BR_ALL) {
            satisfies = parentSatisfies;
        } else if(rule == SWT_BR_SELECTED) {
            satisfies = isSelected();
        } else if(rule == SWT_BR_ANIMATED) {
            satisfies = isAnimated();
        } else if(rule == SWT_BR_NOT_ANIMATED) {
            satisfies = !isAnimated();
        } else if(rule == SWT_BR_VISIBLE) {
            satisfies = isVisible() && parentSatisfies;
        } else if(rule == SWT_BR_HIDDEN) {
            satisfies = !isVisible() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_BR_LOCKED) {
            satisfies = isLocked() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_BR_UNLOCKED) {
            satisfies = !isLocked() && parentSatisfies;
        }
    }
    if(satisfies) {
        const QString &nameSearch = rules.fSearchString;
        if(!nameSearch.isEmpty()) {
            satisfies = prp_mName.contains(nameSearch);
        }
    }
    return satisfies;
}

bool BoundingBox::SWT_visibleOnlyIfParentDescendant() const {
    return false;
}

bool BoundingBox::SWT_dropSupport(const QMimeData * const data) {
    return mRasterEffectsAnimators->SWT_dropSupport(data);
}

bool BoundingBox::SWT_drop(const QMimeData * const data) {
    if(mRasterEffectsAnimators->SWT_dropSupport(data))
        return mRasterEffectsAnimators->SWT_drop(data);
    return false;
}

void BoundingBox::renderDataFinished(BoxRenderData *renderData) {
    auto currentRenderData = mDrawRenderContainer.getSrcRenderData();
    bool newerSate = true;
    bool closerFrame = true;
    if(currentRenderData) {
        newerSate = currentRenderData->fBoxStateId < renderData->fBoxStateId;
        const qreal finishedFrameDist =
                qAbs(anim_getCurrentRelFrame() - renderData->fRelFrame);
        const qreal oldFrameDist =
                qAbs(anim_getCurrentRelFrame() - currentRenderData->fRelFrame);
        closerFrame = finishedFrameDist < oldFrameDist;
    }
    if(newerSate || closerFrame) {
        mDrawRenderContainer.setSrcRenderData(renderData);
        const bool currentState = renderData->fBoxStateId == mStateId;
        const bool currentFrame = isZero4Dec(renderData->fRelFrame - anim_getCurrentRelFrame());
        const bool expired = !currentState || !currentFrame;
        mDrawRenderContainer.setExpired(expired);
        if(expired) updateDrawRenderContainerTransform();
    }
}