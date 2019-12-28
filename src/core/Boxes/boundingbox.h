// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

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
#include "waitingforboxload.h"
#include "MovablePoints/segment.h"
class Canvas;

class QrealAction;
class MovablePoint;

class PathEffect;
class TimelineMovable;
class FillSettingsAnimator;
class OutlineSettingsAnimator;
class PaintSettingsApplier;
class RasterEffectAnimators;
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

enum eBoxType {
    TYPE_VECTOR_PATH,
    TYPE_CIRCLE,
    TYPE_IMAGE,
    TYPE_RECTANGLE,
    TYPE_TEXT,
    TYPE_LAYER,
    TYPE_CANVAS,
    TYPE_INTERNAL_LINK,
    TYPE_INTERNAL_LINK_GROUP,
    TYPE_INTERNAL_LINK_CANVAS,
    TYPE_EXTERNAL_LINK,
    TYPE_VIDEO,
    TYPE_IMAGESQUENCE,
    TYPE_PAINT,
    TYPE_GROUP,
    TYPE_CUSTOM
};

class BoundingBox;
template<typename T> class TypeMenu;
typedef TypeMenu<Property> PropertyMenu;

class BoundingBox : public eBoxOrSound {
    Q_OBJECT
    e_OBJECT
protected:
    BoundingBox(const eBoxType type);
public:
    ~BoundingBox();

    virtual stdsptr<BoxRenderData> createRenderData() = 0;

    static BoundingBox *sGetBoxByDocumentId(const int documentId);

    static void sClearWriteBoxes();

    static void sAddReadBox(BoundingBox * const box);
    static BoundingBox *sGetBoxByReadId(const int readId);
    static void sClearReadBoxes();
    static void sForEveryReadBox(const std::function<void(BoundingBox*)>& func);

    template <typename B, typename T>
    static void sWriteReadMember(B* const from, B* const to, const T member);
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

    virtual void centerPivotPosition();
    virtual QPointF getRelCenterPosition();

    virtual void selectAndAddContainedPointsToList(
            const QRectF &absRect,
            const MovablePoint::PtOp& adder,
            const CanvasMode mode);

    virtual bool relPointInsidePath(const QPointF &relPos) const;

    virtual void setFont(const QFont &font) {
        Q_UNUSED(font)
    }

    virtual void setSelectedFontSize(const qreal fontSize) {
        Q_UNUSED(fontSize)
    }

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

    virtual void setStrokeCapStyle(const SkPaint::Cap capStyle)
    { Q_UNUSED(capStyle) }
    virtual void setStrokeJoinStyle(const SkPaint::Join joinStyle)
    { Q_UNUSED(joinStyle) }
    virtual void setStrokeBrush(SimpleBrushWrapper * const brush)
    { Q_UNUSED(brush) }
    virtual void setStrokeBrushWidthCurve(const qCubicSegment1D& curve)
    { Q_UNUSED(curve) }
    virtual void setStrokeBrushTimeCurve(const qCubicSegment1D& curve)
    { Q_UNUSED(curve) }
    virtual void setStrokeBrushPressureCurve(const qCubicSegment1D& curve)
    { Q_UNUSED(curve) }
    virtual void setStrokeBrushSpacingCurve(const qCubicSegment1D& curve)
    { Q_UNUSED(curve) }
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

    virtual void applyPaintSetting(const PaintSettingsApplier &setting);

    virtual void addPathEffect(const qsptr<PathEffect>&);
    virtual void addFillPathEffect(const qsptr<PathEffect>&);
    virtual void addOutlineBasePathEffect(const qsptr<PathEffect>&);
    virtual void addOutlinePathEffect(const qsptr<PathEffect>&);

    void prp_setupTreeViewMenu(PropertyMenu * const menu);
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
    stdsptr<BoxRenderData> queRender(const qreal relFrame);

    virtual void writeIdentifier(eWriteStream& dst) const;

    virtual void writeBoundingBox(eWriteStream& dst);
    virtual void readBoundingBox(eReadStream& src);

    void setupRasterEffectsF(const qreal relFrame,
                             BoxRenderData * const data);
    virtual SkBlendMode getBlendMode();

