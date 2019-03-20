#include "canvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include "undoredo.h"
#include "GUI/mainwindow.h"
#include "MovablePoints/pathpivot.h"
#include "Boxes/imagebox.h"
#include "edge.h"
#include "Sound/soundcomposition.h"
#include "Boxes/textbox.h"
#include "GUI/BoxesList/OptimalScrollArea/scrollwidgetvisiblepart.h"
#include "Sound/singlesound.h"
#include "global.h"
#include "pointhelpers.h"
#include "MovablePoints/nodepoint.h"
#include "Boxes/linkbox.h"
#include "clipboardcontainer.h"
#include "Boxes/paintbox.h"
#include "Paint/brush.h"
#include <QFile>
#include "renderinstancesettings.h"
#include "videoencoder.h"
#include "PropertyUpdaters/displayedfillstrokesettingsupdater.h"
#include "Animators/effectanimators.h"
#include "PixmapEffects/pixmapeffect.h"
#include "PixmapEffects/rastereffects.h"
#include "MovablePoints/smartnodepoint.h"

Canvas::Canvas(CanvasWindow *canvasWidget,
               const int &canvasWidth, const int &canvasHeight,
               const int &frameCount, const qreal &fps) :
    BoxesGroup(TYPE_CANVAS) {
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
    connect(this, SIGNAL(nameChanged(QString)),
            this, SLOT(emitCanvasNameChanged()));
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
    mRotPivot->hide();

    ca_removeChildAnimator(mTransformAnimator);

    anim_setAbsFrame(0);

    //fitCanvasToSize();
    //setCanvasMode(MOVE_PATH);
}

Canvas::~Canvas() {}

QRectF Canvas::getRelBoundingRectAtRelFrame(const qreal &) {
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
    return QRectF(mCanvasTransformMatrix.dx(),
                  mCanvasTransformMatrix.dy(),
                  mVisibleWidth,
                  mVisibleHeight);
}

void Canvas::zoomCanvas(const qreal &scaleBy, const QPointF &absOrigin) {
    QPointF transPoint = -mapCanvasAbsToRel(absOrigin);

    mCanvasTransformMatrix.translate(-transPoint.x(), -transPoint.y());
    mCanvasTransformMatrix.scale(scaleBy, scaleBy);
    mCanvasTransformMatrix.translate(transPoint.x(), transPoint.y());

    mLastPressPosAbs = mCanvasTransformMatrix.map(mLastPressPosRel);

    mVisibleHeight = mCanvasTransformMatrix.m22()*mHeight;
    mVisibleWidth = mCanvasTransformMatrix.m11()*mWidth;

    if(mHoveredEdge_d) mHoveredEdge_d->generatePainterPath();
}

void Canvas::setCurrentGroupParentAsCurrentGroup() {
    setCurrentBoxesGroup(mCurrentBoxesGroup->getParentGroup());
}

#include "GUI/BoxesList/boxscrollwidget.h"
void Canvas::setCurrentBoxesGroup(BoxesGroup *group) {
    if(mCurrentBoxesGroup) {
        mCurrentBoxesGroup->setIsCurrentGroup_k(false);
        disconnect(mCurrentBoxesGroup, &BoxesGroup::setParentAsCurrentGroup,
                   this, &Canvas::setCurrentGroupParentAsCurrentGroup);
    }
    clearBoxesSelection();
    clearBonesSelection();
    clearPointsSelection();
    clearCurrentEndPoint();
    clearCurrentSmartEndPoint();
    clearLastPressedPoint();
    mCurrentBoxesGroup = group;
    connect(mCurrentBoxesGroup, &BoxesGroup::setParentAsCurrentGroup,
            this, &Canvas::setCurrentGroupParentAsCurrentGroup);
    group->setIsCurrentGroup_k(true);

    //mMainWindow->getObjectSettingsList()->setMainTarget(mCurrentBoxesGroup);
    SWT_scheduleWidgetsContentUpdateWithTarget(mCurrentBoxesGroup,
                                               SWT_TARGET_CURRENT_GROUP);
}

