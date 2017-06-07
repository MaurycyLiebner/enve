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
#include "pathpoint.h"
#include "Boxes/linkbox.h"
#include "Animators/animatorupdater.h"

Canvas::Canvas(FillStrokeSettingsWidget *fillStrokeSettings,
               CanvasWindow *canvasWidget,
               int canvasWidth, int canvasHeight,
               const int &frameCount) :
    BoxesGroup(fillStrokeSettings) {
    mCacheHandler.setParentBox(this);
    mBackgroundColor->qra_setCurrentValue(Color(75, 75, 75));
    ca_addChildAnimator(mBackgroundColor.data());
    mBackgroundColor->prp_setUpdater(
                new DisplayedFillStrokeSettingsUpdater(this));
    mSoundComposition = new SoundComposition(this);
    ca_addChildAnimator(mSoundComposition->getSoundsAnimatorContainer());

    mMaxFrame = frameCount;

    mEffectsPaintEnabled = true;
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

    prp_setAbsFrame(0);
    scheduleHardUpdate();
    //fitCanvasToSize();
    //setCanvasMode(MOVE_PATH);
}

Canvas::~Canvas() {
    delete mRotPivot;
}

void Canvas::showContextMenu(QPointF globalPos) {
//    QMenu menu(mMainWindow);

//    QAction *outlineScaled = new QAction("Scale outline");
//    outlineScaled->setCheckable(true);
//    outlineScaled->setChecked(mOutlineAffectedByScale);
//    menu.addAction(outlineScaled);

//    QAction  *infAction = new QAction("Points influence");
//    infAction->setCheckable(true);
//    infAction->setChecked(mInfluenceEnabled);

//    menu.addAction(infAction);

//    menu.addAction("Delete");
//    QAction *selected_action = menu.exec(globalPos);
//    if(selected_action != NULL)
//    {
//        if(selected_action->text() == "Delete")
//        {

//        } else if(selected_action == infAction) {
//            if(mInfluenceEnabled) {
//                disableInfluence();
//            } else {
//                enableInfluence();
//            }
//        } else if(selected_action == outlineScaled) {
//            setOutlineAffectedByScale(!mOutlineAffectedByScale);
//        }
//    } else {

//    }
}

BoundingBox *Canvas::createLink(BoxesGroup *parent) {
    InternalLinkCanvas *linkGroup = new InternalLinkCanvas(this,
                                                           parent);
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
        box->createSameTransformationLink(linkGroup);
    }
    return linkGroup;
}

void Canvas::setEffectsPaintEnabled(const bool &bT) {
    mEffectsPaintEnabled = bT;
}

