#include "canvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include "undoredo.h"
#include "mainwindow.h"
#include "updatescheduler.h"
#include "pathpivot.h"
#include "Boxes/imagebox.h"
#include "edge.h"
#include "Sound/soundcomposition.h"
#include "Boxes/textbox.h"
#include "fillstrokesettings.h"
#include "BoxesList/OptimalScrollArea/scrollwidgetvisiblepart.h"
#include "Sound/singlesound.h"
#include "global.h"
#include "pointhelpers.h"
#include "nodepoint.h"
#include "Boxes/linkbox.h"
#include "Animators/animatorupdater.h"
#include "clipboardcontainer.h"
#include "Boxes/paintbox.h"
#include "Paint/PaintLib/brush.h"

Canvas::Canvas(FillStrokeSettingsWidget *fillStrokeSettings,
               CanvasWindow *canvasWidget,
               int canvasWidth, int canvasHeight,
               const int &frameCount, const qreal &fps) :
    BoxesGroup(fillStrokeSettings) {
    mFps = fps;
    connect(this, SIGNAL(nameChanged(QString)),
            this, SLOT(emitCanvasNameChanged()));
    mCacheHandler.setParentBox(this);
    mBackgroundColor->qra_setCurrentValue(Color(75, 75, 75));
    ca_addChildAnimator(mBackgroundColor.data());
    mBackgroundColor->prp_setUpdater(
                new DisplayedFillStrokeSettingsUpdater(this));
    mSoundComposition = new SoundComposition(this);
    ca_addChildAnimator(mSoundComposition->getSoundsAnimatorContainer());

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

    mRotPivot = new PathPivot(this);
    mRotPivot->hide();

    mCurrentMode = MOVE_PATH;

    ca_removeChildAnimator(mTransformAnimator.data());

    prp_setAbsFrame(0);

    mCurrentBrush = new Brush();
    //fitCanvasToSize();
    //setCanvasMode(MOVE_PATH);
}

Canvas::~Canvas() {
    delete mRotPivot;
    delete mCurrentBrush;
}

QRectF Canvas::getRelBoundingRectAtRelFrame(const int &) {
    return QRectF(0., 0., mWidth, mHeight);
}

void Canvas::emitCanvasNameChanged() {
    emit canvasNameChanged(this, prp_mName);
}

BoundingBox *Canvas::createLink() {
    return new InternalLinkCanvas(this);
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

#include "BoxesList/boxscrollwidget.h"
void Canvas::setCurrentBoxesGroup(BoxesGroup *group) {
    mCurrentBoxesGroup->setIsCurrentGroup(false);
    clearBoxesSelection();
    clearPointsSelection();
    clearCurrentEndPoint();
    clearLastPressedPoint();
    mCurrentBoxesGroup = group;
    group->setIsCurrentGroup(true);

    //mMainWindow->getObjectSettingsList()->setMainTarget(mCurrentBoxesGroup);
    SWT_scheduleWidgetsContentUpdateWithTarget(mCurrentBoxesGroup,
                                               SWT_CurrentGroup);
}

ImageBox *Canvas::createImageBox(const QString &path) {
    ImageBox *img = new ImageBox(path);
    mCurrentBoxesGroup->addChild(img);
    return img;
}

SingleSound *Canvas::createSoundForPath(const QString &path) {
    SingleSound *singleSound = new SingleSound(path);
    getSoundComposition()->addSoundAnimator(singleSound);
    return singleSound;
}

void Canvas::drawSelectedSk(SkCanvas *canvas,
                            const CanvasMode &currentCanvasMode,
                            const SkScalar &invScale) {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->drawSelectedSk(canvas, currentCanvasMode, invScale);
    }
}

void Canvas::updateHoveredBox() {
    mHoveredBox = mCurrentBoxesGroup->getBoxAt(mCurrentMouseEventPosRel);
}

void Canvas::updateHoveredPoint() {
    mHoveredPoint = getPointAtAbsPos(mCurrentMouseEventPosRel,
                               mCurrentMode,
                               1./mCanvasTransformMatrix.m11());
}

