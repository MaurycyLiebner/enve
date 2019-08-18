#include "canvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include "undoredo.h"
#include "MovablePoints/pathpivot.h"
#include "Boxes/imagebox.h"
#include "Sound/soundcomposition.h"
#include "Boxes/textbox.h"
#include "Sound/singlesound.h"
#include "GUI/global.h"
#include "pointhelpers.h"
#include "Boxes/linkbox.h"
#include "clipboardcontainer.h"
#include "Boxes/paintbox.h"
#include <QFile>
#include "PropertyUpdaters/displayedfillstrokesettingsupdater.h"
#include "MovablePoints/smartnodepoint.h"
#include "Boxes/internallinkcanvas.h"
#include "pointtypemenu.h"
#include "Animators/transformanimator.h"
#include "glhelpers.h"
#include "CacheHandlers/imagecachecontainer.h"

Canvas::Canvas(Document &document,
               const int canvasWidth, const int canvasHeight,
               const int frameCount, const qreal fps) :
    ContainerBox(TYPE_CANVAS), mDocument(document), mPaintTarget(this) {
    mParentScene = this;
    connect(&mDocument, &Document::canvasModeSet,
            this, &Canvas::setCanvasMode);
    std::function<bool(int)> changeFrameFunc =
    [this](const int undoRedoFrame) {
        if(mDocument.fActiveScene != this) return false;
        if(undoRedoFrame != anim_getCurrentAbsFrame()) {
            mDocument.setActiveSceneFrame(undoRedoFrame);
            return true;
        }
        return false;
    };
    mUndoRedoStack = enve::make_shared<UndoRedoStack>(changeFrameFunc);
    mFps = fps;

    mBackgroundColor->qra_setCurrentValue(QColor(75, 75, 75));
    ca_addChild(mBackgroundColor);
    mBackgroundColor->prp_setInheritedUpdater(
                enve::make_shared<DisplayedFillStrokeSettingsUpdater>(this));
    mSoundComposition = qsptr<SoundComposition>::create(this);

    mRange = {0, frameCount};

    mResolutionFraction = 1;

    mWidth = canvasWidth;
    mHeight = canvasHeight;

    mCurrentContainer = this;
    mIsCurrentGroup = true;

    mRotPivot = enve::make_shared<PathPivot>(this);

    mTransformAnimator->SWT_hide();

    //anim_setAbsFrame(0);

    //setCanvasMode(MOVE_PATH);
}

qreal Canvas::getResolutionFraction() {
    return mResolutionFraction;
}

void Canvas::setResolutionFraction(const qreal percent) {
    mResolutionFraction = percent;
}

void Canvas::setCurrentGroupParentAsCurrentGroup() {
    setCurrentBoxesGroup(mCurrentContainer->getParentGroup());
}

void Canvas::setCurrentBoxesGroup(ContainerBox * const group) {
    if(mCurrentContainer) {
        mCurrentContainer->setIsCurrentGroup_k(false);
    }
    clearBoxesSelection();
    clearPointsSelection();
    clearCurrentSmartEndPoint();
    clearLastPressedPoint();
    mCurrentContainer = group;
    group->setIsCurrentGroup_k(true);

    emit currentContainerSet(group);
}

void Canvas::updateHoveredBox(const MouseEvent& e) {
    mHoveredBox = mCurrentContainer->getBoxAt(e.fPos);
}

void Canvas::updateHoveredPoint(const MouseEvent& e) {
    mHoveredPoint_d = getPointAtAbsPos(e.fPos, mCurrentMode, 1/e.fScale);
}

void Canvas::updateHoveredEdge(const MouseEvent& e) {
    if(mCurrentMode != CanvasMode::pointTransform || mHoveredPoint_d)
        return mHoveredNormalSegment.clear();
    mHoveredNormalSegment = getSegment(e);
    if(mHoveredNormalSegment.isValid())
        mHoveredNormalSegment.generateSkPath();
}

void Canvas::clearHovered() {
    mHoveredBox.clear();
    mHoveredPoint_d.clear();
    mHoveredNormalSegment.clear();
}

void Canvas::updateHovered(const MouseEvent& e) {
    updateHoveredPoint(e);
    updateHoveredEdge(e);
    updateHoveredBox(e);
}

