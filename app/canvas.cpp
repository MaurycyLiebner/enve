#include "canvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include "undoredo.h"
#include "GUI/mainwindow.h"
#include "pathpivot.h"
#include "Boxes/imagebox.h"
#include "edge.h"
#include "Sound/soundcomposition.h"
#include "Boxes/textbox.h"
#include "GUI/BoxesList/OptimalScrollArea/scrollwidgetvisiblepart.h"
#include "Sound/singlesound.h"
#include "global.h"
#include "pointhelpers.h"
#include "nodepoint.h"
#include "Boxes/linkbox.h"
#include "Animators/animatorupdater.h"
#include "clipboardcontainer.h"
#include "Boxes/paintbox.h"
#include "Paint//brush.h"
#include <QFile>
#include "renderinstancesettings.h"
#include "videoencoder.h"

Canvas::Canvas(CanvasWindow *canvasWidget,
               int canvasWidth, int canvasHeight,
               const int &frameCount, const qreal &fps) :
    BoxesGroup(TYPE_CANVAS) {
    setCurrentBrush(mMainWindow->getCurrentBrush());
    mUndoRedoStack = new UndoRedoStack(mMainWindow);
    mFps = fps;
    connect(this, SIGNAL(nameChanged(QString)),
            this, SLOT(emitCanvasNameChanged()));
    mBackgroundColor->qra_setCurrentValue(QColor(75, 75, 75));
    ca_addChildAnimator(mBackgroundColor);
    mBackgroundColor->prp_setUpdater(
                SPtrCreate(DisplayedFillStrokeSettingsUpdater)(this));
    mSoundComposition = QSharedPointer<SoundComposition>::create(this);
    auto soundsAnimatorContainer = mSoundComposition->getSoundsAnimatorContainer();
    ca_addChildAnimator(GetAsSPtr(soundsAnimatorContainer, Property));

    mMaxFrame = frameCount;

    mResolutionFraction = 1.;

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

    mCurrentMode = MOVE_PATH;

    ca_removeChildAnimator(mTransformAnimator);

    prp_setAbsFrame(0);

    //fitCanvasToSize();
    //setCanvasMode(MOVE_PATH);
}

Canvas::~Canvas() {
    delete mUndoRedoStack;
}

QRectF Canvas::getRelBoundingRectAtRelFrame(const int &) {
    return QRectF(0., 0., mWidth, mHeight);
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
}

#include "GUI/BoxesList/boxscrollwidget.h"
void Canvas::setCurrentBoxesGroup(BoxesGroup *group) {
    mCurrentBoxesGroup->setIsCurrentGroup(false);
    clearBoxesSelection();
    clearBonesSelection();
    clearPointsSelection();
    clearCurrentEndPoint();
    clearLastPressedPoint();
    mCurrentBoxesGroup = group;
    group->setIsCurrentGroup(true);

    //mMainWindow->getObjectSettingsList()->setMainTarget(mCurrentBoxesGroup);
    SWT_scheduleWidgetsContentUpdateWithTarget(mCurrentBoxesGroup,
                                               SWT_CurrentGroup);
}

void Canvas::drawSelectedSk(SkCanvas *canvas,
                            const CanvasMode &currentCanvasMode,
                            const SkScalar &invScale) {
    Q_FOREACH(const QPointer<BoundingBox>& box, mSelectedBoxes) {
        box->drawSelectedSk(canvas, currentCanvasMode, invScale);
    }
}
#include "Boxes/bone.h"
void Canvas::updateHoveredBox() {
    mHoveredBox = mCurrentBoxesGroup->getBoxAt(mCurrentMouseEventPosRel);
    mHoveredBone = nullptr;
    if(mHoveredBox != nullptr && mBonesSelectionEnabled) {
        if(mHoveredBox->SWT_isBonesBox()) {
            mHoveredBone = GetAsPtr(mHoveredBox, BonesBox)->getBoneAtAbsPos(
                        mCurrentMouseEventPosRel);
        }
    }
}

