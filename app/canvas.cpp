#include "canvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include "undoredo.h"
#include "GUI/mainwindow.h"
#include "MovablePoints/pathpivot.h"
#include "Boxes/imagebox.h"
#include "Sound/soundcomposition.h"
#include "Boxes/textbox.h"
#include "GUI/BoxesList/OptimalScrollArea/scrollwidgetvisiblepart.h"
#include "Sound/singlesound.h"
#include "global.h"
#include "pointhelpers.h"
#include "Boxes/linkbox.h"
#include "clipboardcontainer.h"
#include "Boxes/paintbox.h"
#include <QFile>
#include "renderinstancesettings.h"
#include "videoencoder.h"
#include "PropertyUpdaters/displayedfillstrokesettingsupdater.h"
#include "Animators/effectanimators.h"
#include "PixmapEffects/pixmapeffect.h"
#include "PixmapEffects/rastereffects.h"
#include "MovablePoints/smartnodepoint.h"
#include "Boxes/internallinkcanvas.h"
#include "pointtypemenu.h"
#include "Animators/transformanimator.h"

using namespace std::chrono;
Canvas::Canvas(CanvasWindow *canvasWidget,
               const int &canvasWidth, const int &canvasHeight,
               const int &frameCount, const qreal &fps) :
    LayerBox(TYPE_CANVAS) {
    mMainWindow = MainWindow::getInstance();
    setCurrentBrush(mMainWindow->getCurrentBrush());
    std::function<bool(int)> changeFrameFunc =
    [this](const int& undoRedoFrame) {
        if(undoRedoFrame != mMainWindow->getCurrentFrame()) {
            mMainWindow->setCurrentFrame(undoRedoFrame);
            return true;
        }
        return false;
    };
    mUndoRedoStack = SPtrCreate(UndoRedoStack)(changeFrameFunc);
    mFps = fps;
    connect(this, &Canvas::nameChanged, this, &Canvas::emitCanvasNameChanged);
    mBackgroundColor->qra_setCurrentValue(QColor(75, 75, 75));
    ca_addChildAnimator(mBackgroundColor);
    mBackgroundColor->prp_setInheritedUpdater(
                SPtrCreate(DisplayedFillStrokeSettingsUpdater)(this));
    mSoundComposition = qsptr<SoundComposition>::create(this);
    auto soundsAnimatorContainer = mSoundComposition->getSoundsAnimatorContainer();
    ca_addChildAnimator(GetAsSPtr(soundsAnimatorContainer, Property));

    mMaxFrame = frameCount;

    mResolutionFraction = 1;

    mWidth = canvasWidth;
    mHeight = canvasHeight;
    mVisibleWidth = mWidth;
    mVisibleHeight = mHeight;
    mCanvasWindow = canvasWidget;
    mCanvasWidget = mCanvasWindow->getCanvasWidget();

    mCurrentBoxesGroup = this;
    mIsCurrentGroup = true;

    mRotPivot = SPtrCreate(PathPivot)(this);

    ca_removeChildAnimator(mTransformAnimator);

    //anim_setAbsFrame(0);

    //fitCanvasToSize();
    //setCanvasMode(MOVE_PATH);
}

QRectF Canvas::getRelBoundingRect(const qreal &) {
    return QRectF(0, 0, mWidth, mHeight);
}

void Canvas::emitCanvasNameChanged() {
    emit canvasNameChanged(this, prp_mName);
}

qreal Canvas::getResolutionFraction() {
    return mResolutionFraction;
}

void Canvas::setResolutionFraction(const qreal &percent) {
    mResolutionFraction = percent;
}

QRectF Canvas::getPixBoundingRect() {
    return QRectF(mCanvasTransform.dx(), mCanvasTransform.dy(),
                  mVisibleWidth, mVisibleHeight);
}

void Canvas::zoomCanvas(const qreal &scaleBy, const QPointF &absOrigin) {
    const QPointF transPoint = -mapCanvasAbsToRel(absOrigin);

    mCanvasTransform.translate(-transPoint.x(), -transPoint.y());
    mCanvasTransform.scale(scaleBy, scaleBy);
    mCanvasTransform.translate(transPoint.x(), transPoint.y());

    mLastPressPosAbs = mCanvasTransform.map(mLastPressPosRel);

    mVisibleHeight = mCanvasTransform.m22()*mHeight;
    mVisibleWidth = mCanvasTransform.m11()*mWidth;

    if(mHoveredNormalSegment.isValid()) mHoveredNormalSegment.generateSkPath();
}

