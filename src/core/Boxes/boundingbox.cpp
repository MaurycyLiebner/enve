// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "Boxes/boundingbox.h"
#include "Boxes/containerbox.h"
#include "canvas.h"
#include "swt_abstraction.h"
#include "Timeline/durationrectangle.h"
#include "pointhelpers.h"
#include "skia/skqtconversions.h"
#include "GUI/global.h"
#include "MovablePoints/movablepoint.h"
#include "Private/Tasks/taskscheduler.h"
#include "RasterEffects/rastereffectcollection.h"
#include "Animators/transformanimator.h"
#include "RasterEffects/rastereffect.h"
#include "RasterEffects/customrastereffectcreator.h"
#include "internallinkbox.h"
#include "Animators/qpointfanimator.h"
#include "MovablePoints/pathpointshandler.h"
#include "typemenu.h"
#include "patheffectsmenu.h"
#include "RasterEffects/rastereffectsinclude.h"
#include "RasterEffects/rastereffectmenucreator.h"
#include "matrixdecomposition.h"
#include "paintsettingsapplier.h"
#include "Animators/customproperties.h"
#include "GUI/propertynamedialog.h"
#include "BlendEffects/blendeffectcollection.h"

int BoundingBox::sNextDocumentId = 0;
QList<BoundingBox*> BoundingBox::sDocumentBoxes;
QList<BoundingBox*> BoundingBox::sReadBoxes;
int BoundingBox::sNextWriteId;
QList<const BoundingBox*> BoundingBox::sBoxesWithWriteIds;

BoundingBox::BoundingBox(const QString& name, const eBoxType type) :
    eBoxOrSound(name),
    mDocumentId(sNextDocumentId++), mType(type),
    mCustomProperties(enve::make_shared<CustomProperties>()),
    mBlendEffectCollection(enve::make_shared<BlendEffectCollection>()),
    mTransformAnimator(enve::make_shared<BoxTransformAnimator>()),
    mRasterEffectsAnimators(enve::make_shared<RasterEffectCollection>()) {
    sDocumentBoxes << this;

    ca_addChild(mCustomProperties);
    mCustomProperties->SWT_setVisible(false);

    ca_addChild(mBlendEffectCollection);

    ca_addChild(mTransformAnimator);
    const auto pivotAnim = mTransformAnimator->getPivotAnimator();
    connect(pivotAnim, &Property::prp_currentFrameChanged,
            this, &BoundingBox::requestGlobalPivotUpdateIfSelected);

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

void BoundingBox::writeBoundingBox(eWriteStream& dst) const {
    if(mWriteId < 0) assignWriteId();
    eBoxOrSound::prp_writeProperty(dst);
    dst << mWriteId;
    dst.write(&mBlendMode, sizeof(SkBlendMode));
}

void BoundingBox::readBoundingBox(eReadStream& src) {
    eBoxOrSound::prp_readProperty(src);
    if(src.evFileVersion() < 10) {
        QString name; src >> name;
        prp_setName(name);
    }
    src >> mReadId;
    src.read(&mBlendMode, sizeof(SkBlendMode));

    BoundingBox::sAddReadBox(this);
}

BoundingBox *BoundingBox::sGetBoxByDocumentId(const int documentId) {
    for(const auto& box : sDocumentBoxes) {
        if(box->getDocumentId() == documentId) return box;
    }
    return nullptr;
}

void BoundingBox::prp_afterChangedAbsRange(const FrameRange &range, const bool clip) {
    const auto croppedRange = clip ? prp_absInfluenceRange()*range : range;
    StaticComplexAnimator::prp_afterChangedAbsRange(croppedRange, clip);
    if(croppedRange.inRange(anim_getCurrentAbsFrame())) {
        planUpdate(UpdateReason::userChange);
    }
}

void BoundingBox::ca_childIsRecordingChanged() {
    ComplexAnimator::ca_childIsRecordingChanged();
    SWT_scheduleContentUpdate(SWT_BoxRule::animated);
    SWT_scheduleContentUpdate(SWT_BoxRule::notAnimated);
}

qsptr<BoundingBox> BoundingBox::createLink() {
    auto linkBox = enve::make_shared<InternalLinkBox>(this);
    copyTransformationTo(linkBox.get());
    return std::move(linkBox);
}

void BoundingBox::clearRasterEffects() {
    mRasterEffectsAnimators->clear();
}

QPointF BoundingBox::getRelCenterPosition() {
    return mRelRect.center();
}

void BoundingBox::centerPivotPosition() {
    const auto center = getRelCenterPosition();
    mTransformAnimator->setPivotFixedTransform(center);
    requestGlobalPivotUpdateIfSelected();
}

void BoundingBox::centerPivotPositionAction() {
    const auto pos = mTransformAnimator->getPosAnimator();
    const auto pivot = mTransformAnimator->getPivotAnimator();
    pos->prp_startTransform();
    pivot->prp_startTransform();
    centerPivotPosition();
    pos->prp_finishTransform();
    pivot->prp_finishTransform();
}

void BoundingBox::planCenterPivotPosition() {
    mCenterPivotPlanned = true;
}

void BoundingBox::blendSetup(ChildRenderData &data,
                             const int index, const qreal relFrame,
                             QList<ChildRenderData> &delayed) const {
    mBlendEffectCollection->blendSetup(data, index, relFrame, delayed);
}

void BoundingBox::updateIfUsesProgram(
        const ShaderEffectProgram * const program) const {
    mRasterEffectsAnimators->updateIfUsesProgram(program);
}

void BoundingBox::copyTransformationTo(BoundingBox * const targetBox) const {
    sWriteReadMember(this, targetBox, &BoundingBox::mTransformAnimator);
}

void BoundingBox::copyRasterEffectsTo(BoundingBox * const targetBox) const {
    sWriteReadMember(this, targetBox, &BoundingBox::mRasterEffectsAnimators);
}

void BoundingBox::copyBoundingBoxDataTo(BoundingBox * const targetBox) const {
    copyTransformationTo(targetBox);
    copyRasterEffectsTo(targetBox);
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
                mRasterEffectsAnimators->ca_hasChildren() || enable);
}

