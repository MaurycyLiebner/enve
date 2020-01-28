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

#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "Animators/eboxorsound.h"
#include "boxrendercontainer.h"
#include "skia/skiaincludes.h"
#include "renderdatahandler.h"
#include "smartPointers/ememory.h"
#include "colorhelpers.h"
#include "MovablePoints/segment.h"
#include "Animators/qcubicsegment1danimator.h"
class Canvas;

class QrealAction;
class MovablePoint;

class PathEffect;
class TimelineMovable;
class FillSettingsAnimator;
class OutlineSettingsAnimator;
class PaintSettingsApplier;
class RasterEffectCollection;
class ShaderEffectProgram;
class BoxTransformAnimator;
class BasicTransformAnimator;

class ContainerBox;
class SmartVectorPath;
class DurationRectangle;
struct ContainerBoxRenderData;
class ShaderEffect;
class RasterEffect;
enum class CanvasMode : short;

class SimpleBrushWrapper;

enum class eBoxType {
    vectorPath,
    circle,
    image,
    rectangle,
    text,
    layer,
    canvas,
    internalLink,
    internalLinkGroup,
    internalLinkCanvas,
    externalLink,
    video,
    imageSequence,
    paint,
    group,
    custom,
    sculptPath
};

class BoundingBox;
template<typename T> class TypeMenu;
typedef TypeMenu<Property> PropertyMenu;

class BoundingBox : public eBoxOrSound {
    Q_OBJECT
    e_OBJECT
    typedef qCubicSegment1DAnimator::Action SegAction;
protected:
    BoundingBox(const eBoxType type);
public:
    ~BoundingBox();

    virtual stdsptr<BoxRenderData> createRenderData() = 0;

    static BoundingBox *sGetBoxByDocumentId(const int documentId);

    static void sAddReadBox(BoundingBox * const box);
    static BoundingBox *sGetBoxByReadId(const int readId);
    static void sClearReadBoxes();
    static void sForEveryReadBox(const std::function<void(BoundingBox*)>& func);

    static void sClearWriteBoxes();

    template <typename B, typename T>
    static void sWriteReadMember(const B* const from, B* const to, const T member);
private:
    static int sNextDocumentId;
    static QList<BoundingBox*> sDocumentBoxes;

    static QList<BoundingBox*> sReadBoxes;

    static int sNextWriteId;
    static QList<BoundingBox*> sBoxesWithWriteIds;
protected:
    virtual void getMotionBlurProperties(QList<Property*> &list) const;
public:
    virtual qsptr<BoundingBox> createLink();

    virtual SmartVectorPath *objectToVectorPathBox();
    virtual SmartVectorPath *strokeToVectorPathBox();

    virtual void centerPivotPosition();
    virtual QPointF getRelCenterPosition();

    virtual void selectAndAddContainedPointsToList(
            const QRectF &absRect,
            const MovablePoint::PtOp& adder,
            const CanvasMode mode);

    virtual bool relPointInsidePath(const QPointF &relPos) const;

    virtual void setFont(const QFont &font)
    { Q_UNUSED(font) }
    virtual void setSelectedFontSize(const qreal fontSize)
    { Q_UNUSED(fontSize) }


    virtual void setTextVAlignment(const Qt::Alignment alignment)
    { Q_UNUSED(alignment) }
    virtual void setTextHAlignment(const Qt::Alignment alignment)
    { Q_UNUSED(alignment) }

    virtual void setSelectedFontFamilyAndStyle(const QString &family,
                                               const QString &style) {
        Q_UNUSED(family)
        Q_UNUSED(style)
    }

    virtual void drawPixmapSk(SkCanvas * const canvas,
                              const SkFilterQuality filter);
    virtual void drawHoveredSk(SkCanvas *canvas,
                               const float invScale);

    virtual const SkPath &getRelBoundingRectPath();

    virtual BoundingBox *getBoxAtFromAllDescendents(const QPointF &absPos);

    virtual FillSettingsAnimator *getFillSettings() const;
    virtual OutlineSettingsAnimator *getStrokeSettings() const;

    virtual void applyStrokeBrushWidthAction(const SegAction& action)
    { Q_UNUSED(action) }
    virtual void applyStrokeBrushPressureAction(const SegAction& action)
    { Q_UNUSED(action) }
    virtual void applyStrokeBrushSpacingAction(const SegAction& action)
    { Q_UNUSED(action) }
    virtual void applyStrokeBrushTimeAction(const SegAction& action)
    { Q_UNUSED(action) }

    virtual void setStrokeCapStyle(const SkPaint::Cap capStyle)
    { Q_UNUSED(capStyle) }
    virtual void setStrokeJoinStyle(const SkPaint::Join joinStyle)
    { Q_UNUSED(joinStyle) }
    virtual void setStrokeBrush(SimpleBrushWrapper * const brush)
    { Q_UNUSED(brush) }
    virtual void strokeWidthAction(const QrealAction& action)
    { Q_UNUSED(action) }