void Canvas::setCurrentGroupParentAsCurrentGroup() {
    setCurrentBoxesGroup(mCurrentBoxesGroup->getParentGroup());
}

#include "GUI/BoxesList/boxscrollwidget.h"
void Canvas::setCurrentBoxesGroup(GroupBox * const group) {
    if(mCurrentBoxesGroup) {
        mCurrentBoxesGroup->setIsCurrentGroup_k(false);
        disconnect(mCurrentBoxesGroup, &GroupBox::setParentAsCurrentGroup,
                   this, &Canvas::setCurrentGroupParentAsCurrentGroup);
    }
    clearBoxesSelection();
    clearPointsSelection();
    clearCurrentSmartEndPoint();
    clearLastPressedPoint();
    mCurrentBoxesGroup = group;
    connect(mCurrentBoxesGroup, &GroupBox::setParentAsCurrentGroup,
            this, &Canvas::setCurrentGroupParentAsCurrentGroup);
    group->setIsCurrentGroup_k(true);

    //mMainWindow->getObjectSettingsList()->setMainTarget(mCurrentBoxesGroup);
    SWT_scheduleWidgetsContentUpdateWithTarget(mCurrentBoxesGroup,
                                               SWT_TARGET_CURRENT_GROUP);
}

void Canvas::updateHoveredBox() {
    mHoveredBox = mCurrentBoxesGroup->getBoxAt(mCurrentMouseEventPosRel);
}

void Canvas::updateHoveredPoint() {
    mHoveredPoint_d = getPointAtAbsPos(mCurrentMouseEventPosRel,
                                       mCurrentMode,
                                       1/mCanvasTransform.m11());
}

void Canvas::updateHoveredEdge() {
    if(mCurrentMode != MOVE_POINT || mHoveredPoint_d) {
        clearHoveredEdge();
        return;
    }
    mHoveredNormalSegment = getSmartEdgeAt(mCurrentMouseEventPosRel);
    if(mHoveredNormalSegment.isValid())
        mHoveredNormalSegment.generateSkPath();
}

void Canvas::updateHoveredElements() {
    updateHoveredPoint();
    updateHoveredEdge();
    updateHoveredBox();
}

void Canvas::drawTransparencyMesh(SkCanvas *canvas,
                                  const SkRect &viewRect) {
    if(mBackgroundColor->getCurrentColor().alpha() != 255) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(SkColorSetARGB(125, 255, 255, 255));
        SkScalar currX = viewRect.left();
        SkScalar currY = viewRect.top();
        SkScalar widthT = static_cast<SkScalar>(
                    MIN_WIDGET_HEIGHT*0.5*mCanvasTransform.m11());
        SkScalar heightT = widthT;
        bool isOdd = false;
        while(currY < viewRect.bottom()) {
            widthT = heightT;
            if(currY + heightT > viewRect.bottom()) {
                heightT = viewRect.bottom() - currY;
            }
            currX = viewRect.left();
            if(isOdd) currX += widthT;

            while(currX < viewRect.right()) {
                if(currX + widthT > viewRect.right()) {
                    widthT = viewRect.right() - currX;
                }
                canvas->drawRect(SkRect::MakeXYWH(currX, currY,
                                                  widthT, heightT),
                                 paint);
                currX += 2*widthT;
            }

            isOdd = !isOdd;
            currY += heightT;
        }
    }
}

