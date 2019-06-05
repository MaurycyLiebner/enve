#include "Boxes/boundingbox.h"
#include "Boxes/containerbox.h"
#include "canvas.h"
#include "singlewidgetabstraction.h"
#include "durationrectangle.h"
#include "pointhelpers.h"
#include "skia/skqtconversions.h"
#include "global.h"
#include "MovablePoints/movablepoint.h"
#include "PropertyUpdaters/pixmapeffectupdater.h"
#include "taskscheduler.h"
#include "PixmapEffects/pixmapeffect.h"
#include "Animators/effectanimators.h"
#include "Animators/gpueffectanimators.h"
#include "Animators/transformanimator.h"
#include "GPUEffects/gpurastereffect.h"
#include "linkbox.h"
#include "PropertyUpdaters/transformupdater.h"
#include "PropertyUpdaters/boxpathpointupdater.h"
#include "Animators/qpointfanimator.h"
#include "MovablePoints/pathpointshandler.h"

SkFilterQuality BoundingBox::sDisplayFiltering = kLow_SkFilterQuality;
int BoundingBox::sNextDocumentId;
QList<BoundingBox*> BoundingBox::sDocumentBoxes;

QList<BoundingBox*> BoundingBox::sReadBoxes;
QList<WaitingForBoxLoad> BoundingBox::sFunctionsWaitingForBoxRead;

int BoundingBox::sNextWriteId;
QList<BoundingBox*> BoundingBox::sBoxesWithWriteIds;

BoundingBox::BoundingBox(const BoundingBoxType &type) :
    ComplexAnimator("box"),
    mDocumentId(sNextDocumentId++), mType(type),
    mTransformAnimator(SPtrCreate(BoxTransformAnimator)()),
    mEffectsAnimators(SPtrCreate(EffectAnimators)(this)),
    mGPUEffectsAnimators(SPtrCreate(GPUEffectAnimators)(this)) {
    sDocumentBoxes << this;
    ca_addChildAnimator(mTransformAnimator);
    mTransformAnimator->prp_setOwnUpdater(
                SPtrCreate(TransformUpdater)(mTransformAnimator.get()));
    const auto pivotAnim = mTransformAnimator->getPivotAnimator();
    const auto pivotUpdater = SPtrCreate(BoxPathPointUpdater)(
                mTransformAnimator.get(), this);
    pivotAnim->prp_setOwnUpdater(pivotUpdater);

    mEffectsAnimators->prp_setOwnUpdater(
                SPtrCreate(PixmapEffectUpdater)(this));
    ca_addChildAnimator(mEffectsAnimators);
    mEffectsAnimators->SWT_hide();

    mGPUEffectsAnimators->prp_setOwnUpdater(
                SPtrCreate(PixmapEffectUpdater)(this));
    ca_prependChildAnimator(mEffectsAnimators.data(), mGPUEffectsAnimators);
    mGPUEffectsAnimators->SWT_hide();

    connect(mTransformAnimator.get(),
            &BoxTransformAnimator::totalTransformChanged,
            this, &BoundingBox::afterTotalTransformChanged);
}

BoundingBox::~BoundingBox() {
    sDocumentBoxes.removeOne(this);
}

BoundingBox *BoundingBox::sGetBoxByDocumentId(const int &documentId) {
    for(const auto& box : sDocumentBoxes) {
        if(box->getDocumentId() == documentId) return box;
    }
    return nullptr;
}

void BoundingBox::prp_afterChangedAbsRange(const FrameRange &range) {
    const auto visRange = getVisibleAbsFrameRange();
    const auto croppedRange = visRange*range;
    Property::prp_afterChangedAbsRange(croppedRange);
    if(croppedRange.inRange(anim_getCurrentAbsFrame())) {
        planScheduleUpdate(Animator::USER_CHANGE);
    }
}

void BoundingBox::ca_childAnimatorIsRecordingChanged() {
    ComplexAnimator::ca_childAnimatorIsRecordingChanged();
    SWT_scheduleWidgetsContentUpdateWithRule(SWT_BR_ANIMATED);
    SWT_scheduleWidgetsContentUpdateWithRule(SWT_BR_NOT_ANIMATED);
}

qsptr<BoundingBox> BoundingBox::createLink() {
    auto linkBox = SPtrCreate(InternalLinkBox)(this);
    copyBoundingBoxDataTo(linkBox.get());
    return std::move(linkBox);
}

qsptr<BoundingBox> BoundingBox::createLinkForLinkGroup() {
    qsptr<BoundingBox> box = createLink();
    box->clearRasterEffects();
    return box;
}

void BoundingBox::clearRasterEffects() {
    mEffectsAnimators->ca_removeAllChildAnimators();
}

void BoundingBox::setFont(const QFont &) {}

void BoundingBox::setSelectedFontSize(const qreal &) {}

void BoundingBox::setSelectedFontFamilyAndStyle(const QString &, const QString &) {}

QPointF BoundingBox::getRelCenterPosition() {
    return mRelBoundingRect.center();
}

void BoundingBox::centerPivotPosition() {
    mTransformAnimator->setPivotFixedTransform(
                getRelCenterPosition());
}

void BoundingBox::planCenterPivotPosition() {
    mCenterPivotPlanned = true;
}