bool Canvas::effectsPaintEnabled() {
    return mEffectsPaintEnabled;
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

bool Canvas::processUnfilteredKeyEvent(QKeyEvent *event) {
    Q_UNUSED(event);
    return false;
}

bool Canvas::processFilteredKeyEvent(QKeyEvent *event) {
    if(processUnfilteredKeyEvent(event)) return true;
    if(!mCanvasWidget->hasFocus()) return false;
    if(isCtrlPressed() && event->key() == Qt::Key_G) {
        if(isShiftPressed()) {
            ungroupSelected();
        } else {
            /*BoxesGroup *newGroup = */groupSelectedBoxes();
//            if(newGroup != NULL) {
//                setCurrentBoxesGroup(newGroup);
//            }
        }
        schedulePivotUpdate();


    } else if(event->key() == Qt::Key_PageUp) {
        raiseSelectedBoxes();
    } else if(event->key() == Qt::Key_PageDown) {
        lowerSelectedBoxes();
    } else if(event->key() == Qt::Key_End) {
        lowerSelectedBoxesToBottom();
    } else if(event->key() == Qt::Key_Home) {
        raiseSelectedBoxesToTop();
    } else {
        return false;
    }
    return true;
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

int Canvas::prp_saveToSql(QSqlQuery *query, const int &parentId) {
    Q_UNUSED(parentId);
    int boundingBoxId = BoxesGroup::prp_saveToSql(query, 0);
    query->exec(QString("INSERT INTO canvas "
                        "(boundingboxid, width, height, framecount) VALUES "
                        "(%1, %2, %3, %4)").
                arg(boundingBoxId).
                arg(mWidth).
                arg(mHeight).
                arg(mMaxFrame));
    return boundingBoxId;
}

void Canvas::createImageBox(const QString &path) {
    new ImageBox(mCurrentBoxesGroup, path);
}

void Canvas::createSoundForPath(const QString &path) {
    getSoundComposition()->addSoundAnimator(new SingleSound(path));
}

void Canvas::drawSelectedSk(SkCanvas *canvas,
                          const CanvasMode &currentCanvasMode,
                          const qreal &invScale) {
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

void Canvas::renderSk(SkCanvas *canvas) {
    SkRect viewRect = QRectFToSkRect(getPixBoundingRect());

    SkPaint paint;
    paint.setAntiAlias(true);
    if(isPreviewingOrRendering()) {
        paint.setColor(SK_ColorBLACK);
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawRect(SkRect::MakeWH(mCanvasWidget->width() + 1,
                                        mCanvasWidget->height() + 1),
                         paint);

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
        canvas->clear(SK_ColorBLACK);

        canvas->concat(QMatrixToSkMatrix(mCanvasTransformMatrix));

        if(mCurrentPreviewContainer != NULL) {
            canvas->save();
            SkScalar reversedRes = 1./mResolutionFraction;
            canvas->scale(reversedRes, reversedRes);
            mCurrentPreviewContainer->drawSk(canvas);
            canvas->restore();
        }
#else
        paint.setColor(SkColorSetARGBInline(255, 75, 75, 75));
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawRect(SkRect::MakeWH(mCanvasWindow->width() + 1,
                                        mCanvasWidget->height() + 1),
                         paint);
        paint.setColor(mBackgroundColor->getCurrentColor().getSkColor());
        canvas->drawRect(viewRect, paint);

        canvas->concat(QMatrixToQMatrix(mCanvasTransformMatrix));
        Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes){
            box->drawPixmapSk(canvas);
        }
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

        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(2.);
        paint.setColor(SK_ColorBLACK);
        canvas->drawRect(viewRect.makeInset(1, 1),
                         paint);
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
            paint.setStyle(SkPaint::kStroke_Style);

            canvas->drawString(
                   transStr.toStdString().c_str(),
                   inputRect.x() + paint.getTextSize(),
                   inputRect.y() + 0.5*(inputRect.height() + FONT_HEIGHT),
                   paint);
            //p->drawText(inputRect, Qt::AlignVCenter, transStr);
        }
    }

    if(mCanvasWindow->hasFocus()) {
        paint.setColor(SK_ColorRED);
        paint.setStrokeWidth(4.);
        canvas->drawRect(SkRect::MakeWH(mCanvasWidget->width(),
                                        mCanvasWidget->height()),
                                        paint);
    }
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

void Canvas::scheduleUpdate() {
    if(shouldUpdate()) {
        addUpdateScheduler(new AddBoxAwaitingUpdateScheduler(this));
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
    if(mCurrentPreviewContainer != NULL) {
        mCurrentPreviewContainer->decNumberPointers();
        if(!mRendering) {
            mCurrentPreviewContainer->setBlocked(false);
        }
    }
    mCurrentPreviewContainer = cont;
    if(mCurrentPreviewContainer != NULL) {
        mCurrentPreviewContainer->setBlocked(true);
        mCurrentPreviewContainer->incNumberPointers();
    }
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

void Canvas::beforeUpdate() {
    BoxesGroup::beforeUpdate();
    mRenderBackgroundColor = mBackgroundColor->getCurrentColor();
    mRenderImageSize = QSize(mWidth*mResolutionFraction,
                             mHeight*mResolutionFraction);
    CacheContainer *cont =
          mCacheHandler.getRenderContainerAtRelFrame(mUpdateRelFrame);
    mUpdateReplaceCache = cont == NULL;
    if(cont != NULL) {
        setCurrentPreviewContainer(cont);
    }
}

void Canvas::prp_updateAfterChangedAbsFrameRange(const int &minFrame,
                                                 const int &maxFrame) {
    mCacheHandler.clearCacheForAbsFrameRange(minFrame, maxFrame);
    Property::prp_updateAfterChangedAbsFrameRange(minFrame, maxFrame);

}

void Canvas::afterUpdate() {
    BoxesGroup::afterUpdate();
    if(mUpdateReplaceCache) {
        CacheContainer *cont =
              mCacheHandler.createNewRenderContainerAtRelFrame(mUpdateRelFrame);
        cont->replaceImageSk(mRenderImageSk);
        setCurrentPreviewContainer(cont);
        if(mRendering) {
            //mRenderImage.save(renderDest + QString::number(mUpdateRelFrame) + ".png");
        }
    }
    callUpdateSchedulers();
}

void Canvas::updatePixmaps() {
    renderCurrentFrameToPreview();
}

void Canvas::renderCurrentFrameToPreview() {
    SkImageInfo info = SkImageInfo::Make(mRenderImageSize.width(),
                                         mRenderImageSize.height(),
                                         kBGRA_8888_SkColorType,
                                         kPremul_SkAlphaType,
                                         nullptr);
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    SkCanvas *rasterCanvas = new SkCanvas(bitmap);
    rasterCanvas->clear(mRenderBackgroundColor.getSkColor());

    renderCurrentFrameToSkCanvasSk(rasterCanvas);
    rasterCanvas->flush();

    mRenderImageSk = SkImage::MakeFromBitmap(bitmap);
    bitmap.reset();
    delete rasterCanvas;
}

void Canvas::renderCurrentFrameToOutput(const QString &renderDest) {
    Q_UNUSED(renderDest);
    renderCurrentFrameToPreview();
    //clearCurrentPreviewImage();
}

void Canvas::clearCurrentPreviewImage() {

}

void Canvas::drawPreviewPixmapSk(SkCanvas *canvas) {
    if(isVisibleAndInVisibleDurationRect()) {
        Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes){
            box->drawUpdatePixmapSk(canvas);
        }
    }
}

#include "Boxes/imagesequencebox.h"
void Canvas::createAnimationBoxForPaths(const QStringList &paths) {
    ImageSequenceBox *aniBox = new ImageSequenceBox(mCurrentBoxesGroup);
    aniBox->setListOfFrames(paths);
}

#include "Boxes/videobox.h"
void Canvas::createVideoForPath(const QString &path) {
    /*VideoBox *vidBox = */new VideoBox(path,
                                    mCurrentBoxesGroup);
}

#include "Boxes/linkbox.h"
void Canvas::createLinkToFileWithPath(const QString &path) {
    ExternalLinkBox *extLinkBox = new ExternalLinkBox(mCurrentBoxesGroup);
    extLinkBox->setSrc(path);
}

void Canvas::renderCurrentFrameToSkCanvasSk(SkCanvas *canvas) {
    canvas->scale(mResolutionFraction, mResolutionFraction);

    drawPreviewPixmapSk(canvas);

    canvas->flush();
}

QMatrix Canvas::getCombinedRenderTransform() {
    QMatrix matrix;
    matrix.scale(mCanvasTransformMatrix.m11(),
                 mCanvasTransformMatrix.m22() );
    return matrix;
}

QMatrix Canvas::getCombinedFinalRenderTransform() {
    return QMatrix();
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

void Canvas::moveSecondSelectionPoint(QPointF pos) {
    mSelectionRect.setBottomRight(pos);
}

void Canvas::startSelectionAtPoint(QPointF pos) {
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
        if(getPointsSelectionCount() == 1) {
            mRotPivot->setAbsolutePos(
                        getSelectedPointsAbsPivotPos() +
                            QPointF(0., 20.),
                        false);
        } else {
            mRotPivot->setAbsolutePos(getSelectedPointsAbsPivotPos(),
                                      false);
        }
    } else if(mCurrentMode == MOVE_PATH) {
        if(isSelectionEmpty() ||
           mLocalPivot) {
            mRotPivot->hide();
        } else {
            mRotPivot->show();

        }
        mRotPivot->setAbsolutePos(getSelectedBoxesAbsPivotPos(),
                                  false);
    }
}

void Canvas::setCanvasMode(CanvasMode mode) {
    mCurrentMode = mode;

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
    //mCanvasWindow->setMouseTracking(true);
    mCanvasWindow->grabMouse();
}

void Canvas::releaseMouseAndDontTrack() {
    mIsMouseGrabbing = false;
    //mCanvasWindow->setMouseTracking(false);
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
    } else {
        return false;
    }

    return true;
}
#include "clipboardcontainer.h"
void Canvas::keyPressEvent(QKeyEvent *event) {
    if(isPreviewingOrRendering()) return;

    bool isGrabbingMouse = mCanvasWindow->isMouseGrabber();
    if(isGrabbingMouse ? !handleKeyPressEventWhileMouseGrabbing(event) : true) {
        if(isCtrlPressed() && event->key() == Qt::Key_V) {
            if(event->isAutoRepeat()) return;
            BoxesClipboardContainer *container =
                    (BoxesClipboardContainer*)
                    mMainWindow->getClipboardContainer(CCT_BOXES);
            if(container == NULL) return;
            container->pasteTo(mCurrentBoxesGroup);
        } else if(isCtrlPressed() && event->key() == Qt::Key_C) {
            if(event->isAutoRepeat()) return;
            BoxesClipboardContainer *container =
                    new BoxesClipboardContainer();
            Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
                container->copyBoxToContainer(box);
            }
            mMainWindow->replaceClipboard(container);
        } else if(event->key() == Qt::Key_0) {
            fitCanvasToSize();
        } else if(event->key() == Qt::Key_1) {
            resetTransormation();
        } else if(event->key() == Qt::Key_Delete) {
           if(mCurrentMode == MOVE_POINT) {
               if(isShiftPressed()) {
                   removeSelectedPointsApproximateAndClearList();
               } else {
                   removeSelectedPointsAndClearList();
               }
           } else if(mCurrentMode == MOVE_PATH) {
               removeSelectedBoxesAndClearList();
           }
        } else if(isCtrlPressed() && event->key() == Qt::Key_G) {
           if(isShiftPressed()) {
               ungroupSelected();
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
        } else if(event->key() == Qt::Key_G && isAltPressed(event)) {
            resetSelectedTranslation();
        } else if(event->key() == Qt::Key_S && isAltPressed(event)) {
            resetSelectedScale();
        } else if(event->key() == Qt::Key_R && isAltPressed(event)) {
            resetSelectedRotation();
        } else if(event->key() == Qt::Key_R && (isMovingPath() ||
                  mCurrentMode == MOVE_POINT) && !isGrabbingMouse) {
           mTransformationFinishedBeforeMouseRelease = false;
           QPointF cursorPos = mCanvasWindow->mapFromGlobal(QCursor::pos());
           setLastMouseEventPosAbs(cursorPos);
           setLastMousePressPosAbs(cursorPos);
           mRotPivot->startRotating();
           mDoubleClick = false;
           mFirstMouseMove = true;

           grabMouseAndTrack();
        } else if(event->key() == Qt::Key_S && (isMovingPath() ||
                  mCurrentMode == MOVE_POINT) && !isGrabbingMouse) {
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
                   isCtrlPressed(event) &&
                   !isGrabbingMouse) {
           if(isShiftPressed()) {
               mCurrentBoxesGroup->deselectAllBoxesFromBoxesGroup();
           } else {
               mCurrentBoxesGroup->selectAllBoxesFromBoxesGroup();
           }
        }
        schedulePivotUpdate();
    }

    callUpdateSchedulers();
}

void Canvas::setCurrentEndPoint(PathPoint *point) {
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

void Canvas::updateAfterFrameChanged(const int &currentFrame) {
    anim_mCurrentAbsFrame = currentFrame;
    Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
        box->updateAfterFrameChanged(currentFrame);
    }
    prp_setAbsFrame(currentFrame);
    //mSoundComposition->getSoundsAnimatorContainer()->prp_setAbsFrame(currentFrame);
}

void getMirroredCtrlPtAbsPos(bool mirror, PathPoint *point,
                             QPointF *startCtrlPtPos, QPointF *endCtrlPtPos) {
    if(mirror) {
        *startCtrlPtPos = point->getEndCtrlPtAbsPos();
        *endCtrlPtPos = point->getStartCtrlPtAbsPos();
    } else {
        *startCtrlPtPos = point->getStartCtrlPtAbsPos();
        *endCtrlPtPos = point->getEndCtrlPtAbsPos();
    }
}

void Canvas::connectPointsFromDifferentPaths(PathPoint *pointSrc,
                                             PathPoint *pointDest) {
//    if(pointSrc->getParentPath() == pointDest->getParentPath()) {
//        return;
//    }
//    PathAnimator *pathSrc = pointSrc->getParentPath();
//    PathAnimator *pathDest = pointDest->getParentPath();
//    setCurrentEndPoint(pointDest);
//    if(pointSrc->hasNextPoint()) {
//        PathPoint *point = pointSrc;
//        bool mirror = pointDest->hasNextPoint();
//        while(point != NULL) {
//            QPointF startCtrlPpclab.pltPos;
//            QPointF endCtrlPtPos;
//            getMirroredCtrlPtAbsPos(mirror, point,
//                                    &startCtrlPtPos, &endCtrlPtPos);
//            PathPoint *newPoint = new PathPoint(pathDest);
//            newPoint->setAbsolutePos(point->getAbsolutePos());
//            newPoint->moveStartCtrlPtToAbsPos(startCtrlPtPos);
//            newPoint->moveEndCtrlPtToAbsPos(endCtrlPtPos);

//            setCurrentEndPoint(mCurrentEndPoint->addPoint(newPoint) );
//            point = point->getNextPoint();
//        }
//    } else {
//        PathPoint *point = pointSrc;
//        bool mirror = pointDest->hasPreviousPoint();
//        while(point != NULL) {
//            QPointF startCtrlPtPos;
//            QPointF endCtrlPtPos;
//            getMirroredCtrlPtAbsPos(mirror, point,
//                                    &startCtrlPtPos, &endCtrlPtPos);

//            PathPoint *newPoint = new PathPoint(pathDest);
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

void Canvas::addChildAwaitingUpdate(BoundingBox *child) {
    BoxesGroup::addChildAwaitingUpdate(child);
    if(mAwaitingUpdate) return;
    mAwaitingUpdate = true;
    scheduleUpdate();
    //mCanvasWindow->addBoxAwaitingUpdate(this);
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