    virtual void startSelectedStrokeColorTransform() {}
    virtual void startSelectedFillColorTransform() {}

    virtual void updateAllBoxes(const UpdateReason reason);

    virtual QMatrix getRelativeTransformAtCurrentFrame();
    virtual QMatrix getRelativeTransformAtFrame(const qreal relFrame);
    virtual QMatrix getInheritedTransformAtFrame(const qreal relFrame);
    virtual QMatrix getTotalTransformAtFrame(const qreal relFrame);
    virtual QPointF mapAbsPosToRel(const QPointF &absPos);

    virtual void applyPaintSetting(const PaintSettingsApplier &setting)
    { Q_UNUSED(setting) }
    virtual void addPathEffect(const qsptr<PathEffect>& effect)
    { Q_UNUSED(effect) }
    virtual void addFillPathEffect(const qsptr<PathEffect>& effect)
    { Q_UNUSED(effect) }
    virtual void addOutlineBasePathEffect(const qsptr<PathEffect>& effect)
    { Q_UNUSED(effect) }
    virtual void addOutlinePathEffect(const qsptr<PathEffect>& effect)
    { Q_UNUSED(effect) }

    virtual void setupCanvasMenu(PropertyMenu * const menu);

    virtual void setupRenderData(const qreal relFrame,
                                 BoxRenderData * const data,
                                 Canvas * const scene);
    virtual void renderDataFinished(BoxRenderData *renderData);
    virtual void updateCurrentPreviewDataFromRenderData(
            BoxRenderData* renderData);

    virtual FrameRange getFirstAndLastIdenticalForMotionBlur(
            const int relFrame, const bool takeAncestorsIntoAccount = true);

    virtual HardwareSupport hardwareSupport() const {
        return HardwareSupport::cpuPreffered;
    }

    virtual bool shouldScheduleUpdate() { return true; }
    virtual void queTasks();

    virtual void writeIdentifier(eWriteStream& dst) const;

    virtual void writeBoundingBox(eWriteStream& dst);
    virtual void readBoundingBox(eReadStream& src);

    virtual SkBlendMode getBlendMode()
    { return mBlendMode; }

    virtual void updateIfUsesProgram(const ShaderEffectProgram * const program) const;

    bool SWT_isBoundingBox() const final
    { return true; }
    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool parentSatisfies,
                             const bool parentMainTarget) const;
    bool SWT_visibleOnlyIfParentDescendant() const
    { return false; }

    bool SWT_dropSupport(const QMimeData* const data);
    bool SWT_drop(const QMimeData* const data);
protected:
    void prp_updateCanvasProps();
public:
    void prp_setupTreeViewMenu(PropertyMenu * const menu);

    void prp_afterChangedAbsRange(const FrameRange &range,
                                  const bool clip = true);

    void anim_setAbsFrame(const int frame);

    void ca_childIsRecordingChanged();

    BasicTransformAnimator *getTransformAnimator() const;

    stdsptr<BoxRenderData> queRender(const qreal relFrame);

    void setupWithoutRasterEffects(const qreal relFrame,
                                   BoxRenderData * const data,
                                   Canvas* const scene);
    void setupRasterEffects(const qreal relFrame,
                            BoxRenderData * const data,
                            Canvas* const scene);

    void drawAllCanvasControls(SkCanvas * const canvas,
                               const CanvasMode mode,
                               const float invScale,
                               const bool ctrlPressed);

    void moveByRel(const QPointF &trans);
    void moveByAbs(const QPointF &trans);
    void rotateBy(const qreal rot);
    void scale(const qreal scaleBy);
    void scale(const qreal scaleXBy, const qreal scaleYBy);
    void saveTransformPivotAbsPos(const QPointF &absPivot);

    void startPosTransform();
    void startRotTransform();
    void startScaleTransform();

    void startTransform();
    void finishTransform();
    void cancelTransform();

    QMatrix getTotalTransform() const;

    MovablePoint *getPointAtAbsPos(const QPointF &absPos,
                                   const CanvasMode mode,
                                   const qreal invScale) const;
    NormalSegment getNormalSegment(const QPointF &absPos,
                                   const qreal invScale) const;
    void drawBoundingRect(SkCanvas * const canvas,
                          const float invScale);

    void selectAllCanvasPts(const MovablePoint::PtOp &adder,
                            const CanvasMode mode);

    int getDocumentId() const { return mDocumentId; }

    int assignWriteId();
    void clearWriteId();
    int getWriteId() const;

    int getReadId() const;
    void clearReadId();

    void clearParent();
    void setParentTransform(BasicTransformAnimator *parent);
    bool isParentLinkBox();

    bool isContainedIn(const QRectF &absRect) const;

    QPointF getPivotAbsPos();
    QPointF getAbsolutePos() const;
    bool absPointInsidePath(const QPointF &absPos);

    void setPivotAbsPos(const QPointF &absPos);
    void setPivotRelPos(const QPointF &relPos);

    bool isAnimated() const;

    void rotateRelativeToSavedPivot(const qreal rot);
    void scaleRelativeToSavedPivot(const qreal scaleBy);
    void setAbsolutePos(const QPointF &pos);
    void setRelativePos(const QPointF &relPos);

    void scaleRelativeToSavedPivot(const qreal scaleXBy,
                                   const qreal scaleYBy);
    void startPivotTransform();
    void finishPivotTransform();
    void resetScale();
    void resetTranslation();
    void resetRotation();

    void applyParentTransform();
    bool isTransformationStatic() const;
    BoxTransformAnimator *getBoxTransformAnimator() const;
    QRectF getRelBoundingRect() const;
    void drawHoveredPathSk(SkCanvas *canvas, const SkPath &path,
                           const float invScale);

    void setRasterEffectsEnabled(const bool enable);
    bool getRasterEffectsEnabled() const;

    void clearRasterEffects();

    void addRasterEffect(const qsptr<RasterEffect> &rasterEffect);
    void removeRasterEffect(const qsptr<RasterEffect> &effect);

    void setBlendModeSk(const SkBlendMode blendMode);

    QPointF mapRelPosToAbs(const QPointF &relPos) const;

    void copyTransformationTo(BoundingBox * const targetBox) const;
    void copyRasterEffectsTo(BoundingBox * const targetBox) const;
    void copyBoundingBoxDataTo(BoundingBox * const targetBox) const;