void Canvas::renderSk(SkCanvas * const canvas,
                      GrContext* const grContext) {
    const SkRect viewRect = toSkRect(getPixBoundingRect());

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kFill_Style);

    const SkScalar reversedRes = toSkScalar(1/mResolutionFraction);
    if(isPreviewingOrRendering()) {
        drawTransparencyMesh(canvas, viewRect);
        if(mCurrentPreviewContainer) {
            canvas->save();

            canvas->concat(toSkMatrix(mCanvasTransform));
            canvas->scale(reversedRes, reversedRes);
            mCurrentPreviewContainer->drawSk(canvas, nullptr, grContext);

            canvas->restore();
        }
    } else {
        if(!mClipToCanvasSize) {
            paint.setColor(SkColorSetARGB(255, 75, 75, 75));
            const auto bgRect = mCanvasTransform.mapRect(getMaxBoundsRect());
            canvas->drawRect(toSkRect(bgRect), paint);
        }
        const bool drawCanvas = mCurrentPreviewContainer &&
                !mCurrentPreviewContainerOutdated;
        drawTransparencyMesh(canvas, viewRect);


        if(!mClipToCanvasSize || !drawCanvas) {
            canvas->saveLayer(nullptr, nullptr);
            paint.setColor(toSkColor(mBackgroundColor->getCurrentColor()));
            canvas->drawRect(viewRect, paint);
            canvas->concat(toSkMatrix(mCanvasTransform));
            for(const auto& box : mContainedBoxes) {
                if(box->isVisibleAndInVisibleDurationRect())
                    box->drawPixmapSk(canvas, grContext);
            }
            canvas->restore();
        }
        canvas->concat(toSkMatrix(mCanvasTransform));
        if(drawCanvas) {
            canvas->save();
            canvas->scale(reversedRes, reversedRes);
            mCurrentPreviewContainer->drawSk(canvas, nullptr, grContext);
            canvas->restore();
        }

        const qreal qInvZoom = 1/mCanvasTransform.m11();
        const SkScalar invZoom = toSkScalar(qInvZoom);
        if(!mCurrentBoxesGroup->SWT_isCanvas())
            mCurrentBoxesGroup->drawBoundingRect(canvas, invZoom);
        for(const auto& box : mSelectedBoxes) {
            canvas->save();
            box->drawBoundingRect(canvas, invZoom);
            box->drawCanvasControls(canvas, mCurrentMode, invZoom);
            canvas->restore();
        }

        if(mCurrentMode == CanvasMode::MOVE_BOX ||
           mCurrentMode == CanvasMode::MOVE_POINT) {

            if(mRotPivot->isScaling() || mRotPivot->isRotating()) {
                mRotPivot->drawSk(canvas, mCurrentMode, invZoom, false);
                paint.setStyle(SkPaint::kStroke_Style);
                paint.setColor(SK_ColorBLACK);
                const SkScalar intervals[2] = {MIN_WIDGET_HEIGHT*0.25f*invZoom,
                                               MIN_WIDGET_HEIGHT*0.25f*invZoom};
                paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 0));
                canvas->drawLine(toSkPoint(mRotPivot->getAbsolutePos()),
                                 toSkPoint(mLastMouseEventPosRel), paint);
                paint.setPathEffect(nullptr);
            } else if(!mIsMouseGrabbing || mRotPivot->isSelected()) {
                mRotPivot->drawSk(canvas, mCurrentMode, invZoom, false);
            }
        }

        if(mSelecting) {
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setColor(SkColorSetARGB(255, 0, 0, 255));
            paint.setStrokeWidth(2*invZoom);
            const SkScalar intervals[2] = {MIN_WIDGET_HEIGHT*0.25f*invZoom,
                                           MIN_WIDGET_HEIGHT*0.25f*invZoom};
            paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 0));
            canvas->drawRect(toSkRect(mSelectionRect), paint);
            paint.setPathEffect(nullptr);
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

        if(mPaintDrawableBox) {
            const QRect widRect(0, 0, mCanvasWidget->width(),
                                mCanvasWidget->height());
            const auto canvasRect = mCanvasTransform.inverted().mapRect(widRect);
            const auto pDrawTrans = mPaintDrawableBox->getTotalTransform();
            const auto relDRect = pDrawTrans.inverted().mapRect(canvasRect);
            const auto absPos = QPointF(0, 0);//mPaintDrawableBox->getAbsolutePos();
            canvas->concat(toSkMatrix(pDrawTrans));
            mPaintDrawable->drawOnCanvas(canvas, absPos.toPoint(), &relDRect);
        }

        canvas->resetMatrix();

        if(!mClipToCanvasSize) {
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(2);
            paint.setColor(SK_ColorBLACK);
            canvas->drawRect(viewRect.makeInset(1, 1), paint);
        }
        mValueInput.draw(canvas, mCanvasWidget->height() - MIN_WIDGET_HEIGHT);
    }

    if(mCanvasWindow->hasFocus()) {
        paint.setColor(SK_ColorRED);
        paint.setStrokeWidth(4);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(SkRect::MakeWH(mCanvasWidget->width(),
                                        mCanvasWidget->height()),
                                        paint);
    }
}

void Canvas::setMaxFrame(const int &frame) {
    mMaxFrame = frame;
}