void Canvas::updateHoveredEdge() {
    mHoveredEdge = getEdgeAt(mCurrentMouseEventPosRel);
    if(mHoveredEdge != NULL) {
        mHoveredEdge->generatePainterPath();
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
    if(mBackgroundColor->getCurrentColor().qcol.alpha() != 255) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(SkColorSetARGBInline(125, 255, 255, 255));
        SkScalar currX = viewRect.left();
        SkScalar currY = viewRect.top();
        SkScalar widthT = MIN_WIDGET_HEIGHT*0.5f*mCanvasTransformMatrix.m11();
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
        if(mCurrentPreviewContainer != NULL) {
            canvas->save();

            canvas->concat(QMatrixToSkMatrix(mCanvasTransformMatrix));
            SkScalar reversedRes = 1./mResolutionFraction;
            canvas->scale(reversedRes, reversedRes);
            mCurrentPreviewContainer->drawSk(canvas);

            canvas->restore();
        }
    } else {
        SkScalar invScale = 1./mCanvasTransformMatrix.m11();
#ifdef CPU_ONLY_RENDER
        drawTransparencyMesh(canvas, viewRect);
        canvas->concat(QMatrixToSkMatrix(mCanvasTransformMatrix));

        if(mCurrentPreviewContainer != NULL) {
            canvas->save();
            SkScalar reversedRes = 1./mResolutionFraction;
            canvas->scale(reversedRes, reversedRes);
            mCurrentPreviewContainer->drawSk(canvas);
            canvas->restore();
        }
#else
        if(!mClipToCanvasSize) {
            paint.setColor(SkColorSetARGBInline(255, 75, 75, 75));
            canvas->drawRect(QRectFToSkRect(mCanvasTransformMatrix.mapRect(
                                                getMaxBoundsRect())),
                             paint);
        }

        drawTransparencyMesh(canvas, viewRect);
        paint.setColor(mBackgroundColor->getCurrentColor().getSkColor());
        canvas->drawRect(viewRect, paint);

        canvas->concat(QMatrixToSkMatrix(mCanvasTransformMatrix));
        canvas->saveLayer(NULL, NULL);
        Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes){
            box->drawPixmapSk(canvas);
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
            mRotPivot->drawSk(canvas, invScale);
            if(mRotPivot->isScaling() || mRotPivot->isRotating()) {
                paint.setStyle(SkPaint::kStroke_Style);
                paint.setColor(SK_ColorBLACK);
                SkScalar intervals[2] = {MIN_WIDGET_HEIGHT*0.25f*invScale,
                                         MIN_WIDGET_HEIGHT*0.25f*invScale};
                paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 0));
                canvas->drawLine(QPointFToSkPoint(mRotPivot->getAbsolutePos()),
                                 QPointFToSkPoint(mLastMouseEventPosRel),
                                 paint);
                paint.setPathEffect(NULL);
            }
        }