void BoundingBox::updateIfUsesProgram(
        const GPURasterEffectProgram * const program) const {
    mGPUEffectsAnimators->updateIfUsesProgram(program);
}

void BoundingBox::copyBoundingBoxDataTo(BoundingBox * const targetBox) {
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    BoundingBox::writeBoundingBox(&buffer);
    if(buffer.seek(sizeof(BoundingBoxType))) {
        targetBox->BoundingBox::readBoundingBox(&buffer);
    }
    buffer.close();
}

void BoundingBox::drawHoveredSk(SkCanvas *canvas, const SkScalar &invScale) {
    drawHoveredPathSk(canvas, mSkRelBoundingRectPath, invScale);
}

void BoundingBox::drawHoveredPathSk(SkCanvas *canvas,
                                    const SkPath &path,
                                    const SkScalar &invScale) {
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

void BoundingBox::setGPUEffectsEnabled(const bool &enable) {
    mGPUEffectsAnimators->SWT_setEnabled(enable);
    mGPUEffectsAnimators->SWT_setVisible(
                mGPUEffectsAnimators->hasChildAnimators() || enable);
}

bool BoundingBox::getGPUEffectsEnabled() const {
    return mGPUEffectsAnimators->SWT_isEnabled();
}

void BoundingBox::setRasterEffectsEnabled(const bool &enable) {
    mEffectsAnimators->SWT_setEnabled(enable);
    mEffectsAnimators->SWT_setVisible(
                mEffectsAnimators->hasChildAnimators() || enable);
}

bool BoundingBox::getRasterEffectsEnabled() const {
    return mEffectsAnimators->SWT_isEnabled();
}

void BoundingBox::applyPaintSetting(const PaintSettingsApplier &setting) {
    Q_UNUSED(setting);
}

void BoundingBox::setFillColorMode(const ColorMode &colorMode) {
    Q_UNUSED(colorMode);
}

void BoundingBox::setStrokeColorMode(const ColorMode &colorMode) {
    Q_UNUSED(colorMode);
}

bool BoundingBox::isAncestor(const BoundingBox * const box) const {
    if(!mParentGroup) return false;
    if(mParentGroup == box) return true;
    if(box->SWT_isContainerBox()) return mParentGroup->isAncestor(box);
    return false;
}

Canvas *BoundingBox::getParentCanvas() {
    if(!mParentGroup) return nullptr;
    return mParentGroup->getParentCanvas();
}

void BoundingBox::reloadCacheHandler() { prp_afterWholeInfluenceRangeChanged(); }

bool BoundingBox::SWT_isBoundingBox() const { return true; }

void BoundingBox::updateAllBoxes(const UpdateReason &reason) {
    planScheduleUpdate(reason);
}

void BoundingBox::drawAllCanvasControls(SkCanvas * const canvas,
                                        const CanvasMode &mode,
                                        const SkScalar &invScale) {
    for(const auto& prop : mCanvasProps)
        prop->drawCanvasControls(canvas, mode, invScale);
}

FrameRange BoundingBox::prp_relInfluenceRange() const {
    if(mDurationRectangle)
        return mDurationRectangle->getAbsFrameRange();
    return ComplexAnimator::prp_relInfluenceRange();
}

MovablePoint *BoundingBox::getPointAtAbsPos(const QPointF &absPos,
                                            const CanvasMode &mode,
                                            const qreal &invScale) const {

    for(const auto& prop : mCanvasProps) {
        const auto handler = prop->getPointsHandler();
        if(!handler) continue;
        const auto pt = handler->getPointAtAbsPos(absPos, mode, invScale);
        if(pt) return pt;
    }
    return nullptr;
}

NormalSegment BoundingBox::getNormalSegment(const QPointF &absPos,
                                            const qreal &invScale) const {
    for(const auto& prop : mCanvasProps) {
        const auto handler = prop->getPointsHandler();
        if(!handler) continue;
        const auto pathHandler = dynamic_cast<PathPointsHandler*>(handler);
        if(!pathHandler) continue;
        const auto seg = pathHandler->getNormalSegment(absPos, invScale);
        if(seg.isValid()) return seg;
    }
    return NormalSegment();
}

void BoundingBox::drawPixmapSk(SkCanvas * const canvas,
                               GrContext * const grContext) {
    if(mTransformAnimator->getOpacity() < 0.001) return;
    SkPaint paint;
    const int intAlpha = qRound(mTransformAnimator->getOpacity()*2.55);
    paint.setAlpha(static_cast<U8CPU>(intAlpha));
    paint.setBlendMode(mBlendModeSk);
    paint.setFilterQuality(sDisplayFiltering);
    drawPixmapSk(canvas, &paint, grContext);
}

void BoundingBox::drawPixmapSk(SkCanvas * const canvas,
                               SkPaint * const paint,
                               GrContext* const grContext) {
    if(mTransformAnimator->getOpacity() < 0.001) return;
    paint->setFilterQuality(sDisplayFiltering);
    mDrawRenderContainer.drawSk(canvas, paint, grContext);
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

void BoundingBox::anim_setAbsFrame(const int &frame) {
    const int oldRelFrame = anim_getCurrentRelFrame();
    ComplexAnimator::anim_setAbsFrame(frame);
    const int newRelFrame = anim_getCurrentRelFrame();

    if(prp_differencesBetweenRelFrames(oldRelFrame, newRelFrame)) {
        planScheduleUpdate(Animator::FRAME_CHANGE);
    }
}

void BoundingBox::startAllPointsTransform() {}

void BoundingBox::finishAllPointsTransform() {}

void BoundingBox::setStrokeCapStyle(const Qt::PenCapStyle &capStyle) {
    Q_UNUSED(capStyle); }

void BoundingBox::setStrokeJoinStyle(const Qt::PenJoinStyle &joinStyle) {
    Q_UNUSED(joinStyle); }

void BoundingBox::setStrokeWidth(const qreal &strokeWidth) {
    Q_UNUSED(strokeWidth);
}

void BoundingBox::startSelectedStrokeWidthTransform() {}

void BoundingBox::startSelectedStrokeColorTransform() {}

void BoundingBox::startSelectedFillColorTransform() {}

bool BoundingBox::diffsIncludingInherited(
        const int &relFrame1, const int &relFrame2) {
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

void BoundingBox::setParentGroup(ContainerBox * const parent) {
    if(parent == mParentGroup) return;
    mParentGroup = parent;
    mParent = parent;
    if(!mParentGroup) return;
    anim_setAbsFrame(mParentGroup->anim_getCurrentAbsFrame());
    setParentTransform(parent->getTransformAnimator());
}

void BoundingBox::setParentTransform(BasicTransformAnimator *parent) {
    if(parent == mParentTransform) return;
    mParentTransform = parent;
    mTransformAnimator->setParentTransformAnimator(mParentTransform);
}

void BoundingBox::afterTotalTransformChanged(const UpdateReason &reason) {
    updateDrawRenderContainerTransform();
    planScheduleUpdate(reason);
    requestGlobalPivotUpdateIfSelected();
}

void BoundingBox::clearParent() {
    setParentTransform(mParentGroup->getTransformAnimator());
}

ContainerBox *BoundingBox::getParentGroup() const {
    return mParentGroup;
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

void BoundingBox::select() {
    mSelected = true;
    SWT_scheduleWidgetsContentUpdateWithRule(SWT_BR_SELECTED);
}

void BoundingBox::updateRelBoundingRectFromRenderData(
        BoundingBoxRenderData * const renderData) {
    mRelBoundingRect = renderData->fRelBoundingRect;
    mRelBoundingRectSk = toSkRect(mRelBoundingRect);
    mSkRelBoundingRectPath.reset();
    mSkRelBoundingRectPath.addRect(mRelBoundingRectSk);

    if(mCenterPivotPlanned) {
        mCenterPivotPlanned = false;
        setPivotRelPos(getRelCenterPosition());
    }
}

void BoundingBox::updateCurrentPreviewDataFromRenderData(
        BoundingBoxRenderData* renderData) {
    updateRelBoundingRectFromRenderData(renderData);
}

void BoundingBox::planScheduleUpdate(const UpdateReason& reason) {
    if(!isVisibleAndInVisibleDurationRect()) return;
    if(!shouldPlanScheduleUpdate()) return;
    if(mParentGroup) {
        mParentGroup->planScheduleUpdate(qMin(reason, CHILD_USER_CHANGE));
    } else if(!SWT_isCanvas()) return;
    if(reason != UpdateReason::FRAME_CHANGE) mStateId++;

    mDrawRenderContainer.setExpired(true);
    if(mSchedulePlanned) {
        mPlannedReason = qMax(reason, mPlannedReason);
        return;
    }
    mSchedulePlanned = true;
    mPlannedReason = reason;

    const auto parentCanvas = getParentCanvas();
    if(!parentCanvas) return;
    if(parentCanvas->isPreviewingOrRendering()) {
        scheduleUpdate();
    }
}

void BoundingBox::scheduleUpdate() {
    if(!mSchedulePlanned) return;
    mSchedulePlanned = false;
    const int relFrame = anim_getCurrentRelFrame();
    auto currentRenderData = getCurrentRenderData(relFrame);
    if(currentRenderData) return;
    currentRenderData = updateCurrentRenderData(relFrame, mPlannedReason);
    currentRenderData->scheduleTask();
}

BoundingBoxRenderData *BoundingBox::updateCurrentRenderData(
        const int& relFrame, const UpdateReason& reason) {
    const auto renderData = createRenderData();
    if(!renderData) RuntimeThrow("Failed to create BoundingBoxRenderData instance");
    renderData->fRelFrame = relFrame;
    renderData->fReason = reason;
    mCurrentRenderDataHandler.addItemAtRelFrame(renderData);
    return renderData.get();
}

BoundingBoxRenderData *BoundingBox::getCurrentRenderData(const int& relFrame) {
    auto currentRenderData =
            mCurrentRenderDataHandler.getItemAtRelFrame(relFrame);
    if(!currentRenderData) {
        if(mDrawRenderContainer.isExpired()) return nullptr;
        currentRenderData = mDrawRenderContainer.getSrcRenderData();
        if(!currentRenderData) return nullptr;
//        if(currentRenderData->fRelFrame == relFrame) {
        if(!diffsIncludingInherited(currentRenderData->fRelFrame, relFrame)) {
            auto copy = currentRenderData->makeCopy();
            copy->fRelFrame = relFrame;
            mCurrentRenderDataHandler.addItemAtRelFrame(copy);
            return copy.get();
        }
        return nullptr;
    }
    return currentRenderData;
}


void BoundingBox::nullifyCurrentRenderData(const int& relFrame) {
    mCurrentRenderDataHandler.removeItemAtRelFrame(relFrame);
}

void BoundingBox::deselect() {
    mSelected = false;

    SWT_scheduleWidgetsContentUpdateWithRule(SWT_BR_SELECTED);
}

bool BoundingBox::isContainedIn(const QRectF &absRect) const {
    return absRect.contains(getTotalTransform().mapRect(mRelBoundingRect));
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
                                   const SkScalar &invScale) {
    SkiaHelpers::drawOutlineOverlay(canvas, mSkRelBoundingRectPath,
                                    invScale, toSkMatrix(getTotalTransform()),
                                    true, MIN_WIDGET_HEIGHT*0.25f);
}

const SkPath &BoundingBox::getRelBoundingRectPath() {
    return mSkRelBoundingRectPath;
}

QMatrix BoundingBox::getTotalTransform() const {
    return mTransformAnimator->getTotalTransform();
}

QMatrix BoundingBox::getRelativeTransformAtCurrentFrame() {
    return getRelativeTransformAtRelFrameF(anim_getCurrentRelFrame());
}

void BoundingBox::scale(const qreal &scaleBy) {
    scale(scaleBy, scaleBy);
}

void BoundingBox::scale(const qreal &scaleXBy, const qreal &scaleYBy) {
    mTransformAnimator->scale(scaleXBy, scaleYBy);
}

void BoundingBox::rotateBy(const qreal &rot) {
    mTransformAnimator->rotateRelativeToSavedValue(rot);
}

void BoundingBox::rotateRelativeToSavedPivot(const qreal &rot) {
    mTransformAnimator->rotateRelativeToSavedValue(rot,
                                                   mSavedTransformPivot);
}

void BoundingBox::scaleRelativeToSavedPivot(const qreal &scaleXBy,
                                            const qreal &scaleYBy) {
    mTransformAnimator->scaleRelativeToSavedValue(scaleXBy, scaleYBy,
                                                 mSavedTransformPivot);
}

void BoundingBox::scaleRelativeToSavedPivot(const qreal &scaleBy) {
    scaleRelativeToSavedPivot(scaleBy, scaleBy);
}

QPointF BoundingBox::mapRelPosToAbs(const QPointF &relPos) const {
    return mTransformAnimator->mapRelPosToAbs(relPos);
}

QRectF BoundingBox::getRelBoundingRect() const {
    return mRelBoundingRect;
}

QRectF BoundingBox::getRelBoundingRect(const qreal &relFrame) {
    Q_UNUSED(relFrame);
    return getRelBoundingRect();
}

#include "typemenu.h"
#include "PixmapEffects/pixmapeffectsinclude.h"
template <typename T>
void addEffectAction(const QString& text,
                     BoxTypeMenu * const menu) {
    const BoxTypeMenu::PlainOp<BoundingBox> op = [](BoundingBox * box) {
        box->addEffect<T>();
    };
    menu->addPlainAction(text, op);
}

#include "patheffectsmenu.h"
void BoundingBox::addActionsToMenu(BoxTypeMenu * const menu) {
    menu->addSeparator();

//    if(SWT_isPathBox() || SWT_isContainerBox()) {
        PathEffectsMenu::addPathEffectsToActionMenu(menu);
//    }

    menu->addSeparator();

//    if(!SWT_isGroupBox()) {
        const auto effectsMenu = menu->addMenu("Effects");
        addEffectAction<BlurEffect>("Blur", effectsMenu);
        addEffectAction<SampledMotionBlurEffect>("Motion Blur", effectsMenu);
        addEffectAction<ShadowEffect>("Shadow", effectsMenu);
        addEffectAction<DesaturateEffect>("Desaturate", effectsMenu);
        addEffectAction<ColorizeEffect>("Colorize", effectsMenu);
        addEffectAction<ContrastEffect>("Contrast", effectsMenu);
        addEffectAction<BrightnessEffect>("Brightness", effectsMenu);
        addEffectAction<ReplaceColorEffect>("Replace Color", effectsMenu);

        const auto gpuEffectsMenu = menu->addMenu("GPU Effects");
        for(const auto& creator : GPURasterEffectCreator::sEffectCreators) {
            const BoxTypeMenu::PlainOp<BoundingBox> op =
            [creator](BoundingBox * box) {
                const auto effect = GetAsSPtr(creator->create(), GPURasterEffect);
                box->addGPUEffect(effect);
            };
            gpuEffectsMenu->addPlainAction(creator->fName, op);
        }

        menu->addSeparator();
//    }
}


void BoundingBox::moveByAbs(const QPointF &trans) {
    mTransformAnimator->moveByAbs(trans);
    //    QPointF by = mParent->mapAbsPosToRel(trans) -
//                 mParent->mapAbsPosToRel(QPointF(0., 0.));
// //    QPointF by = mapAbsPosToRel(
// //                trans - mapRelativeToAbsolute(QPointF(0., 0.)));

//    moveByRel(by);
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

qreal BoundingBox::getEffectsMarginAtRelFrame(const int &relFrame) {
    return mEffectsAnimators->getEffectsMarginAtRelFrame(relFrame);
}

qreal BoundingBox::getEffectsMarginAtRelFrameF(const qreal &relFrame) {
    return mEffectsAnimators->getEffectsMarginAtRelFrameF(relFrame);
}

void BoundingBox::setupRenderData(const qreal &relFrame,
                                  BoundingBoxRenderData * const data) {
    data->fBoxStateId = mStateId;
    data->fRelFrame = qRound(relFrame);
    data->fTransform = getTotalTransformAtRelFrameF(relFrame);
    data->fOpacity = mTransformAnimator->getOpacity(relFrame);
    data->fResolution = getParentCanvas()->getResolutionFraction();
    const bool effectsVisible = getParentCanvas()->getRasterEffectsVisible();
    if(effectsVisible) {
        data->fEffectsMargin = getEffectsMarginAtRelFrameF(relFrame)*
                data->fResolution + 2;
    } else {
        data->fEffectsMargin = 2;
    }
    data->fBlendMode = getBlendMode();

    const auto parentCanvas = getParentCanvas();
    data->fMaxBoundsRect = parentCanvas->getMaxBoundsRect();
    if(data->fOpacity > 0.001 && effectsVisible) {
        setupEffectsF(relFrame, data);
        setupGPUEffectsF(relFrame, data);
    }
    if(data->fParentIsTarget && !data->nullifyBeforeProcessing()) {
        nullifyCurrentRenderData(data->fRelFrame);
    }
}

stdsptr<BoundingBoxRenderData> BoundingBox::createRenderData() { return nullptr; }

void BoundingBox::setupEffectsF(const qreal &relFrame,
                                BoundingBoxRenderData * const data) {
    mEffectsAnimators->addEffectRenderDataToListF(relFrame, data);
}

void BoundingBox::setupGPUEffectsF(const qreal &relFrame,
                                   BoundingBoxRenderData * const data) {
    mGPUEffectsAnimators->addEffectRenderDataToListF(relFrame, data);
}

void BoundingBox::addLinkingBox(BoundingBox *box) {
    mLinkingBoxes << box;
}

void BoundingBox::removeLinkingBox(BoundingBox *box) {
    mLinkingBoxes.removeOne(box);
}

const QList<qptr<BoundingBox>> &BoundingBox::getLinkingBoxes() const {
    return mLinkingBoxes;
}

EffectAnimators *BoundingBox::getEffectsAnimators() {
    return mEffectsAnimators.data();
}

void BoundingBox::incReasonsNotToApplyUglyTransform() {
    mNReasonsNotToApplyUglyTransform++;
}

void BoundingBox::decReasonsNotToApplyUglyTransform() {
    mNReasonsNotToApplyUglyTransform--;
}

bool BoundingBox::isSelected() const { return mSelected; }

bool BoundingBox::relPointInsidePath(const QPointF &relPos) const {
    return mRelBoundingRect.contains(relPos.toPoint());
}

bool BoundingBox::absPointInsidePath(const QPointF &absPoint) {
    return relPointInsidePath(mapAbsPosToRel(absPoint));
}

void BoundingBox::cancelTransform() {
    mTransformAnimator->prp_cancelTransform();
    //updateTotalTransform();
}

void BoundingBox::moveUp() {
    mParentGroup->increaseContainedBoxZInList(this);
}

void BoundingBox::moveDown() {
    mParentGroup->decreaseContainedBoxZInList(this);
}

void BoundingBox::bringToFront() {
    mParentGroup->bringContainedBoxToEndList(this);
}

void BoundingBox::bringToEnd() {
    mParentGroup->bringContainedBoxToFrontList(this);
}

void BoundingBox::setZListIndex(const int &z) {
    mZListIndex = z;
}

int BoundingBox::getZIndex() const {
    return mZListIndex;
}

QPointF BoundingBox::getAbsolutePos() const {
    return QPointF(mTransformAnimator->getTotalTransform().dx(),
                   mTransformAnimator->getTotalTransform().dy());
}

void BoundingBox::updateDrawRenderContainerTransform() {
    if(mNReasonsNotToApplyUglyTransform == 0) {
        mDrawRenderContainer.updatePaintTransformGivenNewTotalTransform(
                    getTotalTransformAtRelFrameF(anim_getCurrentRelFrame()));
    }
}

const BoundingBoxType &BoundingBox::getBoxType() const { return mType; }

void BoundingBox::startDurationRectPosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->startPosTransform();
    }
}

void BoundingBox::finishDurationRectPosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->finishPosTransform();
    }
}

void BoundingBox::moveDurationRect(const int &dFrame) {
    if(hasDurationRectangle()) {
        mDurationRectangle->changeFramePosBy(dFrame);
    }
}

void BoundingBox::startMinFramePosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->startMinFramePosTransform();
    }
}