stdsptr<BoundingBoxRenderData> Canvas::createRenderData() {
    return SPtrCreate(CanvasRenderData)(this);
}

const SimpleBrushWrapper *Canvas::getCurrentBrush() const {
    return mCurrentBrush;
}

void Canvas::incBrushRadius() {
    mCurrentBrush->incPaintBrushSize(0.3);
}

void Canvas::decBrushRadius() {
    mCurrentBrush->decPaintBrushSize(0.3);
}

bool Canvas::isMovingPath() {
    return mCurrentMode == CanvasMode::MOVE_BOX;
}

QSize Canvas::getCanvasSize() {
    return QSize(mWidth, mHeight);
}

void Canvas::setPreviewing(const bool &bT) {
    mPreviewing = bT;
}

void Canvas::setRenderingPreview(const bool &bT) {
    mRenderingPreview = bT;
}

void Canvas::setOutputRendering(const bool &bT) {
    mRenderingOutput = bT;
}

void Canvas::setCurrentPreviewContainer(const int& relFrame) {
    auto cont = mCacheHandler.atRelFrame(relFrame);
    setCurrentPreviewContainer(GetAsSPtr(cont, ImageCacheContainer));
}

void Canvas::setCurrentPreviewContainer(const stdsptr<ImageCacheContainer>& cont) {
    setLoadingPreviewContainer(nullptr);
    if(cont == mCurrentPreviewContainer) return;
    if(mCurrentPreviewContainer) {
        if(!mRenderingPreview) {
            mCurrentPreviewContainer->setBlocked(false);
//            if(mRenderingOutput) { // !!! dont keep frames in memory when rendering output
//                mCurrentPreviewContainer->freeAndRemove();
//            }
        }
    }
    if(!cont) {
        mCurrentPreviewContainer.reset();
        return;
    }
    mCurrentPreviewContainer = cont;
    mCurrentPreviewContainer->setBlocked(true);
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

FrameRange Canvas::prp_getIdenticalRelRange(const int &relFrame) const {
    const auto groupRange = LayerBox::prp_getIdenticalRelRange(relFrame);
    //FrameRange canvasRange{0, mMaxFrame};
    return groupRange;//*canvasRange;
}

void Canvas::renderDataFinished(BoundingBoxRenderData *renderData) {
    const auto range = prp_getIdenticalRelRange(renderData->fRelFrame);
//    auto rendRange = range*FrameRange{0, mMaxFrame};
//    for(int i = rendRange.fMin; i <= rendRange.fMax; i++) {
//        const auto path = "/home/ailuropoda/Documents/kom/rend5/" + QString::number(i) + ".png";
//        SkiaHelpers::saveImage(path, renderData->fRenderedImage);
//    }
    auto cont = mCacheHandler.atRelFrame<ImageCacheContainer>(range.fMin);
    if(cont) {
        cont->replaceImageSk(renderData->fRenderedImage);
        cont->setRange(range);
    } else {
        cont = mCacheHandler.createNew<ImageCacheContainer>(
                    range, renderData->fRenderedImage);
    }
    if(mPreviewing || mRenderingOutput) {
        cont->setBlocked(true);
    } else {
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
            const bool currentState =
                    renderData->fBoxStateId == mStateId;
            const bool currentFrame =
                    renderData->fRelFrame == anim_getCurrentRelFrame();
            mDrawRenderContainer.setExpired(!currentState || !currentFrame);
            mCurrentPreviewContainerOutdated =
                    mDrawRenderContainer.isExpired();
            setCurrentPreviewContainer(GetAsSPtr(cont, ImageCacheContainer));
        }
    }
    callUpdateSchedulers();
}

void Canvas::prp_afterChangedAbsRange(const FrameRange &range) {
    Property::prp_afterChangedAbsRange(range);
    const int minId = prp_getIdenticalRelRange(range.fMin).fMin;
    const int maxId = prp_getIdenticalRelRange(range.fMax).fMax;
    mCacheHandler.clearRelRange({minId, maxId});
    if(!mCacheHandler.atRelFrame(anim_getCurrentRelFrame())) {
        mCurrentPreviewContainerOutdated = true;
        planScheduleUpdate(Animator::USER_CHANGE);
    }
}

qsptr<BoundingBox> Canvas::createLink() {
    return SPtrCreate(InternalLinkCanvas)(this);
}

ImageBox *Canvas::createImageBox(const QString &path) {
    auto img = SPtrCreate(ImageBox)(path);
    mCurrentBoxesGroup->addContainedBox(img);
    return img.get();
}