void Canvas::drawSelectedSk(SkCanvas *canvas,
                            const CanvasMode &currentCanvasMode,
                            const SkScalar &invScale) {
    for(const auto& box : mSelectedBoxes) {
        box->drawSelectedSk(canvas, currentCanvasMode, invScale);
    }
}
#include "Boxes/bone.h"
void Canvas::updateHoveredBox() {
    mHoveredBox = mCurrentBoxesGroup->getBoxAt(mCurrentMouseEventPosRel);
    mHoveredBone = nullptr;
    if(mHoveredBox && mBonesSelectionEnabled) {
        if(mHoveredBox->SWT_isBonesBox()) {
            mHoveredBone = GetAsPtr(mHoveredBox, BonesBox)->getBoneAtAbsPos(
                        mCurrentMouseEventPosRel);
        }
    }
}

void Canvas::updateHoveredPoint() {
    mHoveredPoint_d = getPointAtAbsPos(mCurrentMouseEventPosRel,
                               mCurrentMode,
                               1/mCanvasTransformMatrix.m11());
}

void Canvas::updateHoveredEdge() {
    mHoveredEdge_d = getEdgeAt(mCurrentMouseEventPosRel);
    if(mHoveredEdge_d != nullptr) {
        mHoveredEdge_d->generatePainterPath();
    }
}

void Canvas::updateHoveredElements() {
    updateHoveredPoint();
    if(mCurrentMode == MOVE_POINT) {
        updateHoveredEdge();
    } else {
        clearHoveredEdge();
    }
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
                    MIN_WIDGET_HEIGHT*0.5*mCanvasTransformMatrix.m11());
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
    SkRect viewRect = QRectFToSkRect(getPixBoundingRect());

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kFill_Style);
    canvas->clear(SK_ColorBLACK);

    if(isPreviewingOrRendering()) {
        drawTransparencyMesh(canvas, viewRect);
        if(mCurrentPreviewContainer) {
            canvas->save();

            canvas->concat(QMatrixToSkMatrix(mCanvasTransformMatrix));
            SkScalar reversedRes = static_cast<SkScalar>(1/mResolutionFraction);
            canvas->scale(reversedRes, reversedRes);
#ifdef CPU_ONLY_RENDER
            mCurrentPreviewContainer->drawSk(canvas, nullptr, nullptr);
#else
            mCurrentPreviewContainer->drawSk(canvas, nullptr, grContext);
#endif

            canvas->restore();
        }
    } else {
        SkScalar invScale = static_cast<SkScalar>(1/mCanvasTransformMatrix.m11());
#ifdef CPU_ONLY_RENDER
        drawTransparencyMesh(canvas, viewRect);
        canvas->concat(QMatrixToSkMatrix(mCanvasTransformMatrix));

        if(mCurrentPreviewContainer) {
            canvas->save();
            SkScalar reversedRes = static_cast<SkScalar>(1/mResolutionFraction);
            canvas->scale(reversedRes, reversedRes);
            mCurrentPreviewContainer->drawSk(canvas, nullptr, nullptr);
            canvas->restore();
        }
#else
        if(!mClipToCanvasSize) {
            paint.setColor(SkColorSetARGB(255, 75, 75, 75));
            canvas->drawRect(QRectFToSkRect(mCanvasTransformMatrix.mapRect(
                                                getMaxBoundsRect())),
                             paint);
        }
        bool drawCanvas =
//                mEffectsAnimators->hasEffects() &&
                mCurrentPreviewContainer &&
                !mCurrentPreviewContainerOutdated;
                //mExpiredPixmap == 0;
        drawTransparencyMesh(canvas, viewRect);


        if(!mClipToCanvasSize || !drawCanvas) {
            canvas->saveLayer(nullptr, nullptr);
            paint.setColor(QColorToSkColor(mBackgroundColor->getCurrentColor()));
            canvas->drawRect(viewRect, paint);
            canvas->concat(QMatrixToSkMatrix(mCanvasTransformMatrix));
            for(const auto& box : mContainedBoxes) {
                box->drawPixmapSk(canvas, grContext);
            }
            canvas->restore();
        }
        canvas->concat(QMatrixToSkMatrix(mCanvasTransformMatrix));
        if(drawCanvas) {
            canvas->save();
            SkScalar reversedRes = static_cast<SkScalar>(1/mResolutionFraction);
            canvas->scale(reversedRes, reversedRes);
            mCurrentPreviewContainer->drawSk(canvas, nullptr, grContext);
            canvas->restore();
        }
#endif
//        QPen pen = QPen(Qt::black, 1.5);
//        pen.setCosmetic(true);
//        p->setPen(pen);
        mCurrentBoxesGroup->drawSelectedSk(canvas, mCurrentMode, invScale);
        drawSelectedSk(canvas, mCurrentMode, invScale);

        if(mCurrentMode == CanvasMode::MOVE_PATH ||
           mCurrentMode == CanvasMode::MOVE_POINT) {

            if(mRotPivot->isScaling() || mRotPivot->isRotating()) {
                mRotPivot->drawSk(canvas, invScale);
                paint.setStyle(SkPaint::kStroke_Style);
                paint.setColor(SK_ColorBLACK);
                SkScalar intervals[2] = {MIN_WIDGET_HEIGHT*0.25f*invScale,
                                         MIN_WIDGET_HEIGHT*0.25f*invScale};
                paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 0));
                canvas->drawLine(qPointToSk(mRotPivot->getAbsolutePos()),
                                 qPointToSk(mLastMouseEventPosRel),
                                 paint);
                paint.setPathEffect(nullptr);
            } else if(!mIsMouseGrabbing || mRotPivot->isSelected()) {
                mRotPivot->drawSk(canvas, invScale);
            }
        }