void drawTransparencyMesh(SkCanvas * const canvas,
                          const SkRect &drawRect) {
    SkPaint paint;
    SkBitmap bitmap;
    bitmap.setInfo(SkImageInfo::MakeA8(2, 2), 2);
    uint8_t pixels[4] = { 0, 255, 255, 0 };
    bitmap.setPixels(pixels);

    SkMatrix matr;
    const float scale = canvas->getTotalMatrix().getMinScale();
    const float dim = MIN_WIDGET_DIM*0.5f / (scale > 1.f ? 1.f : scale);
    matr.setScale(dim, dim);
    const auto shader = bitmap.makeShader(SkTileMode::kRepeat,
                                          SkTileMode::kRepeat, &matr);
    paint.setShader(shader);
    paint.setColor(SkColorSetARGB(255, 200, 200, 200));
    canvas->drawRect(drawRect, paint);
}

void Canvas::renderSk(SkCanvas * const canvas,
                      const QRect& drawRect,
                      const QMatrix& viewTrans,
                      const bool mouseGrabbing) {
    mDrawnSinceQue = true;
    SkBitmap bitmap;
    bitmap.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat);
    SkPaint paint;
    paint.setStyle(SkPaint::kFill_Style);
    const SkRect canvasRect = SkRect::MakeWH(mWidth, mHeight);
    const qreal qInvZoom = 1/viewTrans.m11();
    const float invZoom = toSkScalar(qInvZoom);
    const SkMatrix skViewTrans = toSkMatrix(viewTrans);
    const QColor bgColor = mBackgroundColor->getColor();
    const float intervals[2] = {MIN_WIDGET_DIM*0.25f*invZoom,
                                   MIN_WIDGET_DIM*0.25f*invZoom};
    const auto dashPathEffect = SkDashPathEffect::Make(intervals, 2, 0);

    canvas->concat(skViewTrans);
    const float reversedRes = toSkScalar(1/mResolutionFraction);
    if(isPreviewingOrRendering()) {
        if(mCurrentPreviewContainer) {
            canvas->clear(SK_ColorBLACK);
            canvas->save();
            if(bgColor.alpha() != 255)
                drawTransparencyMesh(canvas, canvasRect);
            canvas->scale(reversedRes, reversedRes);
            mCurrentPreviewContainer->drawSk(canvas);
            canvas->restore();
        }
        return;
    }
    canvas->save();
    if(mClipToCanvasSize) {
        canvas->clear(SK_ColorBLACK);
        canvas->clipRect(canvasRect);
    } else {
        canvas->clear(SkColorSetARGB(255, 70, 70, 70));
        paint.setColor(SK_ColorBLACK);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setPathEffect(dashPathEffect);
        canvas->drawRect(toSkRect(getCurrentBounds()), paint);
    }
    const bool drawCanvas = mCurrentPreviewContainer &&
            !mCurrentPreviewContainerOutdated;
    if(bgColor.alpha() != 255)
        drawTransparencyMesh(canvas, canvasRect);

    if(!mClipToCanvasSize || !drawCanvas) {
        if(bgColor.alpha() == 255 &&
           skViewTrans.mapRect(canvasRect).contains(toSkRect(drawRect))) {
            canvas->clear(toSkColor(bgColor));
        } else {
            paint.setStyle(SkPaint::kFill_Style);
            paint.setColor(toSkColor(bgColor));
            canvas->drawRect(canvasRect, paint);
        }
        canvas->saveLayer(nullptr, nullptr);
        drawContained(canvas);
        canvas->restore();
    } else if(drawCanvas) {
        canvas->save();
        canvas->scale(reversedRes, reversedRes);
        mCurrentPreviewContainer->drawSk(canvas);
        canvas->restore();
    }

    canvas->restore();

    if(!mCurrentContainer->SWT_isCanvas())
        mCurrentContainer->drawBoundingRect(canvas, invZoom);
    if(!mPaintTarget.isValid()) {
        const bool ctrlPressed = QApplication::queryKeyboardModifiers() & Qt::CTRL;
        for(int i = mSelectedBoxes.count() - 1; i >= 0; i--) {
            const auto& iBox = mSelectedBoxes.at(i);
            canvas->save();
            iBox->drawBoundingRect(canvas, invZoom);
            iBox->drawAllCanvasControls(canvas, mCurrentMode, invZoom, ctrlPressed);
            canvas->restore();
        }
    }

    if(mCurrentMode == CanvasMode::boxTransform ||
       mCurrentMode == CanvasMode::pointTransform) {
        if(mTransMode == TransformMode::rotate ||
           mTransMode == TransformMode::scale) {
            mRotPivot->drawTransforming(canvas, mCurrentMode, invZoom,
                                        MIN_WIDGET_DIM*0.25f*invZoom);
        } else if(!mouseGrabbing || mRotPivot->isSelected()) {
            mRotPivot->drawSk(canvas, mCurrentMode, invZoom, false, false);
        }
    }

    if(mPaintTarget.isValid()) {
        canvas->save();
        mPaintTarget.draw(canvas, viewTrans, drawRect);
        const SkIRect bRect = toSkIRect(mPaintTarget.pixelBoundingRect());
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorRED);
        paint.setPathEffect(dashPathEffect);
        canvas->drawIRect(bRect, paint);
        paint.setPathEffect(nullptr);
        canvas->restore();
    } else {
        if(mSelecting) {
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setPathEffect(dashPathEffect);
            paint.setStrokeWidth(2*invZoom);
            paint.setColor(SkColorSetARGB(255, 0, 55, 255));
            canvas->drawRect(toSkRect(mSelectionRect), paint);
            paint.setStrokeWidth(invZoom);
            paint.setColor(SkColorSetARGB(255, 150, 150, 255));
            canvas->drawRect(toSkRect(mSelectionRect), paint);
            //paint.setPathEffect(nullptr);
        }

        if(mHoveredPoint_d) {
            mHoveredPoint_d->drawHovered(canvas, invZoom);
        } else if(mHoveredNormalSegment.isValid()) {
            mHoveredNormalSegment.drawHoveredSk(canvas, invZoom);
        } else if(mHoveredBox) {
            if(!mCurrentNormalSegment.isValid()) {
                mHoveredBox->drawHoveredSk(canvas, invZoom);
            }
        }
    }

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(invZoom);
    paint.setColor(mClipToCanvasSize ? SK_ColorGRAY : SK_ColorBLACK);
    paint.setPathEffect(nullptr);
    canvas->drawRect(canvasRect, paint);

    canvas->resetMatrix();

    if(mTransMode != TransformMode::none || mValueInput.inputEnabled())
        mValueInput.draw(canvas, drawRect.height() - MIN_WIDGET_DIM);
}

