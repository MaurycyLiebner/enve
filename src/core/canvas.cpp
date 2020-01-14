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
#include "Boxes/internallinkbox.h"
#include "clipboardcontainer.h"
#include "Boxes/paintbox.h"
#include <QFile>
#include "MovablePoints/smartnodepoint.h"
#include "Boxes/internallinkcanvas.h"
#include "pointtypemenu.h"
#include "Animators/transformanimator.h"
#include "glhelpers.h"
#include "Private/document.h"

Canvas::Canvas(Document &document,
               const int canvasWidth, const int canvasHeight,
               const int frameCount, const qreal fps) :
    mDocument(document), mPaintTarget(this) {
    SceneParentSelfAssign(this);
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
    mSoundComposition = qsptr<SoundComposition>::create(this);

    mRange = {0, frameCount};

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
    prp_afterWholeInfluenceRangeChanged();
    updateAllBoxes(UpdateReason::userChange);
}

void Canvas::setCurrentGroupParentAsCurrentGroup() {
    setCurrentBoxesGroup(mCurrentContainer->getParentGroup());
}

void Canvas::queTasks() {
    if(Actions::sInstance->smoothChange() && mCurrentContainer) {
        if(!mDrawnSinceQue) return;
        mCurrentContainer->queChildrenTasks();
    } else ContainerBox::queTasks();
    mDrawnSinceQue = false;
}

void Canvas::addSelectedForGraph(const int widgetId, GraphAnimator * const anim) {
    const auto it = mSelectedForGraph.find(widgetId);
    if(it == mSelectedForGraph.end()) {
        const auto list = std::make_shared<ConnContextObjList<GraphAnimator*>>();
        mSelectedForGraph.insert({widgetId, list});
    }
    auto& connCtxt = mSelectedForGraph[widgetId]->addObj(anim);
    connCtxt << connect(anim, &QObject::destroyed,
                        this, [this, widgetId, anim]() {
        removeSelectedForGraph(widgetId, anim);
    });
}

bool Canvas::removeSelectedForGraph(const int widgetId, GraphAnimator * const anim) {
    return mSelectedForGraph[widgetId]->removeObj(anim);
}