#include "Boxes/imagesequencebox.h"
ImageSequenceBox* Canvas::createAnimationBoxForPaths(const QStringList &paths) {
    auto aniBox = SPtrCreate(ImageSequenceBox)();
    aniBox->setListOfFrames(paths);
    mCurrentBoxesGroup->addContainedBox(aniBox);
    return aniBox.get();
}

#include "Boxes/videobox.h"
VideoBox* Canvas::createVideoForPath(const QString &path) {
    const auto vidBox = SPtrCreate(VideoBox)();
    vidBox->setFilePath(path);
    mCurrentBoxesGroup->addContainedBox(vidBox);
    return vidBox.get();
}

#include "Boxes/linkbox.h"
ExternalLinkBox* Canvas::createLinkToFileWithPath(const QString &path) {
    const auto extLinkBox = SPtrCreate(ExternalLinkBox)();
    extLinkBox->setSrc(path);
    mCurrentBoxesGroup->addContainedBox(extLinkBox);
    return extLinkBox.get();
}

SingleSound* Canvas::createSoundForPath(const QString &path) {
    const auto singleSound = SPtrCreate(SingleSound)();
    getSoundComposition()->addSoundAnimator(singleSound);
    singleSound->setFilePath(path);
    return singleSound.get();
}

void Canvas::scheduleDisplayedFillStrokeSettingsUpdate() {
    mMainWindow->scheduleDisplayedFillStrokeSettingsUpdate();
}

void Canvas::schedulePivotUpdate() {
    if(mRotPivot->isRotating() ||
        mRotPivot->isScaling() ||
        mRotPivot->isSelected()) return;
    if(mLastPressedPoint) {
        if(!mLastPressedPoint->selectionEnabled()) return;
    }
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
    if(mCurrentMode == MOVE_POINT) {
        mRotPivot->setAbsolutePos(getSelectedPointsAbsPivotPos());
    } else if(mCurrentMode == MOVE_BOX) {
        mRotPivot->setAbsolutePos(getSelectedBoxesAbsPivotPos());
    }
}

void Canvas::setCanvasMode(const CanvasMode &mode) {
    if(mIsMouseGrabbing) handleMouseRelease();

    mCurrentMode = mode;
    mSelecting = false;
    updateHoveredElements();
    clearPointsSelection();
    clearCurrentSmartEndPoint();
    clearLastPressedPoint();
    updatePivot();
    updatePaintBox();
}

void Canvas::setPaintDrawable(DrawableAutoTiledSurface * const surf) {
    mPaintDrawable = surf;
}

void Canvas::setPaintBox(PaintBox * const box) {
    if(box) {
        mPaintDrawableBox = GetAsPtr(mSelectedBoxes.last(), PaintBox);
        mPaintAnimSurface = mPaintDrawableBox->getSurface();
        connect(mPaintAnimSurface, &AnimatedSurface::currentSurfaceChanged,
                this, &Canvas::setPaintDrawable);
        mPaintDrawable = mPaintAnimSurface->getCurrentSurface();
    } else if(mPaintDrawableBox) {
        mPaintDrawableBox = nullptr;
        disconnect(mPaintAnimSurface, nullptr, this, nullptr);
        mPaintAnimSurface = nullptr;
        mPaintDrawable = nullptr;
    }
}

void Canvas::updatePaintBox() {
    setPaintBox(nullptr);
    if(mCurrentMode != PAINT_MODE) return;
    for(int i = mSelectedBoxes.count() - 1; i >= 0; i--) {
        const auto& iBox = mSelectedBoxes.at(i);
        if(!iBox->SWT_isPaintBox()) continue;
        setPaintBox(GetAsPtr(mSelectedBoxes.last(), PaintBox));
        break;
    }
}

void Canvas::grabMouseAndTrack() {
    mIsMouseGrabbing = true;
    mCanvasWindow->grabMouse();
}

void Canvas::releaseMouseAndDontTrack() {
    mIsMouseGrabbing = false;
    mCanvasWindow->releaseMouse();
}