void Canvas::setFrameRange(const FrameRange &range) {
    mRange = range;
    emit newFrameRange(range);
}

stdsptr<BoxRenderData> Canvas::createRenderData() {
    return enve::make_shared<CanvasRenderData>(this);
}

QSize Canvas::getCanvasSize() {
    return QSize(mWidth, mHeight);
}

void Canvas::setPreviewing(const bool bT) {
    mPreviewing = bT;
}

void Canvas::setRenderingPreview(const bool bT) {
    mRenderingPreview = bT;
}

void Canvas::anim_scaleTime(const int pivotAbsFrame, const qreal scale) {
    ContainerBox::anim_scaleTime(pivotAbsFrame, scale);
    //        int newAbsPos = qRound(scale*pivotAbsFrame);
    //        anim_shiftAllKeys(newAbsPos - pivotAbsFrame);
    const int newMin = qRound((mRange.fMin - pivotAbsFrame)*scale);
    const int newMax = qRound((mRange.fMax - pivotAbsFrame)*scale);
    setFrameRange({newMin, newMax});
}

void Canvas::setOutputRendering(const bool bT) {
    mRenderingOutput = bT;
}

void Canvas::setCurrentPreviewContainer(const int relFrame) {
    auto cont = mCacheHandler.atFrame(relFrame);
    setCurrentPreviewContainer(enve::shared<ImageCacheContainer>(cont));
}

void Canvas::setCurrentPreviewContainer(const stdsptr<ImageCacheContainer>& cont) {
    setLoadingPreviewContainer(nullptr);
    if(cont == mCurrentPreviewContainer) return emit requestUpdate();
    if(mCurrentPreviewContainer) {
        if(!mRenderingPreview)
            mCurrentPreviewContainer->setBlocked(false);
    }

    mCurrentPreviewContainer = cont;
    if(cont) cont->setBlocked(true);
    emit requestUpdate();
}