//    int prp_getParentFrameShift() const;

    bool diffsIncludingInherited(const int relFrame1, const int relFrame2) const;
    bool diffsIncludingInherited(const qreal relFrame1, const qreal relFrame2) const;

    bool hasCurrentRenderData(const qreal relFrame) const;
    stdsptr<BoxRenderData> getCurrentRenderData(const qreal relFrame) const;
    BoxRenderData *updateCurrentRenderData(const qreal relFrame);

    void updateDrawRenderContainerTransform();

    void addLinkingBox(BoundingBox *box);
    void removeLinkingBox(BoundingBox *box);

    void incReasonsNotToApplyUglyTransform();
    void decReasonsNotToApplyUglyTransform();

    eBoxType getBoxType() const;

    void requestGlobalPivotUpdateIfSelected();
    void requestGlobalFillStrokeUpdateIfSelected();

    void planUpdate(const UpdateReason reason);

    void planCenterPivotPosition();

    bool visibleForScene() const
    { return mVisibleInScene; }
    void setVisibleForScene(const bool visible)
    { mVisibleInScene = visible; }
private:
    void cancelWaitingTasks();
    void afterTotalTransformChanged(const UpdateReason reason);
signals:
    void globalPivotInfluenced();
    void fillStrokeSettingsChanged();
    void blendModeChanged(SkBlendMode);
protected:
    void setRelBoundingRect(const QRectF& relRect);

    uint mStateId = 0;

    int mNReasonsNotToApplyUglyTransform = 0;
protected:
    bool getUpdatePlanned() const
    { return mUpdatePlanned; }

    SkBlendMode getBlendMode() const
    { return mBlendMode; }

    const QList<BoundingBox*>& getLinkingBoxes() const
    { return mLinkingBoxes; }

    const int mDocumentId;

    eBoxType mType;

    QRectF mRelRect;
    SkRect mRelRectSk;
    SkPath mSkRelBoundingRectPath;

    RenderDataHandler mRenderDataHandler;

    const qsptr<BoxTransformAnimator> mTransformAnimator;
    const qsptr<RasterEffectCollection> mRasterEffectsAnimators;
private:
    SkBlendMode mBlendMode = SkBlendMode::kSrcOver;

    int mReadId = -1;
    int mWriteId = -1;

    bool mVisibleInScene = true;
    bool mCenterPivotPlanned = false;
    bool mUpdatePlanned = false;
    UpdateReason mPlannedReason;

    QPointF mSavedTransformPivot;

    BasicTransformAnimator* mParentTransform = nullptr;

    QList<Property*> mCanvasProps;
    QList<BoundingBox*> mLinkingBoxes;

    RenderContainer mDrawRenderContainer;
};

template <typename B, typename T>
void BoundingBox::sWriteReadMember(const B * const from, B* const to, const T member) {
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    eWriteStream writeStream(&buffer);
    (from->*member)->prp_writeProperty(writeStream);
    writeStream.writeFutureTable();
    buffer.seek(0);
    eReadStream readStream(&buffer);
    buffer.seek(buffer.size() - qint64(sizeof(int)));
    readStream.readFutureTable();
    buffer.seek(0);
    (to->*member)->prp_readProperty(readStream);
    buffer.close();
}

#endif // BOUNDINGBOX_H