bool BoundingBox::getRasterEffectsEnabled() const {
    return mRasterEffectsAnimators->SWT_isEnabled();
}

void BoundingBox::updateAllBoxes(const UpdateReason reason) {
    planUpdate(reason);
}

void BoundingBox::drawAllCanvasControls(SkCanvas * const canvas,
                                        const CanvasMode mode,
                                        const float invScale,
                                        const bool ctrlPressed) {
    for(const auto& prop : mCanvasProps)
        prop->prp_drawCanvasControls(canvas, mode, invScale, ctrlPressed);
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

#include "efiltersettings.h"

void BoundingBox::drawPixmapSk(SkCanvas * const canvas,
                               const SkFilterQuality filter) {
    const qreal opacity = mTransformAnimator->getOpacity();
    if(isZero4Dec(opacity) || !mVisibleInScene) return;
    SkPaint paint;
    const int intAlpha = qRound(opacity*2.55);
    paint.setAlpha(static_cast<U8CPU>(intAlpha));
    paint.setBlendMode(mBlendMode);
    paint.setFilterQuality(filter);
    mDrawRenderContainer.drawSk(canvas, paint);
}

void BoundingBox::drawPixmapSk(SkCanvas * const canvas,
                               const SkFilterQuality filter, int& drawId,
                               QList<BlendEffect::Delayed> &delayed) {
    mBlendEffectCollection->drawBlendSetup(this, canvas, filter, drawId, delayed);
}

void BoundingBox::setBlendModeSk(const SkBlendMode blendMode) {
    if(mBlendMode == blendMode) return;
    {
        prp_pushUndoRedoName("Set Blend Mode");
        UndoRedo ur;
        const auto oldValue = mBlendMode;
        const auto newValue = blendMode;
        ur.fUndo = [this, oldValue]() {
            setBlendModeSk(oldValue);
        };
        ur.fRedo = [this, newValue]() {
            setBlendModeSk(newValue);
        };
        prp_addUndoRedo(ur);
    }
    mBlendMode = blendMode;
    prp_afterWholeInfluenceRangeChanged();
    emit blendModeChanged(blendMode);
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
        planUpdate(UpdateReason::frameChange);
    }
}

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
    planUpdate(reason);
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
        centerPivotPosition();
    }
}

void BoundingBox::prp_updateCanvasProps() {
    mCanvasProps.clear();
    ca_execOnDescendants([this](Property * prop) {
        if(prop->prp_drawsOnCanvas()) mCanvasProps.append(prop);
    });
    if(prp_drawsOnCanvas()) mCanvasProps.append(this);
    const auto parentScene = getParentScene();
    if(parentScene) parentScene->requestUpdate();
}