void Canvas::setLoadingPreviewContainer(
        const stdsptr<ImageCacheContainer>& cont) {
    if(cont == mLoadingPreviewContainer) return;
    if(mLoadingPreviewContainer.get()) {
        mLoadingPreviewContainer->setLoadTargetCanvas(nullptr);
        if(!mRenderingPreview || mRenderingOutput) {
            mLoadingPreviewContainer->setBlocked(false);
        }
    }
    if(!cont) {
        mLoadingPreviewContainer.reset();
        return;
    }
    mLoadingPreviewContainer = cont;
    cont->setLoadTargetCanvas(this);
    if(!cont->storesDataInMemory()) {
        cont->scheduleLoadFromTmpFile();
    }
    mLoadingPreviewContainer->setBlocked(true);
}

FrameRange Canvas::prp_getIdenticalRelRange(const int relFrame) const {
    const auto groupRange = ContainerBox::prp_getIdenticalRelRange(relFrame);
    //FrameRange canvasRange{0, mMaxFrame};
    return groupRange;//*canvasRange;
}

void Canvas::renderDataFinished(BoxRenderData *renderData) {
    if(renderData->fBoxStateId < mLastStateId) return;
    const int relFrame = qRound(renderData->fRelFrame);
    mLastStateId = renderData->fBoxStateId;
    const bool currentState = renderData->fBoxStateId == mStateId;

    const auto range = prp_getIdenticalRelRange(relFrame);
    const auto cont = enve::make_shared<ImageCacheContainer>(
                renderData->fRenderedImage, range,
                currentState ? &mCacheHandler : nullptr);
    if(currentState) mCacheHandler.add(cont);

    if((mPreviewing || mRenderingOutput) &&
       mCurrRenderRange.inRange(relFrame)) {
        cont->setBlocked(true);
    } else {
        auto currentRenderData = mDrawRenderContainer.getSrcRenderData();
        bool newerSate = true;
        bool closerFrame = true;
        if(currentRenderData) {
            newerSate = currentRenderData->fBoxStateId < renderData->fBoxStateId;
            const int finishedFrameDist = qAbs(anim_getCurrentRelFrame() -
                                               relFrame);
            const int oldFrameDist = qAbs(anim_getCurrentRelFrame() -
                                          qRound(currentRenderData->fRelFrame));
            closerFrame = finishedFrameDist < oldFrameDist;
        }
        if(newerSate || closerFrame) {
            mDrawRenderContainer.setSrcRenderData(renderData);
            const bool currentFrame = relFrame == anim_getCurrentRelFrame();
            const bool outdated = !currentState || !currentFrame;
            mDrawRenderContainer.setExpired(outdated);
            mCurrentPreviewContainerOutdated = outdated;
            setCurrentPreviewContainer(cont);
        } else if(mRenderingPreview && mCurrRenderRange.inRange(relFrame)) {
            cont->setBlocked(true);
        }
    }
}

void Canvas::prp_afterChangedAbsRange(const FrameRange &range, const bool clip) {
    Property::prp_afterChangedAbsRange(range, clip);
    const int minId = prp_getIdenticalRelRange(range.fMin).fMin;
    const int maxId = prp_getIdenticalRelRange(range.fMax).fMax;
    mCacheHandler.remove({minId, maxId});
    if(!mCacheHandler.atFrame(anim_getCurrentRelFrame())) {
        mCurrentPreviewContainerOutdated = true;
        planScheduleUpdate(UpdateReason::userChange);
    }
}

qsptr<BoundingBox> Canvas::createLink() {
    return enve::make_shared<InternalLinkCanvas>(this);
}

ImageBox *Canvas::createImageBox(const QString &path) {
    const auto img = enve::make_shared<ImageBox>(path);
    img->planCenterPivotPosition();
    mCurrentContainer->addContained(img);
    return img.get();
}

#include "Boxes/imagesequencebox.h"
ImageSequenceBox* Canvas::createAnimationBoxForPaths(const QString &folderPath) {
    const auto aniBox = enve::make_shared<ImageSequenceBox>();
    aniBox->planCenterPivotPosition();
    aniBox->setFolderPath(folderPath);
    mCurrentContainer->addContained(aniBox);
    return aniBox.get();
}