const ConnContextObjList<GraphAnimator*>* Canvas::getSelectedForGraph(const int widgetId) const {
    const auto it = mSelectedForGraph.find(widgetId);
    if(it == mSelectedForGraph.end()) return nullptr;
    return it->second.get();
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

bool Canvas::getPivotLocal() const {
    return mDocument.fLocalPivot;
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

#include "efiltersettings.h"
void Canvas::renderSk(SkCanvas * const canvas,
                      const QRect& drawRect,
                      const QMatrix& viewTrans,
                      const bool mouseGrabbing) {
    mDrawnSinceQue = true;
    SkPaint paint;
    paint.setStyle(SkPaint::kFill_Style);
    const SkRect canvasRect = SkRect::MakeWH(mWidth, mHeight);
    const qreal zoom = viewTrans.m11();
    const auto filter = eFilterSettings::sDisplay(zoom, mResolutionFraction);
    const qreal qInvZoom = 1/viewTrans.m11();
    const float invZoom = toSkScalar(qInvZoom);
    const SkMatrix skViewTrans = toSkMatrix(viewTrans);
    const QColor bgColor = mBackgroundColor->getColor();
    const float intervals[2] = {MIN_WIDGET_DIM*0.25f*invZoom,
                                MIN_WIDGET_DIM*0.25f*invZoom};
    const auto dashPathEffect = SkDashPathEffect::Make(intervals, 2, 0);

    canvas->concat(skViewTrans);
    if(isPreviewingOrRendering()) {
        if(mSceneFrame) {
            canvas->clear(SK_ColorBLACK);
            canvas->save();
            if(bgColor.alpha() != 255)
                drawTransparencyMesh(canvas, canvasRect);
            const float reversedRes = toSkScalar(1/mSceneFrame->fResolution);
            canvas->scale(reversedRes, reversedRes);
            mSceneFrame->drawSk(canvas, filter);
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
    const bool drawCanvas = mSceneFrame && mSceneFrame->fBoxState == mStateId;
    if(bgColor.alpha() != 255)
        drawTransparencyMesh(canvas, canvasRect);

    if(!mClipToCanvasSize || !drawCanvas) {
        canvas->saveLayer(nullptr, nullptr);
        if(bgColor.alpha() == 255 &&
           skViewTrans.mapRect(canvasRect).contains(toSkRect(drawRect))) {
            canvas->clear(toSkColor(bgColor));
        } else {
            paint.setStyle(SkPaint::kFill_Style);
            paint.setColor(toSkColor(bgColor));
            canvas->drawRect(canvasRect, paint);
        }
        drawContained(canvas, filter);
        canvas->restore();
    } else if(drawCanvas) {
        canvas->save();
        const float reversedRes = toSkScalar(1/mSceneFrame->fResolution);
        canvas->scale(reversedRes, reversedRes);
        mSceneFrame->drawSk(canvas, filter);
        canvas->restore();
    }

    canvas->restore();

    if(!mCurrentContainer->SWT_isCanvas())
        mCurrentContainer->drawBoundingRect(canvas, invZoom);
    if(!mPaintTarget.isValid()) {
        const auto mods = QApplication::queryKeyboardModifiers();
        const bool ctrlPressed = mods & Qt::CTRL && mods & Qt::SHIFT;
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
        mPaintTarget.draw(canvas, viewTrans, drawRect,
                          filter, mDocument.fOnionVisible);
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

void Canvas::setSceneFrame(const int relFrame) {
    const auto cont = mSceneFramesHandler.atFrame(relFrame);
    setSceneFrame(enve::shared<SceneFrameContainer>(cont));
}

void Canvas::setSceneFrame(const stdsptr<SceneFrameContainer>& cont) {
    setLoadingSceneFrame(nullptr);
    mSceneFrame = cont;
    emit requestUpdate();
}

void Canvas::setLoadingSceneFrame(const stdsptr<SceneFrameContainer>& cont) {
    if(mLoadingSceneFrame == cont) return;
    mLoadingSceneFrame = cont;
    if(cont) {
        Q_ASSERT(!cont->storesDataInMemory());
        cont->scheduleLoadFromTmpFile();
    }
}

FrameRange Canvas::prp_getIdenticalRelRange(const int relFrame) const {
    const auto groupRange = ContainerBox::prp_getIdenticalRelRange(relFrame);
    //FrameRange canvasRange{0, mMaxFrame};
    return groupRange;//*canvasRange;
}

void Canvas::renderDataFinished(BoxRenderData *renderData) {
    const bool currentState = renderData->fBoxStateId == mStateId;
    if(currentState) mRenderDataHandler.removeItemAtRelFrame(renderData->fRelFrame);
    else if(renderData->fBoxStateId < mLastStateId) return;
    const int relFrame = qRound(renderData->fRelFrame);
    mLastStateId = renderData->fBoxStateId;

    const auto range = prp_getIdenticalRelRange(relFrame);
    const auto cont = enve::make_shared<SceneFrameContainer>(
                this, renderData, range,
                currentState ? &mSceneFramesHandler : nullptr);
    if(currentState) mSceneFramesHandler.add(cont);

    if(!mPreviewing && !mRenderingOutput){
        bool newerSate = true;
        bool closerFrame = true;
        if(mSceneFrame) {
            newerSate = mSceneFrame->fBoxState < renderData->fBoxStateId;
            const int cRelFrame = anim_getCurrentRelFrame();
            const int finishedFrameDist = qMin(qAbs(cRelFrame - range.fMin),
                                               qAbs(cRelFrame - range.fMax));
            const FrameRange cRange = mSceneFrame->getRange();
            const int oldFrameDist = qMin(qAbs(cRelFrame - cRange.fMin),
                                          qAbs(cRelFrame - cRange.fMax));
            closerFrame = finishedFrameDist < oldFrameDist;
        }
        if(newerSate || closerFrame) {
            mSceneFrameOutdated = !currentState;
            setSceneFrame(cont);
        }
    }
}

void Canvas::prp_afterChangedAbsRange(const FrameRange &range, const bool clip) {
    Property::prp_afterChangedAbsRange(range, clip);
    mSceneFramesHandler.remove(range);
    if(!mSceneFramesHandler.atFrame(anim_getCurrentRelFrame())) {
        mSceneFrameOutdated = true;
        planUpdate(UpdateReason::userChange);
    }
}

qsptr<BoundingBox> Canvas::createLink() {
    return enve::make_shared<InternalLinkCanvas>(this);
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
    setPointCtrlsMode(CtrlsMode::symmetric);
}

void Canvas::makePointCtrlsSmooth() {
    setPointCtrlsMode(CtrlsMode::smooth);
}

void Canvas::makePointCtrlsCorner() {
    setPointCtrlsMode(CtrlsMode::corner);
}

void Canvas::makeSegmentLine() {
    makeSelectedPointsSegmentsLines();
}

void Canvas::makeSegmentCurve() {
    makeSelectedPointsSegmentsCurves();
}

void Canvas::newEmptyPaintFrameAction() {
    if(mPaintTarget.isValid())
        mPaintTarget.newEmptyFrame();
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
    mStylusDrawing = false;
    clearPointsSelection();
    clearCurrentSmartEndPoint();
    clearLastPressedPoint();
    updatePivot();
    updatePaintBox();
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
        newEmptyPaintFrameAction();
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
        mStartTransform = false;
    } else if(e.fKey == Qt::Key_Escape) {
        if(!e.fMouseGrabbing) return false;
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
    if(mSelectedBoxes.isEmpty()) return;
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
    if(mSelectedBoxes.isEmpty()) return;
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
    ContainerBox::anim_setAbsFrame(frame);
    const int newRelFrame = anim_getCurrentRelFrame();

    const auto cont = mSceneFramesHandler.atFrame<SceneFrameContainer>(newRelFrame);
    if(cont) {
        if(cont->storesDataInMemory()) {
            setSceneFrame(cont->ref<SceneFrameContainer>());
        } else {
            setLoadingSceneFrame(cont->ref<SceneFrameContainer>());
        }
        mSceneFrameOutdated = !cont->storesDataInMemory();
    } else {
        mSceneFrameOutdated = true;
        planUpdate(UpdateReason::frameChange);
    }

    mUndoRedoStack->setFrame(frame);

    if(mCurrentMode == CanvasMode::paint) mPaintTarget.setupOnionSkin();
    emit currentFrameChanged(frame);

    schedulePivotUpdate();
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
    mStartTransform = true;
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
    mStartTransform = true;
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
    mStartTransform = true;
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
    const auto adder = [this](MovablePoint* const pt) {
        addPointToSelection(pt);
    };
    for(const auto& box : mSelectedBoxes)
        box->selectAllCanvasPts(adder, mCurrentMode);
}

void Canvas::selectOnlyLastPressedBox() {
    clearBoxesSelection();
    if(mPressedBox) addBoxToSelection(mPressedBox);
}

void Canvas::selectOnlyLastPressedPoint() {
    clearPointsSelection();
    if(mPressedPoint) addPointToSelection(mPressedPoint);
}

bool Canvas::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                 const bool parentSatisfies,
                                 const bool parentMainTarget) const {
    Q_UNUSED(parentSatisfies)
    Q_UNUSED(parentMainTarget)
    const SWT_BoxRule rule = rules.fRule;
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

int Canvas::getCurrentFrame() {
    return anim_getCurrentAbsFrame();
}

int Canvas::getMaxFrame() {
    return mRange.fMax;
}

HddCachableCacheHandler &Canvas::getSoundCacheHandler() {
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

void Canvas::addUndoRedo(const stdfunc<void()>& undo,
                         const stdfunc<void()>& redo) {
    mUndoRedoStack->addUndoRedo(undo, redo);
}

SoundComposition *Canvas::getSoundComposition() {
    return mSoundComposition.get();
}

void Canvas::writeBoundingBox(eWriteStream& dst) {
    ContainerBox::writeBoundingBox(dst);
    const int currFrame = getCurrentFrame();
    dst << currFrame;
    dst << mClipToCanvasSize;
    dst << mWidth;
    dst << mHeight;
    dst << mFps;
    dst.write(&mRange, sizeof(FrameRange));
}

void Canvas::readBoundingBox(eReadStream& src) {
    ContainerBox::readBoundingBox(src);
    int currFrame;
    src >> currFrame;
    src >> mClipToCanvasSize;
    src >> mWidth;
    src >> mHeight;
    src >> mFps;
    src.read(&mRange, sizeof(FrameRange));
    anim_setAbsFrame(currFrame);
}