void BoundingBox::updateCurrentPreviewDataFromRenderData(
        BoxRenderData* renderData) {
    setRelBoundingRect(renderData->fRelBoundingRect);
}

void BoundingBox::planUpdate(const UpdateReason reason) {
    if(mUpdatePlanned && mPlannedReason == UpdateReason::userChange) return;
    if(!isVisibleAndInVisibleDurationRect()) return;
    if(mParentGroup) mParentGroup->planUpdate(reason);
    else if(!SWT_isCanvas()) return;
    if(reason == UpdateReason::userChange) {
        mStateId++;
        mRenderDataHandler.clear();
    }

    mDrawRenderContainer.setExpired(true);
    if(mUpdatePlanned) {
        mPlannedReason = qMax(reason, mPlannedReason);
    } else {
        mUpdatePlanned = true;
        mPlannedReason = reason;
    }
}

stdsptr<BoxRenderData> BoundingBox::queRender(const qreal relFrame) {
    const auto currentRenderData = updateCurrentRenderData(relFrame);
    if(!currentRenderData) return nullptr;
    setupRenderData(relFrame, currentRenderData, getParentScene());
    const auto currentSPtr = enve::shared(currentRenderData);
    currentSPtr->queTask();
    return currentSPtr;
}

void BoundingBox::queTasks() {
    if(!mUpdatePlanned) return;
    mUpdatePlanned = false;
    if(!shouldScheduleUpdate()) return;
    const int relFrame = anim_getCurrentRelFrame();
    if(hasCurrentRenderData(relFrame)) return;
    queRender(relFrame);
}

BoxRenderData *BoundingBox::updateCurrentRenderData(const qreal relFrame) {
    const auto renderData = createRenderData();
    if(!renderData) return nullptr;
    renderData->fRelFrame = relFrame;
    mRenderDataHandler.addItemAtRelFrame(renderData);
    return renderData.get();
}

bool BoundingBox::hasCurrentRenderData(const qreal relFrame) const {
    const auto currentRenderData = mRenderDataHandler.getItemAtRelFrame(relFrame);
    if(currentRenderData) return true;
    if(mDrawRenderContainer.isExpired()) return false;
    const auto drawData = mDrawRenderContainer.getSrcRenderData();
    if(!drawData) return false;
    return !diffsIncludingInherited(drawData->fRelFrame, relFrame);
}

