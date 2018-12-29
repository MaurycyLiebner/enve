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

Canvas::Canvas(CanvasWindow *canvasWidget,
               int canvasWidth, int canvasHeight,
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
    mBackgroundColor->prp_setUpdater(
                SPtrCreate(DisplayedFillStrokeSettingsUpdater)(this));
    mSoundComposition = qsptr<SoundComposition>::create(this);
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

Canvas::~Canvas() {}

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

    mVisibleHeight = mCanvasTransformMatrix.m22()*mHeight;
    mVisibleWidth = mCanvasTransformMatrix.m11()*mWidth;

    if(mHoveredEdge_d != nullptr) {
        mHoveredEdge_d->generatePainterPath();
    }
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
    clearLastPressedPoint();
    mCurrentBoxesGroup = group;
    connect(mCurrentBoxesGroup, &BoxesGroup::setParentAsCurrentGroup,
            this, &Canvas::setCurrentGroupParentAsCurrentGroup);
    group->setIsCurrentGroup_k(true);

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

void Canvas::renderSk(SkCanvas * const canvas,
                      GrContext* const grContext) {
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
#ifdef CPU_ONLY_RENDER
            mCurrentPreviewContainer->drawSk(canvas, nullptr, nullptr);
#else
            mCurrentPreviewContainer->drawSk(canvas, nullptr, grContext);
#endif

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
            Q_FOREACH(const qsptr<BoundingBox> &box, mContainedBoxes) {
                box->drawPixmapSk(canvas, grContext);
            }
        }
        if(drawCanvas) {
            SkScalar reversedRes = 1.f/static_cast<SkScalar>(mResolutionFraction);
            canvas->scale(reversedRes, reversedRes);
            mCurrentPreviewContainer->drawSk(canvas, nullptr, grContext);
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

stdsptr<BoundingBoxRenderData> Canvas::createRenderData() {
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
    if(renderData->fRedo) {
        scheduleUpdate(renderData->fRelFrame, Animator::USER_CHANGE);
    }
    int fId;
    int lId;
    //qDebug() << renderData->fRelFrame;
    prp_getFirstAndLastIdenticalRelFrame(&fId, &lId, renderData->fRelFrame);
    CacheContainer *cont = mCacheHandler.getRenderContainerAtRelFrame(fId);
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

qsptr<BoundingBox> Canvas::createLink() {
    return SPtrCreate(InternalLinkCanvas)(this);
}

ImageBox *Canvas::createImageBox(const QString &path) {
    qsptr<ImageBox> img = SPtrCreate(ImageBox)(path);
    mCurrentBoxesGroup->addContainedBox(img);
    return img.get();
}

#include "Boxes/imagesequencebox.h"
ImageSequenceBox* Canvas::createAnimationBoxForPaths(const QStringList &paths) {
    qsptr<ImageSequenceBox> aniBox = SPtrCreate(ImageSequenceBox)();
    aniBox->setListOfFrames(paths);
    mCurrentBoxesGroup->addContainedBox(aniBox);
    return aniBox.get();
}

#include "Boxes/videobox.h"
VideoBox* Canvas::createVideoForPath(const QString &path) {
    qsptr<VideoBox> vidBox = SPtrCreate(VideoBox)(path);
    mCurrentBoxesGroup->addContainedBox(vidBox);
    return vidBox.get();
}

#include "Boxes/linkbox.h"
ExternalLinkBox* Canvas::createLinkToFileWithPath(const QString &path) {
    qsptr<ExternalLinkBox> extLinkBox = SPtrCreate(ExternalLinkBox)();
    extLinkBox->setSrc(path);
    mCurrentBoxesGroup->addContainedBox(extLinkBox);
    return extLinkBox.get();
}

SingleSound* Canvas::createSoundForPath(const QString &path) {
    qsptr<SingleSound> singleSound = SPtrCreate(SingleSound)(path);
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
        Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
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
    target.write(reinterpret_cast<char*>(&nBoxes), sizeof(int));

    std::sort(mSelectedBoxes.begin(), mSelectedBoxes.end(), boxesZSort);
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
        QList<stdptr<MovablePoint>> selectedPts = mSelectedPoints_d;
        foreach(const qptr<BoundingBox>& box, mSelectedBoxes) {
            box->selectAllPoints(this);
        }
        foreach(const stdptr<MovablePoint>& pt, selectedPts) {
            removePointFromSelection(pt);
        }
    } else {//if(mCurrentMode == MOVE_PATH) {
        QList<qptr<BoundingBox>> boxes = mSelectedBoxes;
        selectAllBoxesFromBoxesGroup();
        foreach(const qptr<BoundingBox>& box, boxes) {
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

    Q_FOREACH(const qsptr<BoundingBox> &box, mContainedBoxes) {
        box->prp_setAbsFrame(frame);
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
        BoundingBox* box = bone->getParentBox();
        foreach(const qptr<BoundingBox>& boxT, mSelectedBoxes) {
            if(boxT == box) continue;
            boxT->setParentTransform(trans);
        }
    } else if(mSelectedBoxes.count() > 1) {
        BoundingBox* lastBox = mSelectedBoxes.last();
        BasicTransformAnimator* trans = lastBox->getTransformAnimator();
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
    foreach(const QPointer<BoundingBox>& box, mSelectedBoxes) {
        box->selectAllPoints(this);
    }
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

//    Q_FOREACH(const qsptr<BoundingBox> &box, mChildBoxes) {
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

#include "PixmapEffects/rastereffects.h"
void CanvasRenderData::renderToImage() {
    if(fRenderedToImage) return;
    fRenderedToImage = true;

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

    if(!fPixmapEffects.isEmpty()) {
        foreach(const stdsptr<PixmapEffectRenderData>& effect, fPixmapEffects) {
            effect->applyEffectsSk(bitmap, fResolution);
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

    canvas->scale(qrealToSkScalar(fResolution),
                  qrealToSkScalar(fResolution));
    Q_FOREACH(const stdsptr<BoundingBoxRenderData> &renderData,
              childrenRenderData) {
        //box->draw(p);
        renderData->drawRenderedImageForParent(canvas);
    }

    canvas->restore();
}

void CanvasRenderData::updateRelBoundingRect() {
    fRelBoundingRect = QRectF(0., 0., canvasWidth, canvasHeight);
}
