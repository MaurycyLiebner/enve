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

Canvas::Canvas(FillStrokeSettingsWidget *fillStrokeSettings,
               CanvasWidget *canvasWidget,
               int canvasWidth, int canvasHeight,
               const int &frameCount) :
    BoxesGroup(fillStrokeSettings) {
    mBackgroundColor->qra_setCurrentValue(Color(75, 75, 75));
    ca_addChildAnimator(mBackgroundColor.data());
    mSoundComposition = new SoundComposition(this);
    ca_addChildAnimator(mSoundComposition->getSoundsAnimatorContainer());

    mFrameCount = frameCount;

    mEffectsPaintEnabled = true;
    mResolutionPercent = 1.;

    mWidth = canvasWidth;
    mHeight = canvasHeight;
    mVisibleWidth = mWidth;
    mVisibleHeight = mHeight;
    mCanvasWidget = canvasWidget;

    mCurrentBoxesGroup = this;
    mIsCurrentGroup = true;

    mRotPivot = new PathPivot(this);
    mRotPivot->hide();

    mCurrentMode = MOVE_PATH;

    //fitCanvasToSize();
    //setCanvasMode(MOVE_PATH);
}

Canvas::~Canvas() {
    delete mRotPivot;
}

void Canvas::showContextMenu(QPoint globalPos) {
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

#include "Boxes/linkbox.h"
BoundingBox *Canvas::createLink(BoxesGroup *parent) {
    InternalLinkCanvas *linkGroup = new InternalLinkCanvas(this,
                                                           parent);
    foreach(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
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

qreal Canvas::getResolutionPercent() {
    return Canvas::mResolutionPercent;
}

void Canvas::setResolutionPercent(const qreal &percent) {
    mResolutionPercent = percent;
}

QRectF Canvas::getPixBoundingRect() {
    QPointF absPos = QPointF(mCanvasTransformMatrix.dx(),
                             mCanvasTransformMatrix.dy());
    return QRectF(absPos, QSizeF(mVisibleWidth, mVisibleHeight));
}

void Canvas::zoomCanvas(const qreal &scaleBy, const QPointF &absOrigin) {
    QPointF transPoint = -mCanvasTransformMatrix.inverted().map(absOrigin);

    mCanvasTransformMatrix.translate(-transPoint.x(), -transPoint.y());
    mCanvasTransformMatrix.scale(scaleBy, scaleBy);
    mCanvasTransformMatrix.translate(transPoint.x(), transPoint.y());

    mLastPressPosAbs = mCanvasTransformMatrix.map(mLastPressPosRel);
}

bool Canvas::processUnfilteredKeyEvent(QKeyEvent *event) {
    return false;
}

bool Canvas::processFilteredKeyEvent(QKeyEvent *event) {
    if(processUnfilteredKeyEvent(event)) return true;
    if(!mCanvasWidget->hasFocus()) return false;
    if(isCtrlPressed() && event->key() == Qt::Key_G) {
        if(isShiftPressed()) {
            ungroupSelected();
        } else {
            BoxesGroup *newGroup = groupSelectedBoxes();
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
                arg(mFrameCount));
    return boundingBoxId;
}

void Canvas::createImageBox(const QString &path) {
    mCurrentBoxesGroup->addChild(new ImageBox(mCurrentBoxesGroup, path));
}

void Canvas::createSoundForPath(const QString &path) {
    getSoundComposition()->addSoundAnimator(new SingleSound(path));
}

void Canvas::drawSelected(QPainter *p, const CanvasMode &currentCanvasMode) {
    QPen pen = QPen(Qt::black, 1.);
    pen.setCosmetic(true);
    p->setPen(pen);
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->drawSelected(p, currentCanvasMode);
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

void Canvas::paintEvent(QPainter *p) {
    p->setRenderHint(QPainter::Antialiasing);
    p->setRenderHint(QPainter::SmoothPixmapTransform);

    QRectF viewRect = getPixBoundingRect();

    if(mPreviewing) {
        QPainterPath path;
        path.addRect(0, 0, mCanvasWidget->width() + 1, mCanvasWidget->height() + 1);
        QPainterPath viewRectPath;
        viewRectPath.addRect(viewRect);
        p->setBrush(QColor(0, 0, 0));
        p->setPen(Qt::NoPen);
        p->drawPath(path.subtracted(viewRectPath));

        p->save();
        qreal reversedRes = 1./mResolutionPercent;
        p->translate(getRenderRect().topLeft());
        p->scale(reversedRes, reversedRes);
        if(mCurrentPreviewImg != NULL) {
            p->drawImage(QPointF(0., 0.), *mCurrentPreviewImg);
        }
        p->restore();
    } else {
        p->fillRect(0, 0,
                    mCanvasWidget->width() + 1,
                    mCanvasWidget->height() + 1,
                    QColor(75, 75, 75));
        p->fillRect(viewRect, mBackgroundColor->getCurrentColor().qcol);

        p->setTransform(QTransform(mCanvasTransformMatrix), true);
        foreach(const QSharedPointer<BoundingBox> &box, mChildBoxes){
            box->drawPixmap(p);
        }
        QPen pen = QPen(Qt::black, 1.5);
        pen.setCosmetic(true);
        p->setPen(pen);
        mCurrentBoxesGroup->drawSelected(p, mCurrentMode);
        drawSelected(p, mCurrentMode);

        if(mCurrentMode == CanvasMode::MOVE_PATH ||
           mCurrentMode == CanvasMode::MOVE_POINT) {
            mRotPivot->draw(p);
        }
        pen = QPen(QColor(0, 0, 255, 125), 2., Qt::DotLine);
        pen.setCosmetic(true);
        p->setPen(pen);
        if(mSelecting) {
            p->drawRect(mSelectionRect);
        } 

        if(mHoveredPoint != NULL) {
            mHoveredPoint->drawHovered(p);
        } else if(mHoveredEdge != NULL) {
            mHoveredEdge->drawHover(p);
        } else if(mHoveredBox != NULL) {
            if(mCurrentEdge == NULL) {
                mHoveredBox->drawHovered(p);
            }
        }

        p->resetTransform();

        if(mInputTransformationEnabled) {
            QRect inputRect = QRect(40, mCanvasWidget->height() - 20, 100, 20);
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
        p->setPen(QPen(Qt::black, 2.));
        p->setBrush(Qt::NoBrush);
        p->drawRect(viewRect.adjusted(-1., -1., 1., 1.));
    }

    if(mCanvasWidget->hasFocus() ) {
        p->setPen(QPen(Qt::red, 4.));
    } else {
        p->setPen(Qt::NoPen);
    }
    p->setBrush(Qt::NoBrush);
    p->drawRect(mCanvasWidget->rect());
}

bool Canvas::isMovingPath() {
    return mCurrentMode == CanvasMode::MOVE_PATH;
}

QSize Canvas::getCanvasSize()
{
    return QSize(mWidth, mHeight);
}

void Canvas::setPreviewing(bool bT) {
    mPreviewing = bT;
}

void Canvas::playPreview()
{
    if(mPreviewFrames.isEmpty() ) return;
    mCurrentPreviewFrameId = 0;
    mCurrentPreviewImg = mPreviewFrames.first();
    setPreviewing(true);
    mCanvasWidget->repaint();
}

void Canvas::clearPreview() {
    setPreviewing(false);
    mCurrentPreviewImg = NULL;
    for(int i = 0; i < mPreviewFrames.length(); i++) {
        delete mPreviewFrames.at(i);
    }
    mPreviewFrames.clear();
    mMainWindow->previewFinished();
    mCanvasWidget->stopPreview();
}

void Canvas::nextPreviewFrame() {
    mCurrentPreviewFrameId++;
    if(mCurrentPreviewFrameId >= mPreviewFrames.length() ) {
        clearPreview();
    } else {
        mCurrentPreviewImg = mPreviewFrames.at(mCurrentPreviewFrameId);
    }
    mCanvasWidget->repaint();
}

QRectF Canvas::getRenderRect() {
    return mRenderRect;
    QRectF rectT = mRenderRect;
    rectT.setSize(mRenderRect.size()*mResolutionPercent);
    rectT.moveTo(rectT.topLeft()*mResolutionPercent);
    return rectT;
}

void Canvas::updateRenderRect() {
    mCanvasRect = QRectF(qMax(mCanvasTransformMatrix.dx(),
                              mCanvasTransformMatrix.dx()*
                              mResolutionPercent),
                         qMax(mCanvasTransformMatrix.dy(),
                              mCanvasTransformMatrix.dy()*
                              mResolutionPercent),
                         mVisibleWidth*mResolutionPercent,
                         mVisibleHeight*mResolutionPercent);
    QRectF canvasWidgetRect = QRectF(0., 0.,
                                     (qreal)mCanvasWidget->width(),
                                     (qreal)mCanvasWidget->height());
    mRenderRect = canvasWidgetRect.intersected(mCanvasRect);
}

void Canvas::renderCurrentFrameToPreview() {
    QImage *image = new QImage(mRenderRect.size().toSize(),
                               QImage::Format_ARGB32_Premultiplied);
    image->fill(mBackgroundColor->getCurrentColor().qcol);
    renderCurrentFrameToQImage(image);
    mPreviewFrames << image;
    mCurrentPreviewImg = image;
}

void Canvas::renderCurrentFrameToOutput(QString renderDest) {
    QImage *image = new QImage(mWidth, mHeight,
                               QImage::Format_ARGB32_Premultiplied);
    image->fill(Qt::transparent);
    renderFinalCurrentFrameToQImage(image);
    image->save(renderDest + QString::number(anim_mCurrentAbsFrame) + ".png");
    delete image;
}

void Canvas::drawPreviewPixmap(QPainter *p) {
    if(isVisibleAndInVisibleDurationRect()) {
        p->save();
        foreach(const QSharedPointer<BoundingBox> &box, mChildBoxes){
            box->drawPreviewPixmap(p);
        }

        p->restore();
    }
}

void Canvas::renderFinal(QPainter *p) {
    if(isVisibleAndInVisibleDurationRect()) {
        p->save();

        foreach(const QSharedPointer<BoundingBox> &box, mChildBoxes){
            box->renderFinal(p);
        }

        p->restore();
    }
}
#include "Boxes/imagesequencebox.h"
void Canvas::createAnimationBoxForPaths(const QStringList &paths) {
    ImageSequenceBox *aniBox = new ImageSequenceBox(mCurrentBoxesGroup);
    aniBox->setListOfFrames(paths);
}

#include "Boxes/videobox.h"
void Canvas::createVideoForPath(const QString &path) {
    VideoBox *vidBox = new VideoBox(path,
                                    mCurrentBoxesGroup);
}

#include "Boxes/linkbox.h"
void Canvas::createLinkToFileWithPath(const QString &path) {
    ExternalLinkBox *extLinkBox = new ExternalLinkBox(mCurrentBoxesGroup);
    extLinkBox->setSrc(path);
}

void Canvas::renderCurrentFrameToQImage(QImage *frame)
{
    QPainter p(frame);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    //p.scale(mResolutionPercent, mResolutionPercent);
    //p.translate(getAbsolutePos() - mRenderRect.topLeft());

    p.translate(-mRenderRect.topLeft()*mResolutionPercent);
    //p.translate(-mCanvasRect.topLeft());
    p.scale(mResolutionPercent, mResolutionPercent);
    p.setTransform(QTransform(mCanvasTransformMatrix), true);

    Canvas::drawPreviewPixmap(&p);

    p.end();
}

void Canvas::renderFinalCurrentFrameToQImage(QImage *frame)
{
    QPainter p(frame);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    Canvas::renderFinal(&p);

    p.end();
}

QMatrix Canvas::getCombinedRenderTransform()
{
    QMatrix matrix;
    matrix.scale(mCanvasTransformMatrix.m11(),
                 mCanvasTransformMatrix.m22() );
    return matrix;
}

QMatrix Canvas::getCombinedFinalRenderTransform()
{
    return QMatrix();
}

void Canvas::schedulePivotUpdate()
{
    if(mRotPivot->isRotating() || mRotPivot->isScaling()) return;
    mPivotUpdateNeeded = true;
}

void Canvas::updatePivotIfNeeded()
{
    if(mPivotUpdateNeeded) {
        mPivotUpdateNeeded = false;
        updatePivot();
    }
}

void Canvas::makePointCtrlsSymmetric()
{
    setPointCtrlsMode(CtrlsMode::CTRLS_SYMMETRIC);
}

void Canvas::makePointCtrlsSmooth()
{
    setPointCtrlsMode(CtrlsMode::CTRLS_SMOOTH);
}

void Canvas::makePointCtrlsCorner()
{
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
        if(isPointsSelectionEmpty() ||
           !mGlobalPivotVisible) {
            mRotPivot->hide();
        } else {
            mRotPivot->show();
        }
        if(getPointsSelectionCount() == 1) {
            mRotPivot->setAbsolutePos(
                        getSelectedPointsAbsPivotPos() + QPointF(0., 20.),
                        false);
        } else {
            mRotPivot->setAbsolutePos(getSelectedPointsAbsPivotPos(),
                                      false);
        }
    } else if(mCurrentMode == MOVE_PATH) {
        if(isSelectionEmpty() ||
           mLocalPivot ||
           !mGlobalPivotVisible) {
            mRotPivot->hide();
        } else {
            mRotPivot->show();
            mRotPivot->setAbsolutePos(getSelectedBoxesAbsPivotPos(),
                                      false);
        }
    }
}

void Canvas::setCanvasMode(CanvasMode mode) {
    mCurrentMode = mode;

    mHoveredPoint = NULL;
    if(mHoveredEdge != NULL) {
        delete mHoveredEdge;
        mHoveredEdge = NULL;
    }
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
    //mCanvasWidget->setMouseTracking(true);
    mCanvasWidget->grabMouse();
}

void Canvas::releaseMouseAndDontTrack() {
    mIsMouseGrabbing = false;
    //mCanvasWidget->setMouseTracking(false);
    mCanvasWidget->releaseMouse();
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
    if(mPreviewing) return;

    bool isGrabbingMouse = mCanvasWidget->mouseGrabber() == mCanvasWidget;
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
            foreach(BoundingBox *box, mSelectedBoxes) {
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
           QPoint cursorPos = mCanvasWidget->mapFromGlobal(QCursor::pos());
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

           QPoint cursorPos = mCanvasWidget->mapFromGlobal(QCursor::pos());
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

            QPoint cursorPos = mCanvasWidget->mapFromGlobal(QCursor::pos());
            setLastMouseEventPosAbs(cursorPos);
            setLastMousePressPosAbs(cursorPos);
            mDoubleClick = false;
            mFirstMouseMove = true;

            grabMouseAndTrack();
         } else if(event->key() == Qt::Key_A && isCtrlPressed(event) && !isGrabbingMouse) {
           if(isShiftPressed()) {
               mCurrentBoxesGroup->deselectAllBoxesFromBoxesGroup();
           } else {
               mCurrentBoxesGroup->selectAllBoxesFromBoxesGroup();
           }
        } else if(event->key() == Qt::Key_P) {
            mGlobalPivotVisible = !mGlobalPivotVisible;
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
    moveByRel(QPointF( (mCanvasWidget->width() - mVisibleWidth)*0.5,
                    (mCanvasWidget->height() - mVisibleHeight)*0.5) );

}

void Canvas::fitCanvasToSize() {
    mCanvasTransformMatrix.reset();
    mVisibleHeight = mHeight + 20;
    mVisibleWidth = mWidth + 20;
    qreal widthScale = mCanvasWidget->width()/mVisibleWidth;
    qreal heightScale = mCanvasWidget->height()/mVisibleHeight;
    zoomCanvas(qMin(heightScale, widthScale), QPointF(0., 0.));
    mVisibleHeight = mCanvasTransformMatrix.m22()*mHeight;
    mVisibleWidth = mCanvasTransformMatrix.m11()*mWidth;
    moveByRel(QPointF( (mCanvasWidget->width() - mVisibleWidth)*0.5,
                    (mCanvasWidget->height() - mVisibleHeight)*0.5) );

}

void Canvas::moveByRel(QPointF trans) {
    trans = mapAbsPosToRel(trans) -
            mapAbsPosToRel(QPointF(0, 0));

    mLastPressPosRel = mapAbsPosToRel(mLastPressPosRel);

    mCanvasTransformMatrix.translate(trans.x(), trans.y());

    mLastPressPosRel = mCanvasTransformMatrix.map(mLastPressPosRel);
    schedulePivotUpdate();
}

void Canvas::updateAfterFrameChanged(const int &currentFrame) {
    anim_mCurrentAbsFrame = currentFrame;
    foreach(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
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
    const SWT_Type &type = rules.type;
    const bool &alwaysShowChildren = rules.alwaysShowChildren;
    if(alwaysShowChildren) {
        return false;
    } else {
        if(type == SWT_AllTypes) {
        } else if(type == SWT_SingleSound) {
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
    addUpdateScheduler(new AddBoxAwaitingUpdateScheduler(this));
    //mCanvasWidget->addBoxAwaitingUpdate(this);
}

int Canvas::getCurrentFrame() {
    return anim_mCurrentAbsFrame;
}

int Canvas::getFrameCount() {
    return mFrameCount;
}

SoundComposition *Canvas::getSoundComposition() {
    return mSoundComposition;
}