//        pen = QPen(QColor(0, 0, 255, 125), 2., Qt::DotLine);
//        pen.setCosmetic(true);
//        p->setPen(pen);
        if(mSelecting) {
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setColor(SkColorSetARGBInline(255, 0, 0, 255));
            paint.setStrokeWidth(2.*invScale);
            SkScalar intervals[2] = {MIN_WIDGET_HEIGHT*0.25f*invScale,
                                     MIN_WIDGET_HEIGHT*0.25f*invScale};
            paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 0));
            canvas->drawRect(QRectFToSkRect(mSelectionRect), paint);
            paint.setPathEffect(NULL);
            //SkPath selectionPath;
            //selectionPath.addRect(QRectFToSkRect(mSelectionRect));
            //canvas->drawPath(selectionPath, paint);
        }

        if(mHoveredPoint != NULL) {
            mHoveredPoint->drawHovered(canvas, invScale);
        } else if(mHoveredEdge != NULL) {
            mHoveredEdge->drawHoveredSk(canvas, invScale);
        } else if(mHoveredBox != NULL) {
            if(mCurrentEdge == NULL) {
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
        if(mInputTransformationEnabled) {
            SkRect inputRect = SkRect::MakeXYWH(
                                    2*MIN_WIDGET_HEIGHT,
                                    mCanvasWidget->height() - MIN_WIDGET_HEIGHT,
                                    5*MIN_WIDGET_HEIGHT, MIN_WIDGET_HEIGHT);
            paint.setStyle(SkPaint::kFill_Style);
            paint.setColor(SkColorSetARGBInline(255, 225, 225, 225));
            canvas->drawRect(inputRect, paint);
            QString transStr;
            if(mRotPivot->isRotating()) {
                transStr = "rot: " + mInputText + "|";
            } else if(mXOnlyTransform) {
                transStr = " x: " + mInputText + "|";
            } else if(mYOnlyTransform) {
                transStr = " y: " + mInputText + "|";
            } else {
                transStr = " x, y: " + mInputText + "|";
            }
            paint.setTextSize(FONT_HEIGHT);
            SkRect bounds;
            paint.measureText(transStr.toStdString().c_str(),
                              transStr.size()*sizeof(char),
                              &bounds);
            paint.setColor(SK_ColorBLACK);
            paint.setTypeface(SkTypeface::MakeDefault());
            paint.setStyle(SkPaint::kFill_Style);

            canvas->drawString(
                   transStr.toStdString().c_str(),
                   inputRect.x() + paint.getTextSize(),
                   inputRect.y() + inputRect.height()*0.5 + bounds.height()*0.2f,
                   paint);
            //p->drawText(inputRect, Qt::AlignVCenter, transStr);
        }
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

Brush *Canvas::getCurrentBrush() {
    return mCurrentBrush;
}

void Canvas::drawInputText(QPainter *p) {
    if(mInputTransformationEnabled) {
        QRect inputRect = QRect(2*MIN_WIDGET_HEIGHT,
                                mCanvasWidget->height() - MIN_WIDGET_HEIGHT,
                                5*MIN_WIDGET_HEIGHT, MIN_WIDGET_HEIGHT);
        p->fillRect(inputRect, QColor(225, 225, 225));
        QString text;
        if(mXOnlyTransform) {
            text = " x: " + mInputText + "|";
        } else if(mYOnlyTransform) {
            text = " y: " + mInputText + "|";
        } else {
            text = " x, y: " + mInputText + "|";
        }
        p->drawText(inputRect, Qt::AlignVCenter, text);
    }
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

void Canvas::setRendering(const bool &bT) {
    mRendering = bT;
}

void Canvas::setOutputRendering(const bool &bT) {
    mRendering = bT;
    setPreviewing(bT);
}

void Canvas::setCurrentPreviewContainer(CacheContainer *cont) {
    if(mCurrentPreviewContainer.get() != NULL) {
        if(mNoCache) {
            mCurrentPreviewContainer->setBlocked(false);
            mCurrentPreviewContainer->freeThis();
        } else if(!mRendering) {
            mCurrentPreviewContainer->setBlocked(false);
        }
    }
    if(cont == NULL) {
        mCurrentPreviewContainer.reset();
        return;
    }
    mCurrentPreviewContainer = cont->ref<CacheContainer>();
    mCurrentPreviewContainer->setBlocked(true);
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
    if(mRedoUpdate) {
        scheduleUpdate();
    }
    int fId;
    int lId;
    prp_getFirstAndLastIdenticalRelFrame(&fId, &lId, renderData->relFrame);
    CacheContainer *cont =
          mCacheHandler.getRenderContainerAtRelFrame(fId);
    if(cont == NULL) {
        cont = mCacheHandler.createNewRenderContainerAtRelFrame(fId);
    }
    cont->replaceImageSk(renderData->renderedImage);
    if(mRendering || !mPreviewing) {
        setCurrentPreviewContainer(cont);
    }
}

void Canvas::prp_updateAfterChangedAbsFrameRange(const int &minFrame,
                                                 const int &maxFrame) {
    mCacheHandler.clearCacheForAbsFrameRange(minFrame, maxFrame);
    Property::prp_updateAfterChangedAbsFrameRange(minFrame, maxFrame);
    int fId;
    int lId;
    prp_getFirstAndLastIdenticalRelFrame(&fId, &lId,
                                          anim_mCurrentRelFrame);
    if(fId < minFrame &&
       fId > maxFrame) return;
    scheduleUpdate();
}

//void Canvas::updatePixmaps() {
//    renderCurrentFrameToPreview();
//}

#include <QFile>
void Canvas::renderCurrentFrameToOutput(const QString &renderDest) {
    Q_UNUSED(renderDest);
    SkData *data = mCurrentPreviewContainer->getImageSk()->
            encode(SkEncodedImageFormat::kPNG, 100);
    QFile file;
    file.setFileName(renderDest + "_" +
                     QString::number(anim_mCurrentAbsFrame));
    if(file.open(QIODevice::WriteOnly) ) {
        QByteArray array = QByteArray((const char*)data->data(), data->size());
        file.write(array);
        file.flush();
        file.close();
    }
    data->unref();

    //renderCurrentFrameToPreview();
    //clearCurrentPreviewImage();
}

void Canvas::clearCurrentPreviewImage() {

}

#include "Boxes/imagesequencebox.h"
ImageSequenceBox *Canvas::createAnimationBoxForPaths(const QStringList &paths) {
    ImageSequenceBox *aniBox = new ImageSequenceBox();
    aniBox->setListOfFrames(paths);
    mCurrentBoxesGroup->addChild(aniBox);
    return aniBox;
}

#include "Boxes/videobox.h"
VideoBox *Canvas::createVideoForPath(const QString &path) {
    VideoBox *vidBox = new VideoBox(path);
    mCurrentBoxesGroup->addChild(vidBox);
    return vidBox;
}

#include "Boxes/linkbox.h"
void Canvas::createLinkToFileWithPath(const QString &path) {
    ExternalLinkBox *extLinkBox = new ExternalLinkBox();
    extLinkBox->setSrc(path);
    mCurrentBoxesGroup->addChild(extLinkBox);
}

void Canvas::schedulePivotUpdate() {
    if(mRotPivot->isRotating() || mRotPivot->isScaling()) return;
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
    mCurrentMode = mode;

    mSelecting = false;
    mHoveredPoint = NULL;
    clearHoveredEdge();
    clearPointsSelection();
    clearCurrentEndPoint();
    clearLastPressedPoint();
    if(mCurrentMode == MOVE_PATH || mCurrentMode == MOVE_POINT) {
        schedulePivotUpdate();
    }
}

void Canvas::clearAndDisableInput() {
    mInputTransformationEnabled = false;
    mInputText = "";
}

void Canvas::updateInputValue() {
    if(mInputText.isEmpty()) {
        mInputTransformationEnabled = false;
    } else {
        //mFirstMouseMove = false;
        mInputTransformationEnabled = true;
        mInputTransformationValue = mInputText.toDouble();
    }

    updateTransformation();
}

void Canvas::grabMouseAndTrack() {
    mIsMouseGrabbing = true;
#ifndef QT_DEBUG
    mCanvasWindow->setMouseTracking(true);
#endif
    mCanvasWindow->grabMouse();
}

void Canvas::releaseMouseAndDontTrack() {
    mIsMouseGrabbing = false;
#ifndef QT_DEBUG
    mCanvasWindow->setMouseTracking(false);
#endif
    mCanvasWindow->releaseMouse();
}

bool Canvas::handleKeyPressEventWhileMouseGrabbing(QKeyEvent *event) {
    if(event->key() == Qt::Key_Escape) {
        cancelCurrentTransform();
    } else if(event->key() == Qt::Key_Return ||
              event->key() == Qt::Key_Enter) {
        handleMouseRelease();
        clearAndDisableInput();
    } else if(event->key() == Qt::Key_Minus) {
        if( ((mInputText.isEmpty()) ? false : mInputText.at(0) == '-') ) {
            mInputText.remove("-");
        } else {
            mInputText.prepend("-");
        }
        updateInputValue();
    } else if(event->key() == Qt::Key_0) {
        mInputText += "0";
        if(mInputText == "0" || mInputText == "-0") mInputText += ".";
        updateInputValue();
    } else if(event->key() == Qt::Key_1) {
        mInputText += "1";
        updateInputValue();
    } else if(event->key() == Qt::Key_2) {
        mInputText += "2";
        updateInputValue();
    } else if(event->key() == Qt::Key_3) {
        mInputText += "3";
        updateInputValue();
    } else if(event->key() == Qt::Key_4) {
        mInputText += "4";
        updateInputValue();
    } else if(event->key() == Qt::Key_5) {
        mInputText += "5";
        updateInputValue();
    } else if(event->key() == Qt::Key_6) {
        mInputText += "6";
        updateInputValue();
    } else if(event->key() == Qt::Key_7) {
        mInputText += "7";
        updateInputValue();
    } else if(event->key() == Qt::Key_8) {
        mInputText += "8";
        updateInputValue();
    } else if(event->key() == Qt::Key_9) {
        mInputText += "9";
        updateInputValue();
    } else if(event->key() == Qt::Key_Period ||
              event->key() == Qt::Key_Comma) {
        if(!mInputText.contains(".")) {
            mInputText += ".";
            updateInputValue();
        }
    } else if(event->key() == Qt::Key_Backspace) {
        mInputText.chop(1);
        if(mInputText == "0" ||
           mInputText == "-" ||
           mInputText == "-0") mInputText = "";
        updateInputValue();
    } else if(event->key() == Qt::Key_X) {
        mXOnlyTransform = !mXOnlyTransform;
        mYOnlyTransform = false;

        updateTransformation();
    } else if(event->key() == Qt::Key_Y) {
        mYOnlyTransform = !mYOnlyTransform;
        mXOnlyTransform = false;

        updateTransformation();
    } else if(event->key() == Qt::Key_N) {
        foreach(BoundingBox *box, mSelectedBoxes) {
            if(box->SWT_isPaintBox()) {
                PaintBox *paintBox = (PaintBox*)box;
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

bool boxesZSort(BoundingBox *box1,
                BoundingBox *box2) {
    return box1->getZIndex() < box2->getZIndex();
}

void Canvas::copyAction() {
    BoxesClipboardContainer *container =
            new BoxesClipboardContainer();
    mMainWindow->replaceClipboard(container);
    QBuffer target(container->getBytesArray());
    target.open(QIODevice::WriteOnly);
    int nBoxes = mSelectedBoxes.count();
    target.write((char*)&nBoxes, sizeof(int));

    qSort(mSelectedBoxes.begin(), mSelectedBoxes.end(), boxesZSort);
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->writeBoundingBox(&target);
    }
    target.close();
}

void Canvas::pasteAction() {
    BoxesClipboardContainer *container =
            (BoxesClipboardContainer*)
            mMainWindow->getClipboardContainer(CCT_BOXES);
    if(container == NULL) return;
    container->pasteTo(mCurrentBoxesGroup);
    clearBoxesSelection();
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
//        if(isShiftPressed()) {
//            removeSelectedPointsApproximateAndClearList();
//        } else {
//            removeSelectedPointsAndClearList();
//        }
    } else if(mCurrentMode == MOVE_PATH) {
        selectAllBoxesFromBoxesGroup();
    }
}

void Canvas::clearSelectionAction() {
    if(mCurrentMode == MOVE_POINT) {
//        if(isShiftPressed()) {
//            removeSelectedPointsApproximateAndClearList();
//        } else {
//            removeSelectedPointsAndClearList();
//        }
    } else if(mCurrentMode == MOVE_PATH) {
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
              event->key() == Qt::Key_R) {
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
            if(mSelectedPoints.isEmpty()) return false;
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
            if(mSelectedPoints.isEmpty()) return false;
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
               event->modifiers() & Qt::ControlModifier &&
               !isGrabbingMouse) {
        if(mCurrentMode == MOVE_PATH) {
            if(isShiftPressed()) {
               mCurrentBoxesGroup->deselectAllBoxesFromBoxesGroup();
           } else {
               mCurrentBoxesGroup->selectAllBoxesFromBoxesGroup();
           }
        } else if(mCurrentMode == MOVE_POINT) {
            if(isShiftPressed()) {
                clearPointsSelection();
            } else {
                foreach(BoundingBox *box, mSelectedBoxes) {
                    box->selectAllPoints(this);
                }
            }
        }
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
    } else {
        return false;
    }
    schedulePivotUpdate();

    callUpdateSchedulers();
    return true;
}

void Canvas::setCurrentEndPoint(NodePoint *point) {
    if(mCurrentEndPoint != NULL) {
        mCurrentEndPoint->deselect();
    }
    if(point != NULL) {
        point->select();
    }
    mCurrentEndPoint = point;
}

void Canvas::selectOnlyLastPressedBox() {
    clearBoxesSelection();
    if(mLastPressedBox == NULL) {
        return;
    }
    addBoxToSelection(mLastPressedBox);
}

void Canvas::selectOnlyLastPressedPoint() {
    clearPointsSelection();
    if(mLastPressedPoint == NULL) {
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
//    if(pointSrc->getParentPath() == pointDest->getParentPath()) {
//        return;
//    }
//    PathAnimator *pathSrc = pointSrc->getParentPath();
//    PathAnimator *pathDest = pointDest->getParentPath();
//    setCurrentEndPoint(pointDest);
//    if(pointSrc->hasNextPoint()) {
//        NodePoint *point = pointSrc;
//        bool mirror = pointDest->hasNextPoint();
//        while(point != NULL) {
//            QPointF startCtrlPpclab.pltPos;
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
//        while(point != NULL) {
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
//    mCurrentBoxesGroup->removeChild(pathSrc->getParentBox());
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
        if(rules.type == NULL) {
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

SoundComposition *Canvas::getSoundComposition() {
    return mSoundComposition;
}

void CanvasRenderData::renderToImage() {
    if(renderedToImage) return;
    renderedToImage = true;

    SkImageInfo info = SkImageInfo::Make(canvasWidth,
                                         canvasHeight,
                                         kBGRA_8888_SkColorType,
                                         kPremul_SkAlphaType,
                                         nullptr);
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    SkCanvas *rasterCanvas = new SkCanvas(bitmap);
    rasterCanvas->clear(bgColor);

    drawSk(rasterCanvas);
    rasterCanvas->flush();

    renderedImage = SkImage::MakeFromBitmap(bitmap);
    bitmap.reset();
    delete rasterCanvas;
}