//        pen = QPen(QColor(0, 0, 255, 125), 2., Qt::DotLine);
//        pen.setCosmetic(true);
//        p->setPen(pen);
        if(mSelecting) {
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setColor(SkColorSetARGB(255, 0, 0, 255));
            paint.setStrokeWidth(2.f*invScale);
            SkScalar intervals[2] = {MIN_WIDGET_HEIGHT*0.25f*invScale,
                                     MIN_WIDGET_HEIGHT*0.25f*invScale};
            paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 0));
            canvas->drawRect(QRectFToSkRect(mSelectionRect), paint);
            paint.setPathEffect(nullptr);
            //SkPath selectionPath;
            //selectionPath.addRect(QRectFToSkRect(mSelectionRect));
            //canvas->drawPath(selectionPath, paint);
        }

        if(mHoveredPoint_d != nullptr) {
            mHoveredPoint_d->drawHovered(canvas, invScale);
        } else if(mHoveredEdge_d != nullptr) {
            mHoveredEdge_d->drawHoveredSk(canvas, invScale);
        } else if(mHoveredBone) {
            mHoveredBone->drawHoveredOnlyThisPathSk(canvas, invScale);
        } else if(mHoveredBox) {
            if(!mCurrentEdge) {
                mHoveredBox->drawHoveredSk(canvas, invScale);
            }
        }
        canvas->resetMatrix();

        if(!mClipToCanvasSize) {
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(2.);
            paint.setColor(SK_ColorBLACK);
            canvas->drawRect(viewRect.makeInset(1, 1),
                             paint);
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
    return mCurrentMode == CanvasMode::MOVE_PATH;
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

void Canvas::setCurrentPreviewContainer(const int& relFrame,
                                        const bool &frameEncoded) {
    auto cont = mCacheHandler.getRenderContainerAtRelFrame(relFrame);
    setCurrentPreviewContainer(
                GetAsSPtr(cont, ImageCacheContainer),
                frameEncoded);
}

void Canvas::setCurrentPreviewContainer(
        const stdsptr<ImageCacheContainer>& cont,
        const bool &frameEncoded) {
    if(mRenderingOutput && !frameEncoded) {
        auto range = prp_getIdenticalRelFrameRange(cont->getRangeMin());
        cont->setRangeMax(range.fMax);
        VideoEncoder::sAddCacheContainerToEncoder(cont);
        return;
    }
    setLoadingPreviewContainer(nullptr);
    if(cont == mCurrentPreviewContainer) return;
    if(mCurrentPreviewContainer) {
        if(!mRenderingPreview || mRenderingOutput) {
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
    mLoadingPreviewContainer->setBlocked(true);
}

FrameRange Canvas::prp_getIdenticalRelFrameRange(const int &relFrame) const {
    FrameRange groupRange =
            BoxesGroup::prp_getIdenticalRelFrameRange(relFrame);
    //FrameRange canvasRange{0, mMaxFrame};
    return groupRange;//*canvasRange;
}

void Canvas::renderDataFinished(BoundingBoxRenderData *renderData) {
    if(renderData->fRedo) {
        scheduleUpdate(renderData->fRelFrame, Animator::USER_CHANGE);
    }
    //qDebug() << renderData->fRelFrame;
    auto range = prp_getIdenticalRelFrameRange(renderData->fRelFrame);
    auto cont = mCacheHandler.getRenderContainerAtRelFrame(range.fMin);
    if(cont) {
        cont->replaceImageSk(renderData->fRenderedImage);
    } else {
        cont = mCacheHandler.createNewRenderContainerAtRelFrame(
                    range, renderData->fRenderedImage);
    }
    //cont->setRelFrameRange(range);
    if(mRenderingPreview || mRenderingOutput || !mPreviewing) {
        auto currentRenderData = mDrawRenderContainer->getSrcRenderData();
        bool newerSate = true;
        bool closerFrame = true;
        if(currentRenderData) {
            newerSate = currentRenderData->fBoxStateId <
                    renderData->fBoxStateId;
            const int finishedFrameDist =
                    qAbs(anim_mCurrentRelFrame - renderData->fRelFrame);
            const int oldFrameDist =
                    qAbs(anim_mCurrentRelFrame - currentRenderData->fRelFrame);
            closerFrame = finishedFrameDist < oldFrameDist;
        }
        if(newerSate || closerFrame) {
            mDrawRenderContainer->setSrcRenderData(renderData);
            const bool currentState =
                    renderData->fBoxStateId == mStateId;
            const bool currentFrame =
                    renderData->fRelFrame == anim_mCurrentRelFrame;
            mDrawRenderContainer->setExpired(!currentState || !currentFrame);
            mCurrentPreviewContainerOutdated =
                    mDrawRenderContainer->isExpired();
            setCurrentPreviewContainer(GetAsSPtr(cont, ImageCacheContainer));
        }
    } else if(mPreviewing) {
        cont->setBlocked(true);
    }
    callUpdateSchedulers();
}

void Canvas::prp_updateAfterChangedAbsFrameRange(const FrameRange &range) {
    if(range.inRange(anim_mCurrentAbsFrame)) {
        mCurrentPreviewContainerOutdated = true;
    }
    mCacheHandler.clearCacheForRelFrameRange(
                prp_absRangeToRelRange(range));
    Property::prp_updateAfterChangedAbsFrameRange(range);
    auto rangeT = prp_getIdenticalRelFrameRange(anim_mCurrentRelFrame);
    if(range.overlaps(rangeT)) scheduleUpdate(Animator::USER_CHANGE);
}

//void Canvas::updatePixmaps() {
//    renderCurrentFrameToPreview();
//}


void Canvas::renderCurrentFrameToOutput(const RenderInstanceSettings &renderDest) {
    Q_UNUSED(renderDest);
    return;
//    QString fileName = renderDest.getOutputDestination();
//    sk_sp<SkData> data = mCurrentPreviewContainer->getImageSk()->
//            encodeToData(SkEncodedImageFormat::kPNG, 100);
//    QFile file;
//    fileName.remove(".png");
//    fileName += QString::number(anim_mCurrentAbsFrame) + ".png";
//    file.setFileName(fileName);
//    if(file.open(QIODevice::WriteOnly) ) {
//        file.write(static_cast<const char*>(data->data()),
//                   static_cast<qint64>(data->size()));
//        file.flush();
//        file.close();
//    }
//    data->unref();
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
    auto vidBox = SPtrCreate(VideoBox)(path);
    mCurrentBoxesGroup->addContainedBox(vidBox);
    return vidBox.get();
}

#include "Boxes/linkbox.h"
ExternalLinkBox* Canvas::createLinkToFileWithPath(const QString &path) {
    auto extLinkBox = SPtrCreate(ExternalLinkBox)();
    extLinkBox->setSrc(path);
    mCurrentBoxesGroup->addContainedBox(extLinkBox);
    return extLinkBox.get();
}

SingleSound* Canvas::createSoundForPath(const QString &path) {
    auto singleSound = SPtrCreate(SingleSound)(path);
    getSoundComposition()->addSoundAnimator(singleSound);
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
        if(mLastPressedPoint->isCtrlPoint()) return;
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
        if(isPointsSelectionEmpty() || mLocalPivot) {
            mRotPivot->hide();
        } else {
            mRotPivot->show();
        }
        if(getPointsSelectionCount() == 1 && !mLocalPivot) {
            mRotPivot->setAbsolutePos(
                        getSelectedPointsAbsPivotPos() +
                            QPointF(0., 20.));
        } else {
            mRotPivot->setAbsolutePos(getSelectedPointsAbsPivotPos());
        }
    } else if(mCurrentMode == MOVE_PATH) {
        if(isSelectionEmpty() ||
           mLocalPivot) {
            mRotPivot->hide();
        } else {
            mRotPivot->show();

        }
        mRotPivot->setAbsolutePos(getSelectedBoxesAbsPivotPos());
    }
}

void Canvas::setCanvasMode(const CanvasMode &mode) {
    if(mIsMouseGrabbing) handleMouseRelease();

    mCurrentMode = mode;
    mSelecting = false;
    mHoveredPoint_d = nullptr;
    clearHoveredEdge();
    clearPointsSelection();
    clearCurrentSmartEndPoint();
    clearCurrentEndPoint();
    clearLastPressedPoint();
    if(mCurrentMode == MOVE_PATH || mCurrentMode == MOVE_POINT) {
        schedulePivotUpdate();
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
            if(box->SWT_isPaintBox()) {
                PaintBox *paintBox = GetAsPtr(box, PaintBox);
                paintBox->newEmptyPaintFrameOnCurrentFrame();
            }
        }
    } else {
        return false;
    }

    return true;
}

void Canvas::deleteAction() {
    if(mCurrentMode == MOVE_POINT) {
        if(isShiftPressed()) {
            removeSelectedPointsApproximateAndClearList();
        } else {
            removeSelectedPointsAndClearList();
        }
    } else if(mCurrentMode == MOVE_PATH) {
        removeSelectedBoxesAndClearList();
    }
}

bool boxesZSort(const qptr<BoundingBox>& box1,
                const qptr<BoundingBox>& box2) {
    return box1->getZIndex() < box2->getZIndex();
}

void Canvas::copyAction() {
    stdsptr<BoxesClipboardContainer> container =
            SPtrCreate(BoxesClipboardContainer)();
    mMainWindow->replaceClipboard(container);
    QBuffer target(container->getBytesArray());
    target.open(QIODevice::WriteOnly);
    int nBoxes = mSelectedBoxes.count();
    target.write(rcChar(&nBoxes), sizeof(int));

    std::sort(mSelectedBoxes.begin(), mSelectedBoxes.end(), boxesZSort);
    for(const auto& box : mSelectedBoxes) {
        box->writeBoundingBox(&target);
    }
    target.close();
}

void Canvas::pasteAction() {
    BoxesClipboardContainer *container =
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
        for(const auto& box : mSelectedBoxes) {
            box->selectAllPoints(this);
        }
    } else {//if(mCurrentMode == MOVE_PATH) {
        selectAllBoxesFromBoxesGroup();
    }
}