bool Canvas::handleTransormationInputKeyEvent(QKeyEvent *event) {
    if(mValueInput.handleTransormationInputKeyEvent(event)) {
        if(mRotPivot->isRotating()) {
            mValueInput.setName("rot");
        } else if(mXOnlyTransform) {
            mValueInput.setName("x");
        } else if(mYOnlyTransform) {
            mValueInput.setName("y");
        } else {
            mValueInput.setName("x, y");
        }
        updateTransformation();
    } else if(event->key() == Qt::Key_Escape) {
        cancelCurrentTransform();
    } else if(event->key() == Qt::Key_Return ||
              event->key() == Qt::Key_Enter) {
        handleMouseRelease();
        mValueInput.clearAndDisableInput();
    } else if(event->key() == Qt::Key_X) {
        mXOnlyTransform = !mXOnlyTransform;
        mYOnlyTransform = false;

        if(mXOnlyTransform) {
            mValueInput.setName("x");
        } else {
            mValueInput.setName("x, y");
        }
        updateTransformation();
    } else if(event->key() == Qt::Key_Y) {
        mYOnlyTransform = !mYOnlyTransform;
        mXOnlyTransform = false;

        if(mYOnlyTransform) {
            mValueInput.setName("y");
        } else {
            mValueInput.setName("x, y");
        }
        updateTransformation();
    } else if(event->key() == Qt::Key_N) {
        for(const auto& box : mSelectedBoxes) {
            const auto paintBox = GetAsPtr(box, PaintBox);
            //paintBox->newEmptyPaintFrameOnCurrentFrame();
        }
    } else {
        return false;
    }

    return true;
}

void Canvas::deleteAction() {
    if(mCurrentMode == MOVE_POINT) {
        removeSelectedPointsAndClearList();
    } else if(mCurrentMode == MOVE_BOX) {
        removeSelectedBoxesAndClearList();
    }
}

bool boxesZSort(const qptr<BoundingBox>& box1,
                const qptr<BoundingBox>& box2) {
    return box1->getZIndex() < box2->getZIndex();
}

void Canvas::copyAction() {
    const auto container = SPtrCreate(BoxesClipboardContainer)();
    mMainWindow->replaceClipboard(container);
    QBuffer target(container->getBytesArray());
    target.open(QIODevice::WriteOnly);
    const int nBoxes = mSelectedBoxes.count();
    target.write(rcConstChar(&nBoxes), sizeof(int));

    std::sort(mSelectedBoxes.begin(), mSelectedBoxes.end(), boxesZSort);
    for(const auto& box : mSelectedBoxes) {
        box->writeBoundingBox(&target);
    }
    target.close();

    BoundingBox::sClearWriteBoxes();
}

void Canvas::pasteAction() {
    const auto container =
            static_cast<BoxesClipboardContainer*>(
            mMainWindow->getClipboardContainer(CCT_BOXES));
    if(!container) return;
    clearBoxesSelection();
    container->pasteTo(mCurrentBoxesGroup);
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
    if(mCurrentMode == MOVE_POINT) {
        selectAllPointsAction();
    } else {//if(mCurrentMode == MOVE_PATH) {
        selectAllBoxesFromBoxesGroup();
    }
}

void Canvas::invertSelectionAction() {
    if(mCurrentMode == MOVE_POINT) {
        QList<stdptr<MovablePoint>> selectedPts = mSelectedPoints_d;
        selectAllPointsAction();
        for(const auto& pt : selectedPts) {
            removePointFromSelection(pt);
        }
    } else {//if(mCurrentMode == MOVE_PATH) {
        QList<qptr<BoundingBox>> boxes = mSelectedBoxes;
        selectAllBoxesFromBoxesGroup();
        for(const auto& box : boxes) {
            box->removeFromSelection();
        }
    }
}

void Canvas::anim_setAbsFrame(const int &frame) {
    if(frame == anim_getCurrentAbsFrame()) return;
    int lastRelFrame = anim_getCurrentRelFrame();
    ComplexAnimator::anim_setAbsFrame(frame);
    const auto cont = mCacheHandler.atRelFrame
            <ImageCacheContainer>(anim_getCurrentRelFrame());
    if(cont) {
        if(cont->storesDataInMemory()) { // !!!
            setCurrentPreviewContainer(GetAsSPtr(cont, ImageCacheContainer));
        } else {// !!!
            setLoadingPreviewContainer(GetAsSPtr(cont, ImageCacheContainer));
        }// !!!
        mCurrentPreviewContainerOutdated = !cont->storesDataInMemory();
    } else {
        const bool difference =
                prp_differencesBetweenRelFrames(lastRelFrame,
                                                anim_getCurrentRelFrame());
        if(difference) {
            mCurrentPreviewContainerOutdated = true;
        }
        if(difference) planScheduleUpdate(Animator::FRAME_CHANGE);
    }

    for(const auto &box : mContainedBoxes) {
        box->anim_setAbsFrame(frame);
    }
    mUndoRedoStack->setFrame(frame);
}