#include "Boxes/videobox.h"
VideoBox* Canvas::createVideoForPath(const QString &path) {
    const auto vidBox = enve::make_shared<VideoBox>();
    vidBox->planCenterPivotPosition();
    vidBox->setFilePath(path);
    mCurrentContainer->addContained(vidBox);
    return vidBox.get();
}

#include "Boxes/linkbox.h"
ExternalLinkBox* Canvas::createLinkToFileWithPath(const QString &path) {
    const auto extLinkBox = enve::make_shared<ExternalLinkBox>();
    extLinkBox->setSrc(path);
    mCurrentContainer->addContained(extLinkBox);
    return extLinkBox.get();
}

SingleSound* Canvas::createSoundForPath(const QString &path) {
    const auto singleSound = enve::make_shared<SingleSound>();
    singleSound->setFilePath(path);
    mCurrentContainer->addContained(singleSound);
    return singleSound.get();
}

void Canvas::schedulePivotUpdate() {
    if(mTransMode == TransformMode::rotate ||
       mTransMode == TransformMode::scale ||
       mRotPivot->isSelected()) return;
    mPivotUpdateNeeded = true;
}

void Canvas::updatePivotIfNeeded() {
    if(mPivotUpdateNeeded) {
        mPivotUpdateNeeded = false;
        updatePivot();
    }
}

void Canvas::makePointCtrlsSymmetric() {
    setPointCtrlsMode(CtrlsMode::CTRLS_SYMMETRIC);
}

void Canvas::makePointCtrlsSmooth() {
    setPointCtrlsMode(CtrlsMode::CTRLS_SMOOTH);
}

void Canvas::makePointCtrlsCorner() {
    setPointCtrlsMode(CtrlsMode::CTRLS_CORNER);
}

void Canvas::makeSegmentLine() {
    makeSelectedPointsSegmentsLines();
}

void Canvas::makeSegmentCurve() {
    makeSelectedPointsSegmentsCurves();
}

void Canvas::moveSecondSelectionPoint(const QPointF &pos) {
    mSelectionRect.setBottomRight(pos);
}

void Canvas::startSelectionAtPoint(const QPointF &pos) {
    mSelecting = true;
    mSelectionRect.setTopLeft(pos);
    mSelectionRect.setBottomRight(pos);
}

void Canvas::updatePivot() {
    if(mCurrentMode == CanvasMode::pointTransform) {
        mRotPivot->setAbsolutePos(getSelectedPointsAbsPivotPos());
    } else if(mCurrentMode == CanvasMode::boxTransform) {
        mRotPivot->setAbsolutePos(getSelectedBoxesAbsPivotPos());
    }
}

void Canvas::setCanvasMode(const CanvasMode mode) {
    mCurrentMode = mode;
    mSelecting = false;
    clearPointsSelection();
    clearCurrentSmartEndPoint();
    clearLastPressedPoint();
    updatePivot();
    updatePaintBox();
}

void PaintTarget::afterPaintAnimSurfaceChanged() {
    if(mPaintPressedSinceUpdate && mPaintAnimSurface) {
        mPaintAnimSurface->prp_afterChangedRelRange(
                    mPaintAnimSurface->prp_getIdenticalRelRange(
                        mPaintAnimSurface->anim_getCurrentRelFrame()));
        mPaintPressedSinceUpdate = false;
    }
}

void Canvas::updatePaintBox() {
    mPaintTarget.setPaintBox(nullptr);
    if(mCurrentMode != CanvasMode::paint) return;
    for(int i = mSelectedBoxes.count() - 1; i >= 0; i--) {
        const auto& iBox = mSelectedBoxes.at(i);
        if(iBox->SWT_isPaintBox()) {
            mPaintTarget.setPaintBox(static_cast<PaintBox*>(iBox));
            break;
        }
    }
}

bool Canvas::handlePaintModeKeyPress(const KeyEvent &e) {
    if(mCurrentMode != CanvasMode::paint) return false;
    if(e.fKey == Qt::Key_N && mPaintTarget.isValid()) {
        mPaintTarget.newEmptyFrame();
    } else return false;
    return true;
}