void BoundingBox::finishMinFramePosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->finishMinFramePosTransform();
    }
}

void BoundingBox::moveMinFrame(const int &dFrame) {
    if(hasDurationRectangle()) {
        mDurationRectangle->moveMinFrame(dFrame);
    }
}

void BoundingBox::startMaxFramePosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->startMaxFramePosTransform();
    }
}

void BoundingBox::finishMaxFramePosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->finishMaxFramePosTransform();
    }
}

void BoundingBox::moveMaxFrame(const int &dFrame) {
    if(hasDurationRectangle()) {
        mDurationRectangle->moveMaxFrame(dFrame);
    }
}

DurationRectangle *BoundingBox::getDurationRectangle() {
    return mDurationRectangle.get();
}

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

void BoundingBox::selectionChangeTriggered(const bool &shiftPressed) {
    const auto parentCanvas = getParentCanvas();
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

bool BoundingBox::isAnimated() const {
    return anim_isDescendantRecording();
}

void BoundingBox::addGPUEffect(const qsptr<GPURasterEffect>& rasterEffect) {
    mGPUEffectsAnimators->addEffect(rasterEffect);
}

void BoundingBox::removeGPUEffect(const qsptr<GPURasterEffect>& effect) {
    mGPUEffectsAnimators->ca_removeChildAnimator(effect);
    if(!mGPUEffectsAnimators->hasChildAnimators()) {
        mGPUEffectsAnimators->SWT_hide();
    }

    prp_afterWholeInfluenceRangeChanged();
}

void BoundingBox::addEffect(const qsptr<PixmapEffect>& effect) {
    mEffectsAnimators->addEffect(effect);
}

void BoundingBox::removeEffect(const qsptr<PixmapEffect>& effect) {
    mEffectsAnimators->ca_removeChildAnimator(effect);
    if(!mEffectsAnimators->hasChildAnimators()) {
        mEffectsAnimators->SWT_hide();
    }

    prp_afterWholeInfluenceRangeChanged();
}

//int BoundingBox::prp_getParentFrameShift() const {
//    if(!mParentGroup) {
//        return 0;
//    } else {
//        return mParentGroup->prp_getFrameShift();
//    }
//}

bool BoundingBox::hasDurationRectangle() const {
    return mDurationRectangle;
}

void BoundingBox::createDurationRectangle() {
    const auto durRect = SPtrCreate(DurationRectangle)(this);
//    durRect->setMinFrame(0);
//    const auto parentCanvas = getParentCanvas();
//    if(parentCanvas) {
//        durRect->setFramesDuration(getParentCanvas()->getFrameCount());
//    }
    durRect->setMinFrame(anim_getCurrentRelFrame() - 5);
    durRect->setFramesDuration(10);
    setDurationRectangle(durRect);
}

void BoundingBox::shiftAll(const int &shift) {
    if(hasDurationRectangle()) mDurationRectangle->changeFramePosBy(shift);
    else anim_shiftAllKeys(shift);
}

QMatrix BoundingBox::getRelativeTransformAtRelFrameF(const qreal &relFrame) {
    if(isZero6Dec(relFrame - anim_getCurrentRelFrame()))
        return mTransformAnimator->BasicTransformAnimator::getRelativeTransform();
    return mTransformAnimator->getRelativeTransform(relFrame);
}

QMatrix BoundingBox::getTotalTransformAtRelFrameF(const qreal &relFrame) {
    if(isZero6Dec(relFrame - anim_getCurrentRelFrame()))
        return mTransformAnimator->BasicTransformAnimator::getTotalTransform();
    return mTransformAnimator->getTotalTransformAtRelFrameF(relFrame);
}

int BoundingBox::prp_getRelFrameShift() const {
    if(!mDurationRectangle) return 0;
    return mDurationRectangle->getFrameShift();
}

void BoundingBox::setDurationRectangle(
        const qsptr<DurationRectangle>& durationRect) {
    if(durationRect == mDurationRectangle) return;
    if(mDurationRectangle) {
        disconnect(mDurationRectangle.data(), nullptr, this, nullptr);
    }
    const auto oldDurRect = mDurationRectangle;
    mDurationRectangle = durationRect;
    updateAfterDurationRectangleShifted(0);
    if(!mDurationRectangle)
        return shiftAll(oldDurRect->getFrameShift());

    connect(mDurationRectangle.data(), &DurationRectangle::posChangedBy,
            this, &BoundingBox::updateAfterDurationRectangleShifted);
    connect(mDurationRectangle.data(), &DurationRectangle::rangeChanged,
            this, &BoundingBox::updateAfterDurationRectangleRangeChanged);

    connect(mDurationRectangle.data(), &DurationRectangle::minFrameChangedBy,
            this, &BoundingBox::updateAfterDurationMinFrameChangedBy);
    connect(mDurationRectangle.data(), &DurationRectangle::maxFrameChangedBy,
            this, &BoundingBox::updateAfterDurationMaxFrameChangedBy);
}

void BoundingBox::updateAfterDurationRectangleShifted(const int &dFrame) {
    prp_afterFrameShiftChanged();
    const auto newRange = getVisibleAbsFrameRange();
    const auto oldRange = newRange.shifted(-dFrame);
    Property::prp_afterChangedAbsRange(newRange + oldRange);
    const int absFrame = anim_getCurrentAbsFrame();
    anim_setAbsFrame(absFrame);
}

void BoundingBox::updateAfterDurationMinFrameChangedBy(const int &by) {
    const auto newRange = getVisibleAbsFrameRange();
    const int newMin = newRange.fMin;
    const int oldMin = newRange.fMin - by;

    const int min = qMin(newMin, oldMin);
    const int max = qMax(newMin, oldMin);
    Property::prp_afterChangedAbsRange({min, max});
}

void BoundingBox::updateAfterDurationMaxFrameChangedBy(const int &by) {
    const auto newRange = getVisibleAbsFrameRange();
    const int newMax = newRange.fMax;
    const int oldMax = newRange.fMax - by;

    const int min = qMin(newMax, oldMax);
    const int max = qMax(newMax, oldMax);
    Property::prp_afterChangedAbsRange({min, max});
}

void BoundingBox::updateAfterDurationRectangleRangeChanged() {}

DurationRectangleMovable *BoundingBox::anim_getTimelineMovable(
        const int &relX, const int &minViewedFrame,
        const qreal &pixelsPerFrame) {
    if(!mDurationRectangle) return nullptr;
    return mDurationRectangle->getMovableAt(relX, pixelsPerFrame,
                                            minViewedFrame);
}

void BoundingBox::drawTimelineControls(QPainter * const p,
                                       const qreal &pixelsPerFrame,
                                       const FrameRange &absFrameRange,
                                       const int &rowHeight) {
    if(mDurationRectangle) {
        p->save();
        p->translate(prp_getParentFrameShift()*pixelsPerFrame, 0);
        const int width = qCeil(absFrameRange.span()*pixelsPerFrame);
        const QRect drawRect(0, 0, width, rowHeight);
        const auto parentCanvas = getParentCanvas();
        const qreal fps = parentCanvas ? parentCanvas->getFps() : 1;
        mDurationRectangle->draw(p, drawRect, fps,
                                 pixelsPerFrame, absFrameRange);
        p->restore();
    }

    ComplexAnimator::drawTimelineControls(p, pixelsPerFrame,
                                          absFrameRange, rowHeight);
}

void BoundingBox::addPathEffect(const qsptr<PathEffect> &) {}

void BoundingBox::addFillPathEffect(const qsptr<PathEffect> &) {}

void BoundingBox::addOutlinePathEffect(const qsptr<PathEffect> &) {}

void BoundingBox::removePathEffect(const qsptr<PathEffect> &) {}

void BoundingBox::removeFillPathEffect(const qsptr<PathEffect> &) {}

void BoundingBox::removeOutlinePathEffect(const qsptr<PathEffect> &) {}

void BoundingBox::setName(const QString &name) {
    if(name == prp_mName) return;
    prp_mName = name;

    emit nameChanged(name);
}

const QString &BoundingBox::getName() const {
    return prp_mName;
}

bool BoundingBox::isVisibleAndInVisibleDurationRect() const {
    return isFrameInDurationRect(anim_getCurrentRelFrame()) && mVisible;
}

bool BoundingBox::isFrameInDurationRect(const int &relFrame) const {
    if(!mDurationRectangle) return true;
    return relFrame <= mDurationRectangle->getMaxFrameAsRelFrame() &&
           relFrame >= mDurationRectangle->getMinFrameAsRelFrame();
}

bool BoundingBox::isFrameFInDurationRect(const qreal &relFrame) const {
    if(!mDurationRectangle) return true;
    return qRound(relFrame) <= mDurationRectangle->getMaxFrameAsRelFrame() &&
           qRound(relFrame) >= mDurationRectangle->getMinFrameAsRelFrame();
}

bool BoundingBox::isVisibleAndInDurationRect(
        const int &relFrame) const {
    return isFrameInDurationRect(relFrame) && mVisible;
}

bool BoundingBox::isFrameFVisibleAndInDurationRect(
        const qreal &relFrame) const {
    return isFrameFInDurationRect(relFrame) && mVisible;
}

FrameRange BoundingBox::prp_getIdenticalRelRange(const int &relFrame) const {
    if(mVisible) {
        const auto cRange = ComplexAnimator::prp_getIdenticalRelRange(relFrame);
        if(mDurationRectangle) {
            const auto dRange = mDurationRectangle->getRelFrameRange();
            if(relFrame > dRange.fMax) {
                return {mDurationRectangle->getMaxFrameAsRelFrame() + 1,
                            FrameRange::EMAX};
            } else if(relFrame < dRange.fMin) {
                return {FrameRange::EMIN,
                        mDurationRectangle->getMinFrameAsRelFrame() - 1};
            } else return cRange*dRange;
        }
        return cRange;
    }
    return {FrameRange::EMIN, FrameRange::EMAX};
}


FrameRange BoundingBox::getFirstAndLastIdenticalForMotionBlur(
        const int &relFrame, const bool &takeAncestorsIntoAccount) {
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
    int parentRel = mParentGroup->prp_absFrameToRelFrame(
                prp_relFrameToAbsFrame(relFrame));
    auto parentRange = mParentGroup->BoundingBox::getFirstAndLastIdenticalForMotionBlur(parentRel);

    return range*parentRange;
}

void BoundingBox::cancelWaitingTasks() {
    for(const auto &task : mScheduledTasks) task->cancel();
    mScheduledTasks.clear();
}

void BoundingBox::scheduleWaitingTasks() {
    scheduleUpdate();
    for(const auto &task : mScheduledTasks)
        task->taskQued();
}

void BoundingBox::queScheduledTasks() {
    const auto taskScheduler = TaskScheduler::sGetInstance();
    for(const auto& task : mScheduledTasks)
        taskScheduler->queCPUTask(task);
    mScheduledTasks.clear();
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

BoundingBox *BoundingBox::sGetBoxByReadId(const int &readId) {
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
        const QRectF &absRect, QList<stdptr<MovablePoint>> &selection,
        const CanvasMode& mode) {
    for(const auto& desc : mCanvasProps) {
        const auto handler = desc->getPointsHandler();
        if(!handler) continue;
        handler->addInRectForSelection(absRect, selection, mode);
    }
}

void BoundingBox::selectAllCanvasPts(QList<stdptr<MovablePoint> > &selection,
                                     const CanvasMode& mode) {
    for(const auto& desc : mCanvasProps) {
        const auto handler = desc->getPointsHandler();
        if(!handler) continue;
        handler->addAllPointsToSelection(selection, mode);
    }
}

void BoundingBox::scheduleTask(const stdsptr<BoundingBoxRenderData>& task) {
    mScheduledTasks << task;
}

void BoundingBox::setVisibile(const bool &visible) {
    if(mVisible == visible) return;
    if(mSelected) removeFromSelection();
    mVisible = visible;

    prp_afterWholeInfluenceRangeChanged();

    SWT_scheduleWidgetsContentUpdateWithRule(SWT_BR_VISIBLE);
    SWT_scheduleWidgetsContentUpdateWithRule(SWT_BR_HIDDEN);
    for(const auto&  box : mLinkingBoxes) {
        if(box->isParentLinkBox())
            box->setVisibile(visible);
    }
}

void BoundingBox::switchVisible() {
    setVisibile(!mVisible);
}

bool BoundingBox::isParentLinkBox() {
    return mParentGroup->SWT_isLinkBox();
}

void BoundingBox::switchLocked() {
    setLocked(!mLocked);
}

void BoundingBox::hide() {
    setVisibile(false);
}

void BoundingBox::show() {
    setVisibile(true);
}

bool BoundingBox::isVisibleAndUnlocked() const {
    return isVisible() && !mLocked;
}

bool BoundingBox::isVisible() const {
    return mVisible;
}

bool BoundingBox::isLocked() const {
    return mLocked;
}

void BoundingBox::lock() {
    setLocked(true);
}

void BoundingBox::unlock() {
    setLocked(false);
}

void BoundingBox::setLocked(const bool &bt) {
    if(bt == mLocked) return;
    if(mSelected) getParentCanvas()->removeBoxFromSelection(this);
    mLocked = bt;
    SWT_scheduleWidgetsContentUpdateWithRule(SWT_BR_LOCKED);
    SWT_scheduleWidgetsContentUpdateWithRule(SWT_BR_UNLOCKED);
}

bool BoundingBox::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                      const bool &parentSatisfies,
                                      const bool &parentMainTarget) const {
    const SWT_BoxRule &rule = rules.fRule;
    bool satisfies = false;
    bool alwaysShowChildren = rules.fAlwaysShowChildren;
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

void BoundingBox::removeFromParent_k() {
    if(!mParentGroup) return;
    mParentGroup->removeContainedBox_k(ref<BoundingBox>());
}

void BoundingBox::removeFromSelection() {
    if(mSelected) {
        Canvas* const parentCanvas = getParentCanvas();
        parentCanvas->removeBoxFromSelection(this);
    }
}

QMimeData *BoundingBox::SWT_createMimeData() {
    return new BoundingBoxMimeData(this);
}

void BoundingBox::renderDataFinished(BoundingBoxRenderData *renderData) {
    auto currentRenderData = mDrawRenderContainer.getSrcRenderData();
    bool newerSate = true;
    bool closerFrame = true;
    if(currentRenderData) {
        newerSate = currentRenderData->fBoxStateId < renderData->fBoxStateId;
        const int finishedFrameDist =
                qAbs(anim_getCurrentRelFrame() - renderData->fRelFrame);
        const int oldFrameDist =
                qAbs(anim_getCurrentRelFrame() - currentRenderData->fRelFrame);
        closerFrame = finishedFrameDist < oldFrameDist;
    }
    if(newerSate || closerFrame) {
        mDrawRenderContainer.setSrcRenderData(renderData);
        const bool currentState = renderData->fBoxStateId == mStateId;
        const bool currentFrame = renderData->fRelFrame == anim_getCurrentRelFrame();
        const bool expired = !currentState || !currentFrame;
        mDrawRenderContainer.setExpired(expired);
        if(expired) updateDrawRenderContainerTransform();
    }
}

FrameRange BoundingBox::getVisibleAbsFrameRange() const {
    if(!mDurationRectangle) return {FrameRange::EMIN, FrameRange::EMAX};
    return mDurationRectangle->getAbsFrameRange();
}