void Canvas::clearSelectionAction() {
    if(mCurrentMode == MOVE_POINT) {
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

bool Canvas::startRotatingAction(const QPointF &cursorPos) {
    if(!isMovingPath() && mCurrentMode != MOVE_POINT) return false;
    if(mSelectedBoxes.isEmpty()) return false;
    if(mCurrentMode == MOVE_POINT) {
        if(mSelectedPoints_d.isEmpty()) return false;
    }
    mTransformationFinishedBeforeMouseRelease = false;
    setLastMouseEventPosAbs(cursorPos);
    setLastMousePressPosAbs(cursorPos);
    mRotPivot->startRotating();
    mDoubleClick = false;
    mFirstMouseMove = true;

    grabMouseAndTrack();
    return true;
}

bool Canvas::startScalingAction(const QPointF &cursorPos) {
    if(!isMovingPath() && mCurrentMode != MOVE_POINT) return false;
    if(mSelectedBoxes.isEmpty()) return false;
    if(mCurrentMode == MOVE_POINT) {
        if(mSelectedPoints_d.isEmpty()) return false;
    }
    mTransformationFinishedBeforeMouseRelease = false;
    mXOnlyTransform = false;
    mYOnlyTransform = false;

    setLastMouseEventPosAbs(cursorPos);
    setLastMousePressPosAbs(cursorPos);
    mRotPivot->startScaling();
    mDoubleClick = false;
    mFirstMouseMove = true;

    grabMouseAndTrack();
    return true;
}

bool Canvas::startMovingAction(const QPointF& cursorPos) {
    if(!isMovingPath() && mCurrentMode != MOVE_POINT) return false;
    mTransformationFinishedBeforeMouseRelease = false;
    mXOnlyTransform = false;
    mYOnlyTransform = false;

    setLastMouseEventPosAbs(cursorPos);
    setLastMousePressPosAbs(cursorPos);
    mDoubleClick = false;
    mFirstMouseMove = true;

    grabMouseAndTrack();
    return true;
}

void Canvas::selectAllBoxesAction() {
    mCurrentBoxesGroup->selectAllBoxesFromBoxesGroup();
}

void Canvas::deselectAllBoxesAction() {
    mCurrentBoxesGroup->deselectAllBoxesFromBoxesGroup();
}

void Canvas::selectAllPointsAction() {
    for(const auto& box : mSelectedBoxes)
        box->selectAllCanvasPts(mSelectedPoints_d, mCurrentMode);
}

void Canvas::selectOnlyLastPressedBox() {
    clearBoxesSelection();
    if(mLastPressedBox)
        addBoxToSelection(mLastPressedBox);
}

void Canvas::selectOnlyLastPressedPoint() {
    clearPointsSelection();
    if(mLastPressedPoint)
        addPointToSelection(mLastPressedPoint);
}

void Canvas::resetTransormation() {
    mCanvasTransform.reset();
    mVisibleHeight = mHeight;
    mVisibleWidth = mWidth;
    moveByRel(QPointF((mCanvasWidget->width() - mVisibleWidth)*0.5,
                      (mCanvasWidget->height() - mVisibleHeight)*0.5) );

}

void Canvas::fitCanvasToSize() {
    mCanvasTransform.reset();
    mVisibleHeight = mHeight + MIN_WIDGET_HEIGHT;
    mVisibleWidth = mWidth + MIN_WIDGET_HEIGHT;
    qreal widthScale = mCanvasWidget->width()/mVisibleWidth;
    qreal heightScale = mCanvasWidget->height()/mVisibleHeight;
    zoomCanvas(qMin(heightScale, widthScale), QPointF(0., 0.));
    moveByRel(QPointF((mCanvasWidget->width() - mVisibleWidth)*0.5,
                      (mCanvasWidget->height() - mVisibleHeight)*0.5) );
}

void Canvas::moveByRel(const QPointF &trans) {
    QPointF transRel = mapAbsPosToRel(trans) -
                       mapAbsPosToRel(QPointF(0, 0));

    mLastPressPosRel = mapAbsPosToRel(mLastPressPosRel);

    mCanvasTransform.translate(transRel.x(), transRel.y());

    mLastPressPosRel = mCanvasTransform.map(mLastPressPosRel);
    schedulePivotUpdate();
}

//void Canvas::updateAfterFrameChanged(const int &currentFrame) {
//    anim_mCurrentAbsFrame = currentFrame;

//    for(const auto& box : mChildBoxes) {
//        box->anim_setAbsFrame(currentFrame);
//    }

//    BoxesGroup::anim_setAbsFrame(currentFrame);
//    //mSoundComposition->getSoundsAnimatorContainer()->anim_setAbsFrame(currentFrame);
//}

bool Canvas::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                 const bool &parentSatisfies,
                                 const bool &parentMainTarget) const {
    Q_UNUSED(parentSatisfies);
    Q_UNUSED(parentMainTarget);
    const SWT_BoxRule &rule = rules.fRule;
    const bool &alwaysShowChildren = rules.fAlwaysShowChildren;
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

void Canvas::setIsCurrentCanvas(const bool &bT) {
    mIsCurrentCanvas = bT;
}

int Canvas::getCurrentFrame() {
    return anim_getCurrentAbsFrame();
}

int Canvas::getFrameCount() {
    return mMaxFrame + 1;
}

int Canvas::getMaxFrame() {
    return mMaxFrame;
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

void Canvas::moveDurationRectForAllSelected(const int &dFrame) {
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

void Canvas::moveMinFrameForAllSelected(const int &dFrame) {
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

void Canvas::moveMaxFrameForAllSelected(const int &dFrame) {
    for(const auto& box : mSelectedBoxes)
        box->moveMaxFrame(dFrame);
}

void Canvas::blockUndoRedo() {
    mUndoRedoStack->blockUndoRedo();
}

void Canvas::unblockUndoRedo() {
    mUndoRedoStack->unblockUndoRedo();
}

void Canvas::callUpdateSchedulers() {
    mMainWindow->queScheduledTasksAndUpdate();
}

bool Canvas::isShiftPressed() {
    return mMainWindow->isShiftPressed();
}

bool Canvas::isShiftPressed(QKeyEvent *event) {
    return event->modifiers() & Qt::ShiftModifier;
}

bool Canvas::isCtrlPressed() {
    return mMainWindow->isCtrlPressed();
}

bool Canvas::isCtrlPressed(QKeyEvent *event) {
    return event->modifiers() & Qt::ControlModifier;
}

bool Canvas::isAltPressed() {
    return mMainWindow->isAltPressed();
}

bool Canvas::isAltPressed(QKeyEvent *event) {
    return event->modifiers() & Qt::AltModifier;
}

void Canvas::paintPress(const ulong ts, const qreal &pressure,
                        const qreal &xTilt, const qreal &yTilt) {
    if(mPaintAnimSurface) {
        if(mPaintAnimSurface->anim_isRecording())
            mPaintAnimSurface->anim_saveCurrentValueAsKey();
    }

    if(mPaintDrawable && mCurrentBrush) {
        const auto& target = mPaintDrawable->surface();
        const auto pDrawTrans = mPaintDrawableBox->getTotalTransform();
        const auto pos = pDrawTrans.inverted().map(mLastMouseEventPosRel);
        const auto roi =
                target.paintPressEvent(mCurrentBrush->getBrush(),
                                       pos, 1, pressure, xTilt, yTilt);
        const QRect qRoi(roi.x, roi.y, roi.width, roi.height);
        mPaintDrawable->pixelRectChanged(qRoi);
        mLastTs = ts;
    }
}

void Canvas::paintMove(const ulong ts, const qreal &pressure,
                       const qreal &xTilt, const qreal &yTilt) {
    if(mPaintDrawable && mCurrentBrush) {
        const auto& target = mPaintDrawable->surface();
        const double dt = (ts - mLastTs);
        const auto pDrawTrans = mPaintDrawableBox->getTotalTransform();
        const auto pos = pDrawTrans.inverted().map(mLastMouseEventPosRel);
        const auto roi =
                target.paintMoveEvent(mCurrentBrush->getBrush(),
                                      pos, dt/1000, pressure,
                                      xTilt, yTilt);
        const QRect qRoi(roi.x, roi.y, roi.width, roi.height);
        mPaintDrawable->pixelRectChanged(qRoi);
    }
    mLastTs = ts;
}

SoundComposition *Canvas::getSoundComposition() {
    return mSoundComposition.get();
}