void Canvas::updateHoveredPoint() {
    mHoveredPoint_d = getPointAtAbsPos(mCurrentMouseEventPosRel,
                               mCurrentMode,
                               1./mCanvasTransformMatrix.m11());
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
        SkScalar widthT = MIN_WIDGET_HEIGHT*0.5f*static_cast<SkScalar>(mCanvasTransformMatrix.m11());
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

void Canvas::renderSk(SkCanvas *canvas) {
    SkRect viewRect = QRectFToSkRect(getPixBoundingRect());

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kFill_Style);
    canvas->clear(SK_ColorBLACK);

    if(isPreviewingOrRendering()) {
        drawTransparencyMesh(canvas, viewRect);
        if(mCurrentPreviewContainer != nullptr) {
            canvas->save();

            canvas->concat(QMatrixToSkMatrix(mCanvasTransformMatrix));
            SkScalar reversedRes = 1.f/static_cast<SkScalar>(mResolutionFraction);
            canvas->scale(reversedRes, reversedRes);
            mCurrentPreviewContainer->drawSk(canvas);

            canvas->restore();
        }
    } else {
        SkScalar invScale = 1.f/static_cast<SkScalar>(mCanvasTransformMatrix.m11());
#ifdef CPU_ONLY_RENDER
        drawTransparencyMesh(canvas, viewRect);
        canvas->concat(QMatrixToSkMatrix(mCanvasTransformMatrix));

        if(mCurrentPreviewContainer != nullptr) {
            canvas->save();
            SkScalar reversedRes = 1./mResolutionFraction;
            canvas->scale(reversedRes, reversedRes);
            mCurrentPreviewContainer->drawSk(canvas);
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
                mEffectsAnimators->hasEffects() &&
                mCurrentPreviewContainer != nullptr &&
                mExpiredPixmap == 0;
        drawTransparencyMesh(canvas, viewRect);
        if(!drawCanvas) {
            paint.setColor(QColorToSkColor(mBackgroundColor->getCurrentColor()));
            canvas->drawRect(viewRect, paint);
        }

        canvas->concat(QMatrixToSkMatrix(mCanvasTransformMatrix));
        canvas->saveLayer(nullptr, nullptr);
        if(!mClipToCanvasSize || !drawCanvas) {
            Q_FOREACH(const QSharedPointer<BoundingBox> &box, mContainedBoxes){
                box->drawPixmapSk(canvas);
            }
        }
        if(drawCanvas) {
            SkScalar reversedRes = 1.f/static_cast<SkScalar>(mResolutionFraction);
            canvas->scale(reversedRes, reversedRes);
            mCurrentPreviewContainer->drawSk(canvas);
        }
        canvas->restore();
#endif
//        QPen pen = QPen(Qt::black, 1.5);
//        pen.setCosmetic(true);
//        p->setPen(pen);
        mCurrentBoxesGroup->drawSelectedSk(canvas,
                                           mCurrentMode,
                                           invScale);
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
                canvas->drawLine(QPointFToSkPoint(mRotPivot->getAbsolutePos()),
                                 QPointFToSkPoint(mLastMouseEventPosRel),
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
        } else if(mHoveredBone != nullptr) {
            mHoveredBone->drawHoveredOnlyThisPathSk(canvas, invScale);
        } else if(mHoveredBox != nullptr) {
            if(mCurrentEdge == nullptr) {
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
        paint.setStrokeWidth(4.);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(SkRect::MakeWH(mCanvasWidget->width(),
                                        mCanvasWidget->height()),
                                        paint);
    }
}

void Canvas::setMaxFrame(const int &frame) {
    mMaxFrame = frame;
}

BoundingBoxRenderDataSPtr Canvas::createRenderData() {
    return SPtrCreate(CanvasRenderData)(this);
}

const Brush *Canvas::getCurrentBrush() const {
    return mCurrentBrush;
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

void Canvas::setCurrentPreviewContainer(CacheContainer *cont,
                                        const bool &frameEncoded) {
    if(mRenderingOutput && !frameEncoded) {
        int firstIdentical;
        int lastIdentical;
        prp_getFirstAndLastIdenticalRelFrame(&firstIdentical, &lastIdentical,
                                             cont->getMinRelFrame());
        if(lastIdentical > mMaxFrame) lastIdentical = mMaxFrame;
        cont->setMaxRelFrame(lastIdentical);
        VideoEncoder::addCacheContainerToEncoderStatic(cont);
        return;
    }
    setLoadingPreviewContainer(nullptr);
    if(cont == mCurrentPreviewContainer.get()) return;
    if(mCurrentPreviewContainer.get() != nullptr) {
        if(!mRenderingPreview || mRenderingOutput) {
            mCurrentPreviewContainer->setBlocked(false);
//            if(mRenderingOutput) { // !!! dont keep frames in memory when rendering output
//                mCurrentPreviewContainer->freeAndRemove();
//            }
        }
    }
    if(cont == nullptr) {
        mCurrentPreviewContainer.reset();
        return;
    }
    mCurrentPreviewContainer = GetAsSPtr(cont, CacheContainer);
    mCurrentPreviewContainer->setBlocked(true);
}

void Canvas::setLoadingPreviewContainer(CacheContainer *cont) {
    if(cont == mLoadingPreviewContainer.get()) return;
    if(mLoadingPreviewContainer.get() != nullptr) {
        mLoadingPreviewContainer->setAsCurrentPreviewContainerAfterFinishedLoading(nullptr);
        if(!mRenderingPreview || mRenderingOutput) {
            mLoadingPreviewContainer->setBlocked(false);
        }
    }
    if(cont == nullptr) {
        mLoadingPreviewContainer.reset();
        return;
    }
    mLoadingPreviewContainer = GetAsSPtr(cont, CacheContainer);
    cont->setAsCurrentPreviewContainerAfterFinishedLoading(this);
    mLoadingPreviewContainer->setBlocked(true);
}

void Canvas::playPreview(const int &minPreviewFrameId,
                         const int &maxPreviewFrameId) {
    if(minPreviewFrameId >= maxPreviewFrameId) return;
    mMaxPreviewFrameId = maxPreviewFrameId;
    mCurrentPreviewFrameId = minPreviewFrameId;
    setCurrentPreviewContainer(mCacheHandler.getRenderContainerAtRelFrame(
                                    mCurrentPreviewFrameId));
    setPreviewing(true);
    mCanvasWindow->requestUpdate();
}

#include "memorychecker.h"
int Canvas::getMaxPreviewFrame(const int &minFrame,
                               const int &maxFrame) {
    unsigned long long frameSize =
            static_cast<unsigned long long>(getByteCountPerFrame());
    unsigned long long freeRam = getFreeRam() -
            MemoryChecker::getInstance()->getLowFreeRam();
    int maxNewFrames = static_cast<int>(freeRam/frameSize);
    int maxFrameT = minFrame + maxNewFrames;
    int firstF = minFrame, lastF = minFrame, frameT = minFrame;
    maxFrameT += mCacheHandler.getNumberNotCachedBeforeRelFrame(minFrame);
    while(lastF < maxFrameT && lastF < maxFrame) {
        prp_getFirstAndLastIdenticalRelFrame(&firstF, &lastF, frameT);
//        if(frameT == minFrame) {
//            mCacheHandler.cacheDataBeforeRelFrame(firstF);
//        }
        CacheContainer *cont = mCacheHandler.getRenderContainerAtRelFrame(firstF);
        if(cont != nullptr) {
            if(cont->storesDataInMemory()) {
                maxFrameT++;
            }
            cont->setBlocked(true);
        }
        frameT = lastF + 1;
    }
    mCacheHandler.updateAllAfterFrameInMemoryHandler(firstF);
    return qMin(maxFrame, qMax(minFrame, maxFrameT));
}

void Canvas::clearPreview() {
    mMainWindow->previewFinished();
    mCanvasWindow->stopPreview();
}

void Canvas::nextPreviewFrame() {
    mCurrentPreviewFrameId++;
    if(mCurrentPreviewFrameId > mMaxPreviewFrameId) {
        clearPreview();
    } else {
        setCurrentPreviewContainer(
                mCacheHandler.getRenderContainerAtOrBeforeRelFrame(
                                    mCurrentPreviewFrameId));
    }
    mCanvasWindow->requestUpdate();
}

void Canvas::prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                                   int *lastIdentical,
                                                   const int &relFrame) {
    int fId;
    int lId;
    BoxesGroup::prp_getFirstAndLastIdenticalRelFrame(&fId, &lId, relFrame);
    *firstIdentical = qMax(fId, 0);
    *lastIdentical = qMin(lId, getMaxFrame());
}

void Canvas::renderDataFinished(BoundingBoxRenderData *renderData) {
    mExpiredPixmap = 0;
    if(renderData->redo) {
        scheduleUpdate(renderData->relFrame, Animator::USER_CHANGE);
    }
    int fId;
    int lId;
    //qDebug() << renderData->relFrame;
    prp_getFirstAndLastIdenticalRelFrame(&fId, &lId, renderData->relFrame);
    CacheContainer *cont =
          mCacheHandler.getRenderContainerAtRelFrame(fId);
    if(cont == nullptr) {
        cont = mCacheHandler.createNewRenderContainerAtRelFrame(fId);
    }
    cont->replaceImageSk(renderData->renderedImage);
    if(mRenderingPreview || mRenderingOutput || !mPreviewing) {
        mCurrentPreviewContainerOutdated = false;
        setCurrentPreviewContainer(cont);
    } else if(mPreviewing) {
        cont->setBlocked(true);
    }
    callUpdateSchedulers();
}

void Canvas::prp_updateAfterChangedAbsFrameRange(const int &minFrame,
                                                 const int &maxFrame) {
    if(anim_mCurrentAbsFrame >= minFrame && anim_mCurrentAbsFrame <= maxFrame) {
        mCurrentPreviewContainerOutdated = true;
    }
    mCacheHandler.clearCacheForRelFrameRange(
                prp_absFrameToRelFrame(minFrame),
                prp_absFrameToRelFrame(maxFrame));
    Property::prp_updateAfterChangedAbsFrameRange(minFrame, maxFrame);
    int fId;
    int lId;
    prp_getFirstAndLastIdenticalRelFrame(&fId, &lId,
                                          anim_mCurrentRelFrame);
    if(fId < minFrame && fId > maxFrame) return;
    scheduleUpdate(Animator::USER_CHANGE);
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

BoundingBoxQSPtr Canvas::createLink() {
    return SPtrCreate(InternalLinkCanvas)(this);
}

ImageBox *Canvas::createImageBox(const QString &path) {
    ImageBoxQSPtr img = SPtrCreate(ImageBox)(path);
    mCurrentBoxesGroup->addContainedBox(img);
    return img.get();
}

#include "Boxes/imagesequencebox.h"
ImageSequenceBox* Canvas::createAnimationBoxForPaths(const QStringList &paths) {
    ImageSequenceBoxQSPtr aniBox = SPtrCreate(ImageSequenceBox)();
    aniBox->setListOfFrames(paths);
    mCurrentBoxesGroup->addContainedBox(aniBox);
    return aniBox.get();
}

#include "Boxes/videobox.h"
VideoBox* Canvas::createVideoForPath(const QString &path) {
    VideoBoxQSPtr vidBox = SPtrCreate(VideoBox)(path);
    mCurrentBoxesGroup->addContainedBox(vidBox);
    return vidBox.get();
}

#include "Boxes/linkbox.h"
ExternalLinkBox* Canvas::createLinkToFileWithPath(const QString &path) {
    ExternalLinkBoxQSPtr extLinkBox = SPtrCreate(ExternalLinkBox)();
    extLinkBox->setSrc(path);
    mCurrentBoxesGroup->addContainedBox(extLinkBox);
    return extLinkBox.get();
}

SingleSound* Canvas::createSoundForPath(const QString &path) {
    SingleSoundQSPtr singleSound = SPtrCreate(SingleSound)(path);
    getSoundComposition()->addSoundAnimator(singleSound);
    return singleSound.get();
}

void Canvas::schedulePivotUpdate() {
    if(mRotPivot->isRotating() ||
        mRotPivot->isScaling() ||
        mRotPivot->isSelected()) return;
    if(mLastPressedPoint != nullptr) {
        if(mLastPressedPoint->isCtrlPoint()) {
            return;
        }
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
    if(mIsMouseGrabbing) {
        handleMouseRelease();
    }

    mCurrentMode = mode;
    mSelecting = false;
    mHoveredPoint_d = nullptr;
    clearHoveredEdge();
    clearPointsSelection();
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

bool Canvas::handleKeyPressEventWhileMouseGrabbing(QKeyEvent *event) {
    if(mValueInput.handleKeyPressEventWhileMouseGrabbing(event)) {
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
        Q_FOREACH(const BoundingBoxQPtr& box, mSelectedBoxes) {
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

bool boxesZSort(const BoundingBoxQPtr& box1,
                const BoundingBoxQPtr& box2) {
    return box1->getZIndex() < box2->getZIndex();
}

void Canvas::copyAction() {
    BoxesClipboardContainerSPtr container =
            SPtrCreate(BoxesClipboardContainer)();
    mMainWindow->replaceClipboard(container);
    QBuffer target(container->getBytesArray());
    target.open(QIODevice::WriteOnly);
    int nBoxes = mSelectedBoxes.count();
    target.write(reinterpret_cast<char*>(&nBoxes), sizeof(int));

    qSort(mSelectedBoxes.begin(), mSelectedBoxes.end(), boxesZSort);
    Q_FOREACH(const QPointer<BoundingBox>& box, mSelectedBoxes) {
        box->writeBoundingBox(&target);
    }
    target.close();
}

void Canvas::pasteAction() {
    BoxesClipboardContainer *container =
            static_cast<BoxesClipboardContainer*>(
            mMainWindow->getClipboardContainer(CCT_BOXES));
    if(container == nullptr) return;
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
        foreach(const QPointer<BoundingBox>& box, mSelectedBoxes) {
            box->selectAllPoints(this);
        }
    } else {//if(mCurrentMode == MOVE_PATH) {
        selectAllBoxesFromBoxesGroup();
    }
}

void Canvas::invertSelectionAction() {
    if(mCurrentMode == MOVE_POINT) {
        QList<MovablePointPtr> selectedPts = mSelectedPoints_d;
        foreach(const BoundingBoxQPtr& box, mSelectedBoxes) {
            box->selectAllPoints(this);
        }
        foreach(const MovablePointPtr& pt, selectedPts) {
            removePointFromSelection(pt);
        }
    } else {//if(mCurrentMode == MOVE_PATH) {
        QList<BoundingBoxQPtr> boxes = mSelectedBoxes;
        selectAllBoxesFromBoxesGroup();
        foreach(const BoundingBoxQPtr& box, boxes) {
            box->removeFromSelection();
        }
    }
}

void Canvas::prp_setAbsFrame(const int &frame) {
    int lastRelFrame = anim_mCurrentRelFrame;
    ComplexAnimator::prp_setAbsFrame(frame);
    CacheContainer *cont =
            mCacheHandler.getRenderContainerAtRelFrame(anim_mCurrentRelFrame);
    if(cont == nullptr) {
        bool difference = prp_differencesBetweenRelFrames(lastRelFrame,
                                                          anim_mCurrentRelFrame);
        mCurrentPreviewContainerOutdated = difference;
        if(difference) {
            scheduleUpdate(Animator::FRAME_CHANGE);
        }
    } else {
        if(cont->storesDataInMemory()) { // !!!
            setCurrentPreviewContainer(cont);
        } else {// !!!
            setLoadingPreviewContainer(cont);
        }// !!!
    }

    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
        box->prp_setAbsFrame(frame);
    }
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

bool Canvas::keyPressEvent(QKeyEvent *event) {
    if(isPreviewingOrRendering()) return false;

    bool isGrabbingMouse = mCanvasWindow->isMouseGrabber();
    if(isGrabbingMouse) {
        if(handleKeyPressEventWhileMouseGrabbing(event)) return true;
    }
    if(event->modifiers() & Qt::ControlModifier &&
            event->key() == Qt::Key_V) {
        if(event->isAutoRepeat()) return false;
        pasteAction();
    } else if(event->modifiers() & Qt::ControlModifier &&
              event->key() == Qt::Key_C) {
        if(event->isAutoRepeat()) return false;
        copyAction();
    } else if(event->modifiers() & Qt::ControlModifier &&
              event->key() == Qt::Key_D) {
        if(event->isAutoRepeat()) return false;
        duplicateAction();
    } else if(event->modifiers() & Qt::ControlModifier &&
              event->key() == Qt::Key_X) {
        if(event->isAutoRepeat()) return false;
        cutAction();
    } else if(event->key() == Qt::Key_0 &&
              event->modifiers() & Qt::KeypadModifier) {
        fitCanvasToSize();
    } else if(event->key() == Qt::Key_1 &&
              event->modifiers() & Qt::KeypadModifier) {
        resetTransormation();
    } else if(event->key() == Qt::Key_Delete) {
        deleteAction();
    } else if(event->modifiers() & Qt::ControlModifier &&
              event->key() == Qt::Key_P) {
        if(!mSelectedBones.isEmpty()) {
            Bone *bone = mSelectedBones.last();
            BasicTransformAnimator *trans = bone->getTransformAnimator();
            BoundingBox* box = bone->getParentBox();
            foreach(const BoundingBoxQPtr& boxT, mSelectedBoxes) {
                if(boxT == box) continue;
                boxT->setParent(trans);
            }
        } else if(mSelectedBoxes.count() > 1) {
            BoundingBox* lastBox = mSelectedBoxes.last();
            BasicTransformAnimator* trans = lastBox->getTransformAnimator();
            for(int i = 0; i < mSelectedBoxes.count() - 1; i++) {
                mSelectedBoxes.at(i)->setParent(trans);
            }
        }
    } else if(event->modifiers() & Qt::AltModifier &&
              event->key() == Qt::Key_P) {
        for(int i = 0; i < mSelectedBoxes.count(); i++) {
            mSelectedBoxes.at(i)->clearParent();
        }
    } else if(event->modifiers() & Qt::ControlModifier &&
              event->key() == Qt::Key_G) {
       if(isShiftPressed()) {
           ungroupSelectedBoxes();
       } else {
           groupSelectedBoxes();
       }
    } else if(event->key() == Qt::Key_PageUp) {
       raiseSelectedBoxes();
    } else if(event->key() == Qt::Key_PageDown) {
       lowerSelectedBoxes();
    } else if(event->key() == Qt::Key_End) {
       lowerSelectedBoxesToBottom();
    } else if(event->key() == Qt::Key_Home) {
       raiseSelectedBoxesToTop();
    } else if(event->key() == Qt::Key_G &&
              isAltPressed(event)) {
        resetSelectedTranslation();
    } else if(event->key() == Qt::Key_S &&
              isAltPressed(event)) {
        resetSelectedScale();
    } else if(event->modifiers() & Qt::ControlModifier &&
              (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)) {
       if(event->modifiers() & Qt::ShiftModifier) {
           revertAllPointsForAllKeys();
       } else {
           revertAllPoints();
       }
   } else if(event->key() == Qt::Key_R &&
              isAltPressed(event)) {
        resetSelectedRotation();
    } else if(event->key() == Qt::Key_R &&
              (isMovingPath() ||
              mCurrentMode == MOVE_POINT) &&
              !isGrabbingMouse) {
        if(mSelectedBoxes.isEmpty()) return false;
        if(mCurrentMode == MOVE_POINT) {
            if(mSelectedPoints_d.isEmpty()) return false;
        }
        mTransformationFinishedBeforeMouseRelease = false;
        QPointF cursorPos = mCanvasWindow->mapFromGlobal(QCursor::pos());
        setLastMouseEventPosAbs(cursorPos);
        setLastMousePressPosAbs(cursorPos);
        mRotPivot->startRotating();
        mDoubleClick = false;
        mFirstMouseMove = true;

        grabMouseAndTrack();
    } else if(event->key() == Qt::Key_S && (mCurrentMode == MOVE_PATH ||
              mCurrentMode == MOVE_POINT) && !isGrabbingMouse) {
        if(mSelectedBoxes.isEmpty()) return false;
        if(mCurrentMode == MOVE_POINT) {
            if(mSelectedPoints_d.isEmpty()) return false;
        }
        mTransformationFinishedBeforeMouseRelease = false;
        mXOnlyTransform = false;
        mYOnlyTransform = false;

        QPointF cursorPos = mCanvasWindow->mapFromGlobal(QCursor::pos());
        setLastMouseEventPosAbs(cursorPos);
        setLastMousePressPosAbs(cursorPos);
        mRotPivot->startScaling();
        mDoubleClick = false;
        mFirstMouseMove = true;

        grabMouseAndTrack();
    } else if(event->key() == Qt::Key_G && (isMovingPath() ||
                                            mCurrentMode == MOVE_POINT) &&
              !isGrabbingMouse) {
        mTransformationFinishedBeforeMouseRelease = false;
        mXOnlyTransform = false;
        mYOnlyTransform = false;

        QPointF cursorPos = mCanvasWindow->mapFromGlobal(QCursor::pos());
        setLastMouseEventPosAbs(cursorPos);
        setLastMousePressPosAbs(cursorPos);
        mDoubleClick = false;
        mFirstMouseMove = true;

        grabMouseAndTrack();
     } else if(event->key() == Qt::Key_A &&
               !isGrabbingMouse) {
        if(mCurrentMode == MOVE_PATH) {
            if(isAltPressed()) {
               mCurrentBoxesGroup->deselectAllBoxesFromBoxesGroup();
           } else {
               mCurrentBoxesGroup->selectAllBoxesFromBoxesGroup();
           }
        } else if(mCurrentMode == MOVE_POINT) {
            if(isAltPressed()) {
                clearPointsSelection();
            } else {
                foreach(const QPointer<BoundingBox>& box, mSelectedBoxes) {
                    box->selectAllPoints(this);
                }
            }
        }
    } else if(event->key() == Qt::Key_I && !isGrabbingMouse) {
        invertSelectionAction();
    } else if(event->key() == Qt::Key_W) {
        MainWindow::getInstance()->incBrushRadius();
    } else if(event->key() == Qt::Key_Q) {
        MainWindow::getInstance()->decBrushRadius();
    } else if(event->modifiers() & Qt::ControlModifier &&
              event->key() == Qt::Key_Right) {
        if(event->modifiers() & Qt::ShiftModifier) {
            shiftAllPointsForAllKeys(1);
        } else {
            shiftAllPoints(1);
        }
    } else if(event->modifiers() & Qt::ControlModifier &&
              event->key() == Qt::Key_Left) {
        if(event->modifiers() & Qt::ShiftModifier) {
            shiftAllPointsForAllKeys(-1);
        } else {
            shiftAllPoints(-1);
        }
    } else if(event->key() == Qt::Key_Minus ||
              event->key() == Qt::Key_Plus) {
        if(isPreviewingOrRendering()) return false;
        if(event->key() == Qt::Key_Plus) {
            zoomCanvas(1.2, mCanvasWindow->mapFromGlobal(
                           QCursor::pos()));
        } else {
            zoomCanvas(0.8, mCanvasWindow->mapFromGlobal(
                           QCursor::pos()));
        }
        mVisibleHeight = mCanvasTransformMatrix.m22()*mHeight;
        mVisibleWidth = mCanvasTransformMatrix.m11()*mWidth;

        if(mHoveredEdge_d != nullptr) {
            mHoveredEdge_d->generatePainterPath();
        }
    } else {
        return false;
    }

    callUpdateSchedulers();
    return true;
}

void Canvas::setCurrentEndPoint(NodePoint *point) {
    if(mCurrentEndPoint != nullptr) {
        mCurrentEndPoint->deselect();
    }
    if(point != nullptr) {
        point->select();
    }
    mCurrentEndPoint = point;
}

void Canvas::selectOnlyLastPressedBone() {
    clearBonesSelection();
    if(mLastPressedBone == nullptr) {
        return;
    }
    addBoneToSelection(mLastPressedBone);
}

void Canvas::selectOnlyLastPressedBox() {
    clearBoxesSelection();
    if(mLastPressedBox == nullptr) {
        return;
    }
    addBoxToSelection(mLastPressedBox);
}

void Canvas::selectOnlyLastPressedPoint() {
    clearPointsSelection();
    if(mLastPressedPoint == nullptr) {
        return;
    }
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
    mVisibleHeight = mCanvasTransformMatrix.m22()*mHeight;
    mVisibleWidth = mCanvasTransformMatrix.m11()*mWidth;
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

//    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
//        box->prp_setAbsFrame(currentFrame);
//    }

//    BoxesGroup::prp_setAbsFrame(currentFrame);
//    //mSoundComposition->getSoundsAnimatorContainer()->prp_setAbsFrame(currentFrame);
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
                                 const bool &) {
    Q_UNUSED(parentSatisfies);
    const SWT_Rule &rule = rules.rule;
    const bool &alwaysShowChildren = rules.alwaysShowChildren;
    if(alwaysShowChildren) {
        return false;
    } else {
        if(rules.type == nullptr) {
        } else if(rules.type == &SingleWidgetTarget::SWT_isSingleSound) {
            return false;
        }
        if(rule == SWT_NoRule) {
            return true;
        } else if(rule == SWT_Selected) {
            return false;
        } else if(rule == SWT_Animated) {
            return false;
        } else if(rule == SWT_NotAnimated) {
            return false;
        } else if(rule == SWT_Visible) {
            return true;
        } else if(rule == SWT_Invisible) {
            return false;
        } else if(rule == SWT_Locked) {
            return false;
        } else if(rule == SWT_Unlocked) {
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
    foreach(const QPointer<BoundingBox>& box, mSelectedBoxes) {
        box->startDurationRectPosTransform();
    }
}

void Canvas::finishDurationRectPosTransformForAllSelected() {
    foreach(const QPointer<BoundingBox>& box, mSelectedBoxes) {
        box->finishDurationRectPosTransform();
    }
}

void Canvas::moveDurationRectForAllSelected(const int &dFrame) {
    foreach(const QPointer<BoundingBox>& box, mSelectedBoxes) {
        box->moveDurationRect(dFrame);
    }
}

void Canvas::startMinFramePosTransformForAllSelected() {
    foreach(const QPointer<BoundingBox>& box, mSelectedBoxes) {
        box->startMinFramePosTransform();
    }
}

void Canvas::finishMinFramePosTransformForAllSelected() {
    foreach(const QPointer<BoundingBox>& box, mSelectedBoxes) {
        box->finishMinFramePosTransform();
    }
}

void Canvas::moveMinFrameForAllSelected(const int &dFrame) {
    foreach(const QPointer<BoundingBox>& box, mSelectedBoxes) {
        box->moveMinFrame(dFrame);
    }
}

void Canvas::startMaxFramePosTransformForAllSelected() {
    foreach(const QPointer<BoundingBox>& box, mSelectedBoxes) {
        box->startMaxFramePosTransform();
    }
}

void Canvas::finishMaxFramePosTransformForAllSelected() {
    foreach(const QPointer<BoundingBox>& box, mSelectedBoxes) {
        box->finishMaxFramePosTransform();
    }
}

void Canvas::moveMaxFrameForAllSelected(const int &dFrame) {
    foreach(const QPointer<BoundingBox>& box, mSelectedBoxes) {
        box->moveMaxFrame(dFrame);
    }
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

CanvasRenderData::CanvasRenderData(BoundingBox* parentBoxT) :
    BoxesGroupRenderData(parentBoxT) {}

#include "PixmapEffects/fmt_filters.h"
void CanvasRenderData::renderToImage() {
    if(renderedToImage) return;
    renderedToImage = true;

    SkImageInfo info = SkImageInfo::Make(qCeil(canvasWidth),
                                         qCeil(canvasHeight),
                                         kBGRA_8888_SkColorType,
                                         kPremul_SkAlphaType,
                                         nullptr);
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    bitmap.eraseColor(bgColor);
    SkCanvas *rasterCanvas = new SkCanvas(bitmap);
    //rasterCanvas->clear(bgColor);

    drawSk(rasterCanvas);
    rasterCanvas->flush();

    if(!pixmapEffects.isEmpty()) {
        SkPixmap pixmap;
        bitmap.peekPixels(&pixmap);
        fmt_filters::image img(static_cast<uint8_t*>(pixmap.writable_addr()),
                               pixmap.width(), pixmap.height());
        foreach(const PixmapEffectRenderDataSPtr& effect, pixmapEffects) {
            effect->applyEffectsSk(bitmap, img, resolution);
        }
        clearPixmapEffects();
    }

    bitmap.setImmutable();
    renderedImage = SkImage::MakeFromBitmap(bitmap);
    bitmap.reset();
    delete rasterCanvas;
}

void CanvasRenderData::drawSk(SkCanvas *canvas) {
    canvas->save();

    canvas->scale(qrealToSkScalar(resolution),
                  qrealToSkScalar(resolution));
    Q_FOREACH(const BoundingBoxRenderDataSPtr &renderData,
              childrenRenderData) {
        //box->draw(p);
        renderData->drawRenderedImageForParent(canvas);
    }

    canvas->restore();
}

void CanvasRenderData::updateRelBoundingRect() {
    relBoundingRect = QRectF(0., 0., canvasWidth, canvasHeight);
}