    bool SWT_isBoundingBox() const;
    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool parentSatisfies,
                             const bool parentMainTarget) const;
    bool SWT_visibleOnlyIfParentDescendant() const;

    bool SWT_dropSupport(const QMimeData* const data);
    bool SWT_drop(const QMimeData* const data);

    void prp_afterChangedAbsRange(const FrameRange &range,
                                  const bool clip = true);
    void drawAllCanvasControls(SkCanvas * const canvas,
                               const CanvasMode mode,
                               const float invScale,
                               const bool ctrlPressed);

    void anim_setAbsFrame(const int frame);

    void ca_childIsRecordingChanged();

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

    BasicTransformAnimator *getTransformAnimator() const;
    BoxTransformAnimator *getBoxTransformAnimator() const;
    QRectF getRelBoundingRect() const;
    void drawHoveredPathSk(SkCanvas *canvas,
                           const SkPath &path,
                           const float invScale);

    void setRasterEffectsEnabled(const bool enable);
    bool getRasterEffectsEnabled() const;

    void clearRasterEffects();

    void addRasterEffect(const qsptr<RasterEffect> &rasterEffect);
    void removeRasterEffect(const qsptr<RasterEffect> &effect);

    void setBlendModeSk(const SkBlendMode blendMode);

    QPointF mapRelPosToAbs(const QPointF &relPos) const;

    void copyTransformationTo(BoundingBox * const targetBox);
    void copyBoundingBoxDataTo(BoundingBox * const targetBox);

//    int prp_getParentFrameShift() const;

    bool diffsIncludingInherited(const int relFrame1, const int relFrame2) const;
    bool diffsIncludingInherited(const qreal relFrame1, const qreal relFrame2) const;

    bool hasCurrentRenderData(const qreal relFrame) const;
    stdsptr<BoxRenderData> getCurrentRenderData(const qreal relFrame) const;
    BoxRenderData *updateCurrentRenderData(const qreal relFrame);

    void updateDrawRenderContainerTransform();

    void addLinkingBox(BoundingBox *box);
    void removeLinkingBox(BoundingBox *box);
    const QList<BoundingBox*> &getLinkingBoxes() const;


    void incReasonsNotToApplyUglyTransform();
    void decReasonsNotToApplyUglyTransform();

    eBoxType getBoxType() const;

    void requestGlobalPivotUpdateIfSelected();
    void requestGlobalFillStrokeUpdateIfSelected();

    void planUpdate(const UpdateReason reason);

    void planCenterPivotPosition();

    bool visibleForCanvas() const { return mVisibleInScene; }
    void setVisibleForScene(const bool visible) {
        mVisibleInScene = visible;
    }

    virtual void updateIfUsesProgram(const ShaderEffectProgram * const program) const;
protected:
    void setRelBoundingRect(const QRectF& relRect);

    void prp_updateCanvasProps() {
        mCanvasProps.clear();
        ca_execOnDescendants([this](Property * prop) {
            if(prop->drawsOnCanvas()) mCanvasProps.append(prop);
        });
        if(drawsOnCanvas()) mCanvasProps.append(this);
    }

    uint mStateId = 0;

    int mNReasonsNotToApplyUglyTransform = 0;
    int mReadId = -1;
    int mWriteId = -1;
    const int mDocumentId;

    eBoxType mType;
    SkBlendMode mBlendMode = SkBlendMode::kSrcOver;

    QPointF mSavedTransformPivot;

    QRectF mRelRect;
    SkRect mRelRectSk;
    SkPath mSkRelBoundingRectPath;

    BasicTransformAnimator* mParentTransform = nullptr;

    QList<BoundingBox*> mLinkingBoxes;

    RenderDataHandler mRenderDataHandler;
    RenderContainer mDrawRenderContainer;

    const qsptr<BoxTransformAnimator> mTransformAnimator;
    const qsptr<RasterEffectAnimators> mRasterEffectsAnimators;

    bool mVisibleInScene = true;
    bool mCenterPivotPlanned = false;
    bool mUpdatePlanned = false;
    UpdateReason mPlannedReason;

    QList<Property*> mCanvasProps;
private:
    void cancelWaitingTasks();
    void afterTotalTransformChanged(const UpdateReason reason);
signals:
    void globalPivotInfluenced();
    void fillStrokeSettingsChanged();
    void blendModeChanged(SkBlendMode);
};

template <typename B, typename T>
void BoundingBox::sWriteReadMember(B* const from, B* const to, const T member) {
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