stdsptr<BoxRenderData> BoundingBox::getCurrentRenderData(const qreal relFrame) const {
    const auto currentRenderData =
            mRenderDataHandler.getItemAtRelFrame(relFrame);
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

void BoundingBox::setStrokeCapStyle(const SkPaint::Cap capStyle) {
    const auto strokeSettings = getStrokeSettings();
    if(!strokeSettings) return;
    strokeSettings->setCapStyle(capStyle);
    prp_afterWholeInfluenceRangeChanged();
    planUpdate(UpdateReason::userChange);
}

void BoundingBox::setStrokeJoinStyle(const SkPaint::Join joinStyle) {
    const auto strokeSettings = getStrokeSettings();
    if(!strokeSettings) return;
    strokeSettings->setJoinStyle(joinStyle);
    prp_afterWholeInfluenceRangeChanged();
    planUpdate(UpdateReason::userChange);
}

void BoundingBox::setOutlineCompositionMode(
        const QPainter::CompositionMode compositionMode) {
    const auto strokeSettings = getStrokeSettings();
    if(!strokeSettings) return;
    strokeSettings->setOutlineCompositionMode(compositionMode);
    prp_afterWholeInfluenceRangeChanged();
    planUpdate(UpdateReason::userChange);
}

void BoundingBox::setStrokeBrush(SimpleBrushWrapper * const brush) {
    const auto strokeSettings = getStrokeSettings();
    if(!strokeSettings) return;
    strokeSettings->setStrokeBrush(brush);
    requestGlobalFillStrokeUpdateIfSelected();
}

void BoundingBox::applyStrokeBrushWidthAction(const SegAction &action) {
    const auto strokeSettings = getStrokeSettings();
    if(!strokeSettings) return;
    strokeSettings->applyStrokeBrushWidthAction(action);
}

void BoundingBox::applyStrokeBrushPressureAction(const SegAction &action) {
    const auto strokeSettings = getStrokeSettings();
    if(!strokeSettings) return;
    strokeSettings->applyStrokeBrushPressureAction(action);
}

void BoundingBox::applyStrokeBrushSpacingAction(const SegAction &action) {
    const auto strokeSettings = getStrokeSettings();
    if(!strokeSettings) return;
    strokeSettings->applyStrokeBrushSpacingAction(action);
}

void BoundingBox::applyStrokeBrushTimeAction(const SegAction &action) {
    const auto strokeSettings = getStrokeSettings();
    if(!strokeSettings) return;
    strokeSettings->applyStrokeBrushTimeAction(action);
}

QPointF BoundingBox::mapAbsPosToRel(const QPointF &absPos) {
    return mTransformAnimator->mapAbsPosToRel(absPos);
}

void BoundingBox::strokeWidthAction(const QrealAction& action) {
    const auto strokeSettings = getStrokeSettings();
    if(!strokeSettings) return;
    strokeSettings->strokeWidthAction(action);
}

void BoundingBox::startSelectedStrokeColorTransform() {
    const auto strokeSettings = getStrokeSettings();
    if(!strokeSettings) return;
    strokeSettings->getColorAnimator()->prp_startTransform();
}

void BoundingBox::startSelectedFillColorTransform() {
    const auto fillSettings = getStrokeSettings();
    if(!fillSettings) return;
    fillSettings->getColorAnimator()->prp_startTransform();
}

void BoundingBox::applyPaintSetting(const PaintSettingsApplier &setting)
{ setting.apply(this); }

void BoundingBox::drawBoundingRect(SkCanvas * const canvas,
                                   const float invScale) {
    SkiaHelpers::drawOutlineOverlay(canvas, mSkRelBoundingRectPath,
                                    invScale, toSkMatrix(getTotalTransform()),
                                    true, MIN_WIDGET_DIM*0.25f);
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

void BoundingBox::setupCanvasMenu(PropertyMenu * const menu) {
    if(menu->hasActionsForType<BoundingBox>()) return;
    menu->addedActionsForType<BoundingBox>();
    const auto pScene = getParentScene();
    Q_ASSERT(pScene);

    menu->addSection("Box");

    menu->addPlainAction("Create Link", [pScene]() {
        pScene->createLinkBoxForSelected();
    });
    menu->addPlainAction("Center Pivot", [pScene]() {
        pScene->centerPivotForSelected();
    });

    menu->addSeparator();

    menu->addPlainAction("Copy", [pScene]() {
        pScene->copyAction();
    })->setShortcut(Qt::CTRL + Qt::Key_C);

    menu->addPlainAction("Cut", [pScene]() {
        pScene->cutAction();
    })->setShortcut(Qt::CTRL + Qt::Key_X);

    menu->addPlainAction("Duplicate", [pScene]() {
        pScene->duplicateAction();
    })->setShortcut(Qt::CTRL + Qt::Key_D);

    menu->addPlainAction("Delete", [pScene]() {
        pScene->removeSelectedBoxesAndClearList();
    })->setShortcut(Qt::Key_Delete);

    menu->addSeparator();

    menu->addPlainAction("Group", [pScene]() {
        pScene->groupSelectedBoxes();
    })->setShortcut(Qt::CTRL + Qt::Key_G);

    menu->addPlainAction("Ungroup", [pScene]() {
        pScene->ungroupSelectedBoxes();
    })->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_G);



    menu->addSeparator();

    const auto rasterEffectsMenu = menu->addMenu("Raster Effects");
    RasterEffectMenuCreator::addEffects(
                rasterEffectsMenu, &BoundingBox::addRasterEffect);
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
                                  BoxRenderData * const data,
                                  Canvas* const scene) {
    setupWithoutRasterEffects(relFrame, data, scene);
    setupRasterEffects(relFrame, data, scene);
}

void BoundingBox::setupWithoutRasterEffects(const qreal relFrame,
                                            BoxRenderData * const data,
                                            Canvas* const scene) {
    Q_ASSERT(scene);
    if(!scene) return;

    data->fBoxStateId = mStateId;
    data->fRelFrame = relFrame;
    data->fRelTransform = getRelativeTransformAtFrame(relFrame);
    data->fInheritedTransform = getInheritedTransformAtFrame(relFrame);
    data->fTotalTransform = getTotalTransformAtFrame(relFrame);
    data->fResolution = scene->getResolutionFraction();
    data->fResolutionScale.reset();
    data->fResolutionScale.scale(data->fResolution, data->fResolution);
    data->fOpacity = mTransformAnimator->getOpacity(relFrame);
    data->fBaseMargin = QMargins() + 2;
    data->fBlendMode = getBlendMode();

    {
        QRectF maxBoundsF;
        if(mParentGroup) maxBoundsF = QRectF(mParentGroup->currentGlobalBounds());
        else maxBoundsF = QRectF(scene->getCurrentBounds());
        const QRectF scaledMaxBoundsF = data->fResolutionScale.mapRect(maxBoundsF);
        data->fMaxBoundsRect = scaledMaxBoundsF.toAlignedRect();
    }
}

void BoundingBox::setupRasterEffects(const qreal relFrame,
                                     BoxRenderData * const data,
                                     Canvas* const scene) {
    Q_ASSERT(scene);
    if(!scene) return;
    const bool effectsVisible = scene->getRasterEffectsVisible();
    if(data->fOpacity > 0.001 && effectsVisible) {
        mRasterEffectsAnimators->addEffects(relFrame, data);
    }
}

void BoundingBox::addLinkingBox(BoundingBox *box) {
    mLinkingBoxes << box;
}

void BoundingBox::removeLinkingBox(BoundingBox *box) {
    mLinkingBoxes.removeOne(box);
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

void BoundingBox::applyParentTransform() {
    if(!mParentTransform) return;
    const auto parentTransform = mParentTransform->getRelativeTransform();
    const auto thisTransform = mTransformAnimator->getRelativeTransform();
    const auto newTransform = thisTransform*parentTransform;
    const auto pivot = mTransformAnimator->getPivot();
    const auto dec = MatrixDecomposition::decomposePivoted(newTransform, pivot);

    mTransformAnimator->startTransformSkipOpacity();
    mTransformAnimator->setValues(dec);
    mTransformAnimator->prp_finishTransform();
}

bool BoundingBox::isTransformationStatic() const {
    return !mTransformAnimator->anim_isRecording();
}

BasicTransformAnimator *BoundingBox::getTransformAnimator() const {
    return getBoxTransformAnimator();
}

BoxTransformAnimator *BoundingBox::getBoxTransformAnimator() const {
    return mTransformAnimator.get();
}

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

QMatrix BoundingBox::getInheritedTransformAtFrame(const qreal relFrame) {
    if(isZero6Dec(relFrame - anim_getCurrentRelFrame()))
        return mTransformAnimator->getInheritedTransform();
    return mTransformAnimator->getInheritedTransformAtFrame(relFrame);
}

QMatrix BoundingBox::getTotalTransformAtFrame(const qreal relFrame) {
    if(isZero6Dec(relFrame - anim_getCurrentRelFrame()))
        return mTransformAnimator->getTotalTransform();
    return mTransformAnimator->getTotalTransformAtFrame(relFrame);
}

#include <QInputDialog>
void BoundingBox::prp_setupTreeViewMenu(PropertyMenu * const menu) {
    if(menu->hasActionsForType<BoundingBox>()) return;
    menu->addedActionsForType<BoundingBox>();
    const auto parentWidget = menu->getParentWidget();
    menu->addPlainAction("Rename", [this, parentWidget]() {
        PropertyNameDialog::sRenameBox(this, parentWidget);
    });
    menu->addSeparator();
    {
        const PropertyMenu::CheckSelectedOp<BoundingBox> visRangeOp =
        [](BoundingBox* const box, const bool checked) {
            box->mCustomProperties->SWT_setVisible(checked);
        };
        menu->addCheckableAction("Custom Properties",
                                 mCustomProperties->SWT_isVisible(),
                                 visRangeOp);
    }
    menu->addSeparator();
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
    menu->addSeparator();
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
            if(relFrame > mDurationRectangle->getMaxRelFrame()) {
                return mDurationRectangle->getAbsFrameRangeToTheRight();
            } else if(relFrame < mDurationRectangle->getMinRelFrame()) {
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

void BoundingBox::writeIdentifier(eWriteStream &dst) const {
    dst.write(&mType, sizeof(eBoxType));
}

int BoundingBox::getReadId() const {
    return mReadId;
}

int BoundingBox::getWriteId() const {
    return mWriteId;
}

int BoundingBox::assignWriteId() const {
    mWriteId = sNextWriteId++;
    sBoxesWithWriteIds << this;
    return mWriteId;
}

void BoundingBox::clearReadId() const {
    mReadId = -1;
}

void BoundingBox::clearWriteId() const {
    mWriteId = -1;
}

BoundingBox *BoundingBox::sGetBoxByReadId(const int readId) {
    for(const auto& box : sReadBoxes) {
        if(box->getReadId() == readId) return box;
    }
    return nullptr;
}

void BoundingBox::sAddReadBox(BoundingBox * const box) {
    sReadBoxes << box;
}

void BoundingBox::sClearWriteBoxes() {
    for(const auto& box : sBoxesWithWriteIds) {
        box->clearWriteId();
    }
    sNextWriteId = 0;
    sBoxesWithWriteIds.clear();
}

#include "simpletask.h"
void BoundingBox::sClearReadBoxes() {
    SimpleTask::sSchedule([]() {
        for(const auto& box : sReadBoxes) {
            box->clearReadId();
        }
        sReadBoxes.clear();
    });
}

void BoundingBox::sForEveryReadBox(const std::function<void(BoundingBox*)> &func) {
    for(const auto& box : sReadBoxes) func(box);
}

void BoundingBox::selectAndAddContainedPointsToList(
        const QRectF &absRect,
        const MovablePoint::PtOp &adder,
        const CanvasMode mode) {
    for(const auto& desc : mCanvasProps) {
        const auto handler = desc->getPointsHandler();
        if(!handler) continue;
        handler->addInRectForSelection(absRect, adder, mode);
    }
}

void BoundingBox::selectAllCanvasPts(const MovablePoint::PtOp &adder,
                                     const CanvasMode mode) {
    for(const auto& desc : mCanvasProps) {
        const auto handler = desc->getPointsHandler();
        if(!handler) continue;
        handler->addAllPointsToSelection(adder, mode);
    }
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
    if(rules.fType == SWT_Type::sound) return false;
    if(alwaysShowChildren) {
        if(rule == SWT_BoxRule::all) {
            satisfies = parentSatisfies;
        } else if(rule == SWT_BoxRule::selected) {
            satisfies = isSelected() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_BoxRule::animated) {
            satisfies = isAnimated() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_BoxRule::notAnimated) {
            satisfies = !isAnimated() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_BoxRule::visible) {
            satisfies = isVisible() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_BoxRule::hidden) {
            satisfies = !isVisible() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_BoxRule::locked) {
            satisfies = isLocked() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_BoxRule::unlocked) {
            satisfies = !isLocked() ||
                    (parentSatisfies && !parentMainTarget);
        }
    } else {
        if(rule == SWT_BoxRule::all) {
            satisfies = parentSatisfies;
        } else if(rule == SWT_BoxRule::selected) {
            satisfies = isSelected();
        } else if(rule == SWT_BoxRule::animated) {
            satisfies = isAnimated();
        } else if(rule == SWT_BoxRule::notAnimated) {
            satisfies = !isAnimated();
        } else if(rule == SWT_BoxRule::visible) {
            satisfies = isVisible() && parentSatisfies;
        } else if(rule == SWT_BoxRule::hidden) {
            satisfies = !isVisible() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_BoxRule::locked) {
            satisfies = isLocked() ||
                    (parentSatisfies && !parentMainTarget);
        } else if(rule == SWT_BoxRule::unlocked) {
            satisfies = !isLocked() && parentSatisfies;
        }
    }
    if(satisfies) {
        if(!rules.fSearchString.isEmpty()) {
            satisfies = prp_getName().contains(rules.fSearchString);
        }
    }
    return satisfies;
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
    const bool currentState = renderData->fBoxStateId == mStateId;
    const qreal relFrame = renderData->fRelFrame;
    if(currentState) mRenderDataHandler.removeItemAtRelFrame(relFrame);
    auto currentRenderData = mDrawRenderContainer.getSrcRenderData();
    bool newerSate = true;
    bool closerFrame = true;
    if(currentRenderData) {
        newerSate = currentRenderData->fBoxStateId < renderData->fBoxStateId;
        const qreal finishedFrameDist =
                qAbs(anim_getCurrentRelFrame() - relFrame);
        const qreal oldFrameDist =
                qAbs(anim_getCurrentRelFrame() - currentRenderData->fRelFrame);
        closerFrame = finishedFrameDist < oldFrameDist;
    }
    if(newerSate || closerFrame) {
        mDrawRenderContainer.setSrcRenderData(renderData);
        const bool currentFrame = isZero4Dec(relFrame - anim_getCurrentRelFrame());
        const bool expired = !currentState || !currentFrame;
        mDrawRenderContainer.setExpired(expired);
        if(expired) updateDrawRenderContainerTransform();
    }
}