void Canvas::invertSelectionAction() {
    if(mCurrentMode == MOVE_POINT) {
        QList<stdptr<MovablePoint>> selectedPts = mSelectedPoints_d;
        for(const auto& box : mSelectedBoxes) {
            box->selectAllPoints(this);
        }
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
    if(frame == anim_mCurrentAbsFrame) return;
    int lastRelFrame = anim_mCurrentRelFrame;
    ComplexAnimator::anim_setAbsFrame(frame);
    ImageCacheContainer * const cont =
            mCacheHandler.getRenderContainerAtRelFrame(anim_mCurrentRelFrame);
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
                                                anim_mCurrentRelFrame);
        if(difference) {
            mCurrentPreviewContainerOutdated = true;
        }
        if(difference) scheduleUpdate(Animator::FRAME_CHANGE);
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
        clearBonesSelection();
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
    if(!mSelectedBones.isEmpty()) {
        Bone *bone = mSelectedBones.last();
        BasicTransformAnimator *trans = bone->getTransformAnimator();
        auto box = bone->getParentBox();
        for(const auto& boxT : mSelectedBoxes) {
            if(boxT == box) continue;
            boxT->setParentTransform(trans);
        }
    } else if(mSelectedBoxes.count() > 1) {
        const auto& lastBox = mSelectedBoxes.last();
        auto trans = lastBox->getTransformAnimator();
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
    for(const auto& box : mSelectedBoxes) {
        box->selectAllPoints(this);
    }
}

void Canvas::setCurrentEndPoint(NodePoint *point) {
    if(mCurrentEndPoint) mCurrentEndPoint->deselect();
    if(point) point->select();
    mCurrentEndPoint = point;
}

void Canvas::selectOnlyLastPressedBone() {
    clearBonesSelection();
    if(mLastPressedBone)
        addBoneToSelection(mLastPressedBone);
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
    mCanvasTransformMatrix.reset();
    mVisibleHeight = mHeight;
    mVisibleWidth = mWidth;
    moveByRel(QPointF((mCanvasWidget->width() - mVisibleWidth)*0.5,
                      (mCanvasWidget->height() - mVisibleHeight)*0.5) );

}

void Canvas::fitCanvasToSize() {
    mCanvasTransformMatrix.reset();
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
                       mapAbsPosToRel(QPointF(0., 0.));

    mLastPressPosRel = mapAbsPosToRel(mLastPressPosRel);

    mCanvasTransformMatrix.translate(transRel.x(), transRel.y());

    mLastPressPosRel = mCanvasTransformMatrix.map(mLastPressPosRel);
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

void getMirroredCtrlPtAbsPos(bool mirror,
                             NodePoint *point,
                             QPointF *startCtrlPtPos,
                             QPointF *endCtrlPtPos) {
    if(mirror) {
        *startCtrlPtPos = point->getEndCtrlPtAbsPos();
        *endCtrlPtPos = point->getStartCtrlPtAbsPos();
    } else {
        *startCtrlPtPos = point->getStartCtrlPtAbsPos();
        *endCtrlPtPos = point->getEndCtrlPtAbsPos();
    }
}

void Canvas::connectPointsFromDifferentPaths(NodePoint *pointSrc,
                                             NodePoint *pointDest) {
    Q_UNUSED(pointSrc)
    Q_UNUSED(pointDest)
//    if(pointSrc->getParentPath() == pointDest->getParentPath()) {
//        return;
//    }
//    VectorPathAnimator *pathSrc = pointSrc->getParentPath();
//    VectorPathAnimator *pathDest = pointDest->getParentPath();
//    setCurrentEndPoint(pointDest);
//    if(pointSrc->hasNextPoint()) {
//        NodePoint *point = pointSrc;
//        bool mirror = pointDest->hasNextPoint();
//        while(point != nullptr) {
//            QPointF startCtrlPtPos;
//            QPointF endCtrlPtPos;
//            getMirroredCtrlPtAbsPos(mirror, point,
//                                    &startCtrlPtPos, &endCtrlPtPos);
//            NodePoint *newPoint = new NodePoint(pathDest);
//            newPoint->setAbsolutePos(point->getAbsolutePos());
//            newPoint->moveStartCtrlPtToAbsPos(startCtrlPtPos);
//            newPoint->moveEndCtrlPtToAbsPos(endCtrlPtPos);

//            setCurrentEndPoint(mCurrentEndPoint->addPoint(newPoint) );
//            point = point->getNextPoint();
//        }
//    } else {
//        NodePoint *point = pointSrc;
//        bool mirror = pointDest->hasPreviousPoint();
//        while(point != nullptr) {
//            QPointF startCtrlPtPos;
//            QPointF endCtrlPtPos;
//            getMirroredCtrlPtAbsPos(mirror, point,
//                                    &startCtrlPtPos, &endCtrlPtPos);

//            NodePoint *newPoint = new NodePoint(pathDest);
//            newPoint->setAbsolutePos(point->getAbsolutePos());
//            newPoint->moveStartCtrlPtToAbsPos(startCtrlPtPos);
//            newPoint->moveEndCtrlPtToAbsPos(endCtrlPtPos);

//            setCurrentEndPoint(mCurrentEndPoint->addPoint(newPoint) );
//            point = point->getPreviousPoint();
//        }
//    }
    //mCurrentBoxesGroup->removeChild(pathSrc->getParentBox());
}

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
    return anim_mCurrentAbsFrame;
}

int Canvas::getFrameCount() {
    return mMaxFrame + 1;
}

int Canvas::getMaxFrame() {
    return mMaxFrame;
}

void Canvas::startDurationRectPosTransformForAllSelected() {
    for(const auto& box : mSelectedBoxes) {
        box->startDurationRectPosTransform();
    }
}

void Canvas::finishDurationRectPosTransformForAllSelected() {
    for(const auto& box : mSelectedBoxes) {
        box->finishDurationRectPosTransform();
    }
}

void Canvas::moveDurationRectForAllSelected(const int &dFrame) {
    for(const auto& box : mSelectedBoxes) {
        box->moveDurationRect(dFrame);
    }
}

void Canvas::startMinFramePosTransformForAllSelected() {
    for(const auto& box : mSelectedBoxes) {
        box->startMinFramePosTransform();
    }
}

void Canvas::finishMinFramePosTransformForAllSelected() {
    for(const auto& box : mSelectedBoxes) {
        box->finishMinFramePosTransform();
    }
}

void Canvas::moveMinFrameForAllSelected(const int &dFrame) {
    for(const auto& box : mSelectedBoxes) {
        box->moveMinFrame(dFrame);
    }
}

void Canvas::startMaxFramePosTransformForAllSelected() {
    for(const auto& box : mSelectedBoxes) {
        box->startMaxFramePosTransform();
    }
}

void Canvas::finishMaxFramePosTransformForAllSelected() {
    for(const auto& box : mSelectedBoxes) {
        box->finishMaxFramePosTransform();
    }
}

void Canvas::moveMaxFrameForAllSelected(const int &dFrame) {
    for(const auto& box : mSelectedBoxes) {
        box->moveMaxFrame(dFrame);
    }
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

SoundComposition *Canvas::getSoundComposition() {
    return mSoundComposition.get();
}

CanvasRenderData::CanvasRenderData(BoundingBox* parentBoxT) :
    BoxesGroupRenderData(parentBoxT) {}

#include "skia/skiahelpers.h"
void CanvasRenderData::renderToImage() {
    if(fRenderedToImage) return;
    fRenderedToImage = true;

    const auto info = SkiaHelpers::getPremulBGRAInfo(
                qCeil(canvasWidth), qCeil(canvasHeight));
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    bitmap.eraseColor(fBgColor);
    SkCanvas rasterCanvas(bitmap);
    //rasterCanvas->clear(bgColor);

    drawSk(&rasterCanvas);
    rasterCanvas.flush();

    if(!fPixmapEffects.isEmpty()) {
        for(const auto& effect : fPixmapEffects) {
            effect->applyEffectsSk(bitmap, fResolution);
        }
        clearPixmapEffects();
    }

    bitmap.setImmutable();
    fRenderedImage = SkImage::MakeFromBitmap(bitmap);
    bitmap.reset();
}

void CanvasRenderData::drawSk(SkCanvas *canvas) {
    canvas->save();

    canvas->scale(qrealToSkScalar(fResolution),
                  qrealToSkScalar(fResolution));
    for(const auto &renderData : fChildrenRenderData) {
        //box->draw(p);
        renderData->drawRenderedImageForParent(canvas);
    }

    canvas->restore();
}

void CanvasRenderData::updateRelBoundingRect() {
    fRelBoundingRect = QRectF(0, 0, canvasWidth, canvasHeight);
}