bool Canvas::handleModifierChange(const KeyEvent &e) {
    if(mCurrentMode == CanvasMode::pointTransform) {
        if(e.fKey == Qt::Key_Alt ||
           e.fKey == Qt::Key_Shift ||
           e.fKey == Qt::Key_Meta) {
            handleMovePointMouseMove(e);
            return true;
        } else if(e.fKey == Qt::Key_Control) return true;
    }
    return false;
}

bool Canvas::handleTransormationInputKeyEvent(const KeyEvent &e) {
    if(mValueInput.handleTransormationInputKeyEvent(e.fKey)) {
        if(mTransMode == TransformMode::rotate) mValueInput.setupRotate();
        updateTransformation(e);
    } else if(e.fKey == Qt::Key_Escape) {
        cancelCurrentTransform();
        e.fReleaseMouse();
    } else if(e.fKey == Qt::Key_Return ||
              e.fKey == Qt::Key_Enter) {
        handleLeftMouseRelease(e);
    } else if(e.fKey == Qt::Key_X) {
        mValueInput.switchXOnlyMode();
        updateTransformation(e);
    } else if(e.fKey == Qt::Key_Y) {
        mValueInput.switchYOnlyMode();
        updateTransformation(e);
    } else return false;
    return true;
}

void Canvas::deleteAction() {
    if(mCurrentMode == CanvasMode::pointTransform) {
        removeSelectedPointsAndClearList();
    } else if(mCurrentMode == CanvasMode::boxTransform) {
        removeSelectedBoxesAndClearList();
    }
}

void Canvas::copyAction() {
    const auto container = enve::make_shared<BoxesClipboard>(mSelectedBoxes.getList());
    Document::sInstance->replaceClipboard(container);
}

void Canvas::pasteAction() {
    const auto container = Document::sInstance->getBoxesClipboard();
    if(!container) return;
    clearBoxesSelection();
    container->pasteTo(mCurrentContainer);
}

void Canvas::cutAction() {
    copyAction();
    deleteAction();
}

void Canvas::duplicateAction() {
    copyAction();
    pasteAction();
}

void Canvas::selectAllAction() {
    if(mCurrentMode == CanvasMode::pointTransform) {
        selectAllPointsAction();
    } else {//if(mCurrentMode == MOVE_PATH) {
        selectAllBoxesFromBoxesGroup();
    }
}

void Canvas::invertSelectionAction() {
    if(mCurrentMode == CanvasMode::pointTransform) {
        QList<MovablePoint*> selectedPts = mSelectedPoints_d;
        selectAllPointsAction();
        for(const auto& pt : selectedPts) removePointFromSelection(pt);
    } else {//if(mCurrentMode == MOVE_PATH) {
        QList<BoundingBox*> boxes = mSelectedBoxes.getList();
        selectAllBoxesFromBoxesGroup();
        for(const auto& box : boxes) removeBoxFromSelection(box);
    }
}

void Canvas::anim_setAbsFrame(const int frame) {
    if(frame == anim_getCurrentAbsFrame()) return;
    const int oldRelFrame = anim_getCurrentRelFrame();
    ContainerBox::anim_setAbsFrame(frame);
    const int newRelFrame = anim_getCurrentRelFrame();

    mPaintTarget.afterPaintAnimSurfaceChanged();

    const auto cont = mCacheHandler.atFrame<ImageCacheContainer>(newRelFrame);
    if(cont) {
        if(cont->storesDataInMemory()) { // !!!
            setCurrentPreviewContainer(cont->ref<ImageCacheContainer>());
        } else {// !!!
            setLoadingPreviewContainer(cont->ref<ImageCacheContainer>());
        }// !!!
        mCurrentPreviewContainerOutdated = !cont->storesDataInMemory();
    } else {
        const bool difference =
                prp_differencesBetweenRelFrames(oldRelFrame, newRelFrame);
        if(difference) {
            mCurrentPreviewContainerOutdated = true;
        }
        if(difference) planScheduleUpdate(UpdateReason::frameChange);
    }

    mUndoRedoStack->setFrame(frame);

    if(mCurrentMode == CanvasMode::paint)
        mPaintTarget.setupOnionSkin();
    emit currentFrameChanged(frame);
}

void Canvas::clearSelectionAction() {
    if(mCurrentMode == CanvasMode::pointTransform) {
        clearPointsSelection();
    } else {//if(mCurrentMode == MOVE_PATH) {
        clearPointsSelection();
        clearBoxesSelection();
    }
}

void Canvas::clearParentForSelected() {
    for(int i = 0; i < mSelectedBoxes.count(); i++) {
        mSelectedBoxes.at(i)->clearParent();
    }
}

void Canvas::setParentToLastSelected() {
    if(mSelectedBoxes.count() > 1) {
        const auto& lastBox = mSelectedBoxes.last();
        const auto trans = lastBox->getTransformAnimator();
        for(int i = 0; i < mSelectedBoxes.count() - 1; i++) {
            mSelectedBoxes.at(i)->setParentTransform(trans);
        }
    }
}

bool Canvas::startRotatingAction(const KeyEvent &e) {
    if(mCurrentMode != CanvasMode::boxTransform &&
       mCurrentMode != CanvasMode::pointTransform) return false;
    if(mSelectedBoxes.isEmpty()) return false;
    if(mCurrentMode == CanvasMode::pointTransform) {
        if(mSelectedPoints_d.isEmpty()) return false;
    }
    mValueInput.clearAndDisableInput();
    mValueInput.setupRotate();

    mRotPivot->setMousePos(e.fPos);
    mTransMode = TransformMode::rotate;
    mRotHalfCycles = 0;
    mLastDRot = 0;

    mDoubleClick = false;
    mFirstMouseMove = true;
    e.fGrabMouse();
    return true;
}

bool Canvas::startScalingAction(const KeyEvent &e) {
    if(mCurrentMode != CanvasMode::boxTransform &&
       mCurrentMode != CanvasMode::pointTransform) return false;

    if(mSelectedBoxes.isEmpty()) return false;
    if(mCurrentMode == CanvasMode::pointTransform) {
        if(mSelectedPoints_d.isEmpty()) return false;
    }
    mValueInput.clearAndDisableInput();
    mValueInput.setupScale();

    mRotPivot->setMousePos(e.fPos);
    mTransMode = TransformMode::scale;
    mDoubleClick = false;
    mFirstMouseMove = true;
    e.fGrabMouse();
    return true;
}

bool Canvas::startMovingAction(const KeyEvent &e) {
    if(mCurrentMode != CanvasMode::boxTransform &&
       mCurrentMode != CanvasMode::pointTransform) return false;
    mValueInput.clearAndDisableInput();
    mValueInput.setupMove();

    mTransMode = TransformMode::move;
    mDoubleClick = false;
    mFirstMouseMove = true;
    e.fGrabMouse();
    return true;
}

void Canvas::selectAllBoxesAction() {
    mCurrentContainer->selectAllBoxesFromBoxesGroup();
}

void Canvas::deselectAllBoxesAction() {
    mCurrentContainer->deselectAllBoxesFromBoxesGroup();
}

void Canvas::selectAllPointsAction() {
    for(const auto& box : mSelectedBoxes)
        box->selectAllCanvasPts(mSelectedPoints_d, mCurrentMode);
}

void Canvas::selectOnlyLastPressedBox() {
    clearBoxesSelection();
    if(mPressedBox) addBoxToSelection(mPressedBox);
}

void Canvas::selectOnlyLastPressedPoint() {
    clearPointsSelection();
    if(mPressedPoint) addPointToSelection(mPressedPoint);
}

//void Canvas::updateAfterFrameChanged(const int currentFrame) {
//    anim_mCurrentAbsFrame = currentFrame;

//    for(const auto& box : mChildBoxes) {
//        box->anim_setAbsFrame(currentFrame);
//    }

//    BoxesGroup::anim_setAbsFrame(currentFrame);
//    //mSoundComposition->getSoundsAnimatorContainer()->anim_setAbsFrame(currentFrame);
//}

bool Canvas::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                 const bool parentSatisfies,
                                 const bool parentMainTarget) const {
    Q_UNUSED(parentSatisfies);
    Q_UNUSED(parentMainTarget);
    const SWT_BoxRule &rule = rules.fRule;
    const bool alwaysShowChildren = rules.fAlwaysShowChildren;
    if(alwaysShowChildren) {
        return false;
    } else {
        if(rules.fType == SWT_TYPE_SOUND) return false;

        if(rule == SWT_BR_ALL) {
            return true;
        } else if(rule == SWT_BR_SELECTED) {
            return false;
        } else if(rule == SWT_BR_ANIMATED) {
            return false;
        } else if(rule == SWT_BR_NOT_ANIMATED) {
            return false;
        } else if(rule == SWT_BR_VISIBLE) {
            return true;
        } else if(rule == SWT_BR_HIDDEN) {
            return false;
        } else if(rule == SWT_BR_LOCKED) {
            return false;
        } else if(rule == SWT_BR_UNLOCKED) {
            return true;
        }
    }
    return false;
}

void Canvas::setIsCurrentCanvas(const bool bT) {
    mIsCurrentCanvas = bT;
}

int Canvas::getCurrentFrame() {
    return anim_getCurrentAbsFrame();
}

int Canvas::getMaxFrame() {
    return mRange.fMax;
}

HDDCachableCacheHandler &Canvas::getSoundCacheHandler() {
    return mSoundComposition->getCacheHandler();
}

void Canvas::startDurationRectPosTransformForAllSelected() {
    for(const auto& box : mSelectedBoxes)
        box->startDurationRectPosTransform();
}

void Canvas::finishDurationRectPosTransformForAllSelected() {
    for(const auto& box : mSelectedBoxes)
        box->finishDurationRectPosTransform();
}

void Canvas::moveDurationRectForAllSelected(const int dFrame) {
    for(const auto& box : mSelectedBoxes)
        box->moveDurationRect(dFrame);
}

void Canvas::startMinFramePosTransformForAllSelected() {
    for(const auto& box : mSelectedBoxes)
        box->startMinFramePosTransform();
}

void Canvas::finishMinFramePosTransformForAllSelected() {
    for(const auto& box : mSelectedBoxes)
        box->finishMinFramePosTransform();
}

void Canvas::moveMinFrameForAllSelected(const int dFrame) {
    for(const auto& box : mSelectedBoxes)
        box->moveMinFrame(dFrame);
}

void Canvas::startMaxFramePosTransformForAllSelected() {
    for(const auto& box : mSelectedBoxes)
        box->startMaxFramePosTransform();
}

void Canvas::finishMaxFramePosTransformForAllSelected() {
    for(const auto& box : mSelectedBoxes)
        box->finishMaxFramePosTransform();
}

void Canvas::moveMaxFrameForAllSelected(const int dFrame) {
    for(const auto& box : mSelectedBoxes)
        box->moveMaxFrame(dFrame);
}

bool Canvas::newUndoRedoSet() {
    const bool ret = mUndoRedoStack->finishSet();
    mUndoRedoStack->startNewSet();
    return ret;
}

void Canvas::undo() {
    mUndoRedoStack->undo();
}

void Canvas::redo() {
    mUndoRedoStack->redo();
}

void Canvas::blockUndoRedo() {
    mUndoRedoStack->blockUndoRedo();
}

void Canvas::unblockUndoRedo() {
    mUndoRedoStack->unblockUndoRedo();
}

SoundComposition *Canvas::getSoundComposition() {
    return mSoundComposition.get();
}

void Canvas::writeBoundingBox(QIODevice * const target) {
    ContainerBox::writeBoundingBox(target);
    const int currFrame = getCurrentFrame();
    target->write(rcConstChar(&currFrame), sizeof(int));
    target->write(rcConstChar(&mClipToCanvasSize), sizeof(bool));
    target->write(rcConstChar(&mWidth), sizeof(int));
    target->write(rcConstChar(&mHeight), sizeof(int));
    target->write(rcConstChar(&mFps), sizeof(qreal));
    target->write(rcConstChar(&mRange), sizeof(FrameRange));
}

void Canvas::readBoundingBox(QIODevice * const target) {
    ContainerBox::readBoundingBox(target);
    int currFrame;
    target->read(rcChar(&currFrame), sizeof(int));
    target->read(rcChar(&mClipToCanvasSize), sizeof(bool));
    target->read(rcChar(&mWidth), sizeof(int));
    target->read(rcChar(&mHeight), sizeof(int));
    target->read(rcChar(&mFps), sizeof(qreal));
    target->read(rcChar(&mRange), sizeof(FrameRange));
    anim_setAbsFrame(currFrame);
}
