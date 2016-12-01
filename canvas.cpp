#include "canvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include "undoredo.h"
#include "mainwindow.h"
#include "updatescheduler.h"
#include "pathpivot.h"
#include "boxeslist.h"
#include "imagebox.h"

bool Canvas::mEffectsPaintEnabled = true;

Canvas::Canvas(FillStrokeSettingsWidget *fillStrokeSettings,
               MainWindow *parent) :
    QWidget(parent),
    BoxesGroup(fillStrokeSettings)
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);

    mBoxListItemDetailsVisible = true;
    incNumberPointers();

    mPreviewFPSTimer = new QTimer(this);
    mPreviewFPSTimer->setInterval(1000/24.);
    connect(mPreviewFPSTimer, SIGNAL(timeout()),
            this, SLOT(nextPreviewFrame()) );

    mCurrentBoxesGroup = this;
    mIsCurrentGroup = true;
    setFocusPolicy(Qt::StrongFocus);
    mRotPivot = new PathPivot(this);
    mRotPivot->hide();
    setMinimumSize(500, 500);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    mCurrentMode = MOVE_PATH;
    //setCanvasMode(MOVE_PATH);
}

Canvas::~Canvas()
{
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

void Canvas::enableHighQualityPaint() {
    mHighQualityPaint = true;
}

void Canvas::disableHighQualityPaint() {
    mHighQualityPaint = false;
}

void Canvas::updateAllBoxes() {
    BoxesGroup::updateAllBoxes();
    callUpdateSchedulers();
}

bool Canvas::highQualityPaint() {
    return mHighQualityPaint;
}

void Canvas::enableEffectsPaint()
{
    mEffectsPaintEnabled = true;
}

void Canvas::disableEffectsPaint()
{
    mEffectsPaintEnabled = false;
}

bool Canvas::effectsPaintEnabled()
{
    return mEffectsPaintEnabled;
}

void Canvas::updateDisplayedFillStrokeSettings() {
    mCurrentBoxesGroup->setDisplayedFillStrokeSettingsFromLastSelected();
}

QRectF Canvas::getPixBoundingRect()
{
    QPointF absPos = getAbsolutePos();
    return QRectF(absPos, absPos + QPointF(mVisibleWidth, mVisibleHeight));
}

void Canvas::pickPathForSettings() {
    setCanvasMode(PICK_PATH_SETTINGS);
}

void Canvas::scale(qreal scaleXBy, qreal scaleYBy, QPointF absOrigin)
{
    QPointF transPoint = -mapAbsPosToRel(absOrigin);

    mLastPressPos = mapAbsPosToRel(mLastPressPos);

    mCombinedTransformMatrix.translate(-transPoint.x(), -transPoint.y());
    mCombinedTransformMatrix.scale(scaleXBy, scaleYBy);
    mCombinedTransformMatrix.translate(transPoint.x(), transPoint.y());

    mLastPressPos = mCombinedTransformMatrix.map(mLastPressPos);

    updateAfterCombinedTransformationChanged();
}

void Canvas::scale(qreal scaleBy, QPointF absOrigin)
{
    scale(scaleBy, scaleBy, absOrigin);
}

bool Canvas::processUnfilteredKeyEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_F1) {
        setCanvasMode(CanvasMode::MOVE_PATH);
    } else if(event->key() == Qt::Key_F2) {
        setCanvasMode(CanvasMode::MOVE_POINT);
    } else if(event->key() == Qt::Key_F3) {
        setCanvasMode(CanvasMode::ADD_POINT);
    } else if(event->key() == Qt::Key_F4) {
        setCanvasMode(CanvasMode::ADD_CIRCLE);
    } else if(event->key() == Qt::Key_F5) {
        setCanvasMode(CanvasMode::ADD_RECTANGLE);
    } else if(event->key() == Qt::Key_F6) {
        setCanvasMode(CanvasMode::ADD_TEXT);
    } else {
        return false;
    }
    return true;
}

bool Canvas::processFilteredKeyEvent(QKeyEvent *event) {
    if(processUnfilteredKeyEvent(event)) return true;
    if(!hasFocus()) return false;
    if(isCtrlPressed() && event->key() == Qt::Key_G) {
        if(isShiftPressed()) {
            mCurrentBoxesGroup->ungroupSelected();
        } else {
            BoxesGroup *newGroup = mCurrentBoxesGroup->groupSelectedBoxes();
            if(newGroup != NULL) {
                setCurrentBoxesGroup(newGroup);
            }
        }
        schedulePivotUpdate();

        callUpdateSchedulers();
    } else if(event->key() == Qt::Key_PageUp) {
        mCurrentBoxesGroup->raiseSelectedBoxes();
    } else if(event->key() == Qt::Key_PageDown) {
        mCurrentBoxesGroup->lowerSelectedBoxes();
    } else if(event->key() == Qt::Key_End) {
        mCurrentBoxesGroup->lowerSelectedBoxesToBottom();
    } else if(event->key() == Qt::Key_Home) {
        mCurrentBoxesGroup->raiseSelectedBoxesToTop();
    } else {
        return false;
    }

    return true;
}

void Canvas::setCurrentBoxesGroup(BoxesGroup *group) {
    mCurrentBoxesGroup->setIsCurrentGroup(false);
    clearAllPathsSelection();
    clearAllPointsSelection();
    mCurrentBoxesGroup = group;
    group->setIsCurrentGroup(true);
}

void Canvas::rotateBoxesBy(qreal rotChange, QPointF absOrigin, bool startTrans)
{
    mCurrentBoxesGroup->rotateSelectedBy(rotChange, absOrigin, startTrans);
}

void Canvas::scaleBoxesBy(qreal scaleBy, QPointF absOrigin, bool startTrans)
{
    mCurrentBoxesGroup->scaleSelectedBy(scaleBy, absOrigin, startTrans);
}

void Canvas::scaleBoxesBy(qreal scaleXBy, qreal scaleYBy, QPointF absOrigin,
                          bool startTrans)
{
    mCurrentBoxesGroup->scaleSelectedBy(scaleXBy, scaleYBy, absOrigin,
                                        startTrans);
}

void Canvas::saveToSql()
{
    foreach(BoundingBox *box, mChildren) {
        box->saveToSql(0);
    }
}

void Canvas::saveSelectedToSqlForCurrentBox() {
    mCurrentBoxesGroup->saveSelectedToSql();
}

void Canvas::loadAllBoxesFromSql(bool loadInBox) {
    BoxesGroup *container = mCurrentBoxesGroup->loadChildrenFromSql(0,
                                                                    loadInBox);
}

void Canvas::setPartialRepaintRect(QRectF absRect) {
    mPartialRepaintRect = absRect;
}

void Canvas::partialRepaintRectToPoint(QPointF point) {
    mPartialRepaintRect.setTopLeft(point);
    mPartialRepaintRect.setBottomRight(point);
}

void Canvas::makePartialRepaintInclude(QPointF pointToInclude) {
    if(pointToInclude.x() > mPartialRepaintRect.right() ) {
        mPartialRepaintRect.setRight(pointToInclude.x() );
    } else if(pointToInclude.x() < mPartialRepaintRect.left() ) {
        mPartialRepaintRect.setLeft(pointToInclude.x() );
    }
    if(pointToInclude.y() > mPartialRepaintRect.bottom() ) {
        mPartialRepaintRect.setBottom(pointToInclude.y() );
    } else if(pointToInclude.y() < mPartialRepaintRect.top() ) {
        mPartialRepaintRect.setTop(pointToInclude.y() );
    }
}

void Canvas::repaintPartially() {
    mFullRepaint = false;
    repaint();
    mFullRepaint = true;
}

void Canvas::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    QPointF absPos = getAbsolutePos();
    QRectF viewRect = QRectF(absPos,
                             absPos + QPointF(mVisibleWidth, mVisibleHeight));

    if(mPreviewing) {
        QPainterPath path;
        path.addRect(0, 0, width() + 1, height() + 1);
        QPainterPath viewRectPath;
        viewRectPath.addRect(viewRect);
        p.setBrush(QColor(0, 0, 0));
        p.setPen(Qt::NoPen);
        p.drawPath(path.subtracted(viewRectPath));

        p.drawImage(mRenderRect.topLeft(), *mCurrentPreviewImg);
    } else if(mFullRepaint) {
        p.fillRect(0, 0, width() + 1, height() + 1, QColor(75, 75, 75));
        p.fillRect(viewRect, Qt::white);

        foreach(BoundingBox *box, mChildren){
            box->drawPixmap(&p);
        }
        mCurrentBoxesGroup->drawSelected(&p, mCurrentMode);

        p.setPen(QPen(Qt::black, 2.f));
        p.setBrush(Qt::NoBrush);
        p.drawRect(viewRect.adjusted(-1., -1., 1., 1.));

        p.setPen(QPen(QColor(0, 0, 255, 125), 2.f, Qt::DotLine));
        if(mSelecting) {
            p.drawRect(mSelectionRect);
        }
        if(mCurrentMode == CanvasMode::MOVE_PATH) {
            mRotPivot->draw(&p);
        }

        if(mInputTransformationEnabled) {
            QRect inputRect = QRect(40, height() - 20, 100, 20);
            p.fillRect(inputRect, QColor(225, 225, 225));
            QString text;
            if(mXOnlyTransform) {
                text = " x: " + mInputText + "|";
            } else if(mYOnlyTransform) {
                text = " y: " + mInputText + "|";
            } else {
                text = " x, y: " + mInputText + "|";
            }
            p.drawText(inputRect, Qt::AlignVCenter, text);
        }
    }

    if(hasFocus() ) {
        p.setPen(QPen(Qt::red, 4.));
    } else {
        p.setPen(Qt::NoPen);
    }
    p.setBrush(Qt::NoBrush);
    p.drawRect(rect());

    p.end();
}

bool Canvas::isMovingPath() {
    return mCurrentMode == CanvasMode::MOVE_PATH;
}

qreal Canvas::getCurrentCanvasScale()
{
    return mCombinedTransformMatrix.m11();
}

void Canvas::ctrlsVisiblityChanged()
{
    mCurrentBoxesGroup->updateSelectedPointsAfterCtrlsVisiblityChanged();
}

QSize Canvas::getCanvasSize()
{
    return QSize(mWidth, mHeight);
}

void Canvas::playPreview()
{
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    if(mPreviewFrames.isEmpty() ) return;
    mCurrentPreviewFrameId = 0;
    mCurrentPreviewImg = mPreviewFrames.first();
    mPreviewing = true;
    repaint();

    mPreviewFPSTimer->start();
}

void Canvas::clearPreview() {
    if(mPreviewing) {
        mPreviewFPSTimer->stop();
        mPreviewing = false;
        for(int i = 0; i < mPreviewFrames.length(); i++) {
            delete mPreviewFrames.at(i);
        }
        mPreviewFrames.clear();
        mMainWindow->previewFinished();

        setAttribute(Qt::WA_OpaquePaintEvent, true);
    }
}

void Canvas::nextPreviewFrame()
{
    mCurrentPreviewFrameId++;
    if(mCurrentPreviewFrameId >= mPreviewFrames.length() ) {
        clearPreview();
    } else {
        mCurrentPreviewImg = mPreviewFrames.at(mCurrentPreviewFrameId);
    }
    repaint();
}

void Canvas::raiseAction()
{
    mCurrentBoxesGroup->raiseSelectedBoxes();
}

void Canvas::lowerAction()
{
    mCurrentBoxesGroup->lowerSelectedBoxes();
}

void Canvas::raiseToTopAction()
{
    mCurrentBoxesGroup->raiseSelectedBoxesToTop();
}

void Canvas::lowerToBottomAction()
{
    mCurrentBoxesGroup->lowerSelectedBoxesToBottom();
}

void Canvas::objectsToPathAction()
{
    mCurrentBoxesGroup->convertSelectedBoxesToPath();
}

void Canvas::updateRenderRect() {
    mRenderRect = QRectF(qMax(mCombinedTransformMatrix.dx(), 0.),
                         qMax(mCombinedTransformMatrix.dy(), 0.),
                         qMin(mVisibleWidth, (qreal)width()),
                         qMin(mVisibleHeight, (qreal)height()));
}

void Canvas::renderCurrentFrameToPreview()
{
    QImage *image = new QImage(mRenderRect.size().toSize(),
                               QImage::Format_ARGB32);
    image->fill(Qt::transparent);
    renderCurrentFrameToQImage(image);
    mPreviewFrames << image;
}

void Canvas::renderCurrentFrameToOutput(QString renderDest)
{
    QImage *image = new QImage(mWidth, mHeight,
                               QImage::Format_ARGB32);
    image->fill(Qt::transparent);
    renderFinalCurrentFrameToQImage(image);
    image->save(renderDest + QString::number(getCurrentFrame()) + ".png");
    delete image;
}

void Canvas::renderCurrentFrameToQImage(QImage *frame)
{
    QPainter p(frame);
    p.translate(getAbsolutePos() - mRenderRect.topLeft());
    p.setRenderHint(QPainter::Antialiasing);

    BoxesGroup::render(&p);

    p.end();
}

void Canvas::renderFinalCurrentFrameToQImage(QImage *frame)
{
    QPainter p(frame);
    p.setRenderHint(QPainter::Antialiasing);

    BoxesGroup::renderFinal(&p);

    p.end();
}

QMatrix Canvas::getCombinedRenderTransform()
{
    QMatrix matrix;
    matrix.scale(mCombinedTransformMatrix.m11(),
                 mCombinedTransformMatrix.m22() );
    return matrix;
}

QMatrix Canvas::getCombinedFinalRenderTransform()
{
    return QMatrix();
}

void Canvas::schedulePivotUpdate()
{
    mPivotUpdateNeeded = true;
}

void Canvas::clearAll() {
    setCurrentBoxesGroup(this);

    foreach(BoundingBox *box, mChildren) {
        box->decNumberPointers();
    }
    mChildren.clear();

    resetTransormation();
}

void Canvas::updatePivotIfNeeded()
{
    if(mPivotUpdateNeeded) {
        mPivotUpdateNeeded = false;
        updatePivot();
    }
}

void Canvas::connectPointsSlot()
{
    mCurrentBoxesGroup->connectPoints();
}

void Canvas::disconnectPointsSlot()
{
    mCurrentBoxesGroup->disconnectPoints();
}

void Canvas::mergePointsSlot()
{
    mCurrentBoxesGroup->mergePoints();
}

void Canvas::makePointCtrlsSymmetric()
{
    mCurrentBoxesGroup->setPointCtrlsMode(CtrlsMode::CTRLS_SYMMETRIC);
}

void Canvas::makePointCtrlsSmooth()
{
    mCurrentBoxesGroup->setPointCtrlsMode(CtrlsMode::CTRLS_SMOOTH);
}

void Canvas::makePointCtrlsCorner()
{
    mCurrentBoxesGroup->setPointCtrlsMode(CtrlsMode::CTRLS_CORNER);
}

void Canvas::moveSecondSelectionPoint(QPointF pos) {
    mSelectionRect.setBottomRight(pos);

}

void Canvas::startSelectionAtPoint(QPointF pos) {
    mSelectionRect.setTopLeft(pos);
    mSelectionRect.setBottomRight(pos);

}

void Canvas::updatePivot() {
    if(mCurrentMode != MOVE_PATH) {
        return;
    }
    if(mCurrentBoxesGroup->isSelectionEmpty() ) {
        mRotPivot->hide();
    } else {
        mRotPivot->show();
        mRotPivot->setAbsolutePos(mCurrentBoxesGroup->getSelectedBoxesPivotPos(),
                                  false);
    }
}

//void Canvas::updateAfterCombinedTransformationChanged()
//{
//    BoundingBox::updateAfterCombinedTransformationChanged();
//    mRotPivot->updateRotationMappedPath();
//}

void Canvas::setCanvasMode(CanvasMode mode) {
    mCurrentMode = mode;
    if(mCurrentMode == MOVE_PATH) {
        setCursor(QCursor(Qt::ArrowCursor) );
    } else if(mCurrentMode == MOVE_POINT) {
        setCursor(QCursor(QPixmap("pixmaps/cursor-node.xpm"), 0, 0) );
    } else if(mCurrentMode == PICK_PATH_SETTINGS) {
        setCursor(QCursor(QPixmap("pixmaps/cursor_color_picker.png"), 2, 20) );
    } else if(mCurrentMode == ADD_CIRCLE) {
        setCursor(QCursor(QPixmap("pixmaps/cursor-ellipse.xpm"), 4, 4) );
    } else if(mCurrentMode == ADD_RECTANGLE) {
        setCursor(QCursor(QPixmap("pixmaps/cursor-rect.xpm"), 4, 4) );
    } else if(mCurrentMode == ADD_TEXT) {
        setCursor(QCursor(QPixmap("pixmaps/cursor-text.xpm"), 4, 4) );
    } else {
        setCursor(QCursor(QPixmap("pixmaps/cursor-pen.xpm"), 4, 4) );
    }
    clearAllPointsSelection();
    if(mCurrentMode == MOVE_PATH) {
        schedulePivotUpdate();
    }

    mMainWindow->updateCanvasModeButtonsChecked(mode);
}

void Canvas::clearAndDisableInput() {
    mInputTransformationEnabled = false;
    mInputText = "";
}

void Canvas::updateInputValue() {
    if(mInputText.isEmpty()) {
        mInputTransformationEnabled = false;
    } else {
        mInputTransformationEnabled = true;
        mInputTransformationValue = mInputText.toDouble();
    }

    updateTransformation();
}

void Canvas::grabMouseAndTrack() {
    mIsMouseGrabbing = true;
    setMouseTracking(true);
    grabMouse();
}

void Canvas::setFontFamilyAndStyle(QString family, QString style)
{
    mCurrentBoxesGroup->setSelectedFontFamilyAndStyle(family, style);

    callUpdateSchedulers();
}

void Canvas::setFontSize(qreal size)
{
    mCurrentBoxesGroup->setSelectedFontSize(size);

    callUpdateSchedulers();
}

void Canvas::releaseMouseAndDontTrack() {
    mIsMouseGrabbing = false;
    setMouseTracking(false);
    releaseMouse();
}

void Canvas::groupSelectedBoxesAction() {
    BoxesGroup *newGroup = mCurrentBoxesGroup->groupSelectedBoxes();
    if(newGroup != NULL) {
        setCurrentBoxesGroup(newGroup);
    }
}

void Canvas::keyPressEvent(QKeyEvent *event)
{
    if(mPreviewing) return;
    bool isGrabbingMouse = mouseGrabber() == this;
    if(isGrabbingMouse) {
        if(event->key() == Qt::Key_Escape) {
            cancelCurrentTransform();
            return;
        } else if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            mTransformationFinishedBeforeMouseRelease = true;
            handleMouseRelease(mLastMouseEventPos);
            return;
        } else if(event->key() == Qt::Key_Minus) {
            if( ((mInputText.isEmpty()) ? false : mInputText.at(0) == '-') ) {
                mInputText.remove("-");
            } else {
                mInputText.prepend("-");
            }
            updateInputValue();
            return;
        } else if(event->key() == Qt::Key_0) {
            mInputText += "0";
            if(mInputText == "0" || mInputText == "-0") mInputText += ".";
            updateInputValue();
            return;
        } else if(event->key() == Qt::Key_1) {
            mInputText += "1";
            updateInputValue();
            return;
        } else if(event->key() == Qt::Key_2) {
            mInputText += "2";
            updateInputValue();
            return;
        } else if(event->key() == Qt::Key_3) {
            mInputText += "3";
            updateInputValue();
            return;
        } else if(event->key() == Qt::Key_4) {
            mInputText += "4";
            updateInputValue();
            return;
        } else if(event->key() == Qt::Key_5) {
            mInputText += "5";
            updateInputValue();
            return;
        } else if(event->key() == Qt::Key_6) {
            mInputText += "6";
            updateInputValue();
            return;
        } else if(event->key() == Qt::Key_7) {
            mInputText += "7";
            updateInputValue();
            return;
        } else if(event->key() == Qt::Key_8) {
            mInputText += "8";
            updateInputValue();
            return;
        } else if(event->key() == Qt::Key_9) {
            mInputText += "9";
            updateInputValue();
            return;
        } else if(event->key() == Qt::Key_Period ||
                  event->key() == Qt::Key_Comma) {
            if(mInputText.contains(".")) return;
            mInputText += ".";
            updateInputValue();
            return;
        } else if(event->key() == Qt::Key_Backspace) {
            mInputText.chop(1);
            if(mInputText == "0" ||
               mInputText == "-" ||
               mInputText == "-0") mInputText = "";
            updateInputValue();
            return;
        } else if(event->key() == Qt::Key_X) {
            mXOnlyTransform = !mXOnlyTransform;
            mYOnlyTransform = false;

            updateTransformation();
            return;
        } else if(event->key() == Qt::Key_Y) {
            mYOnlyTransform = !mYOnlyTransform;
            mXOnlyTransform = false;

            updateTransformation();
            return;
        }
    }
    if(event->key() == Qt::Key_0) {
        fitCanvasToSize();
    } else if(event->key() == Qt::Key_1) {
        resetTransormation();
    } else if(event->key() == Qt::Key_Delete) {
       if(mCurrentMode == MOVE_POINT) {
           if(isShiftPressed()) {
               mCurrentBoxesGroup->removeSelectedPointsApproximateAndClearList();
           } else {
               mCurrentBoxesGroup->removeSelectedPointsAndClearList();
           }
       } else if(mCurrentMode == MOVE_PATH) {
           mCurrentBoxesGroup->removeSelectedBoxesAndClearList();
       }
    } else if(isCtrlPressed() && event->key() == Qt::Key_G) {
       if(isShiftPressed()) {
           mCurrentBoxesGroup->ungroupSelected();
       } else {
           groupSelectedBoxesAction();
       }

    } else if(event->key() == Qt::Key_PageUp) {
       raiseAction();
    } else if(event->key() == Qt::Key_PageDown) {
       lowerAction();
    } else if(event->key() == Qt::Key_End) {
       lowerToBottomAction();
    } else if(event->key() == Qt::Key_Home) {
       raiseToTopAction();
    } else if(event->key() == Qt::Key_G && isAltPressed(event)) {
        mCurrentBoxesGroup->resetSelectedTranslation();
    } else if(event->key() == Qt::Key_S && isAltPressed(event)) {
        mCurrentBoxesGroup->resetSelectedScale();
    } else if(event->key() == Qt::Key_R && isAltPressed(event)) {
        mCurrentBoxesGroup->resetSelectedRotation();
    } else if(event->key() == Qt::Key_R && isMovingPath() && !isGrabbingMouse) {
       mTransformationFinishedBeforeMouseRelease = false;
       mLastMouseEventPos = mapFromGlobal(QCursor::pos());
       mLastPressPos = mLastMouseEventPos;
       mRotPivot->startRotating();
       mDoubleClick = false;
       mFirstMouseMove = true;

       grabMouseAndTrack();
    } else if(event->key() == Qt::Key_S && isMovingPath() && !isGrabbingMouse) {
       mTransformationFinishedBeforeMouseRelease = false;
       mXOnlyTransform = false;
       mYOnlyTransform = false;

       mLastMouseEventPos = mapFromGlobal(QCursor::pos());
       mLastPressPos = mLastMouseEventPos;
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

        mLastMouseEventPos = mapFromGlobal(QCursor::pos());
        mLastPressPos = mLastMouseEventPos;
        mDoubleClick = false;
        mFirstMouseMove = true;

        grabMouseAndTrack();
     } else if(event->key() == Qt::Key_A && isCtrlPressed(event) && !isGrabbingMouse) {
       if(isShiftPressed()) {
           mCurrentBoxesGroup->deselectAllBoxes();
       } else {
           mCurrentBoxesGroup->selectAllBoxes();
       }
    }

    schedulePivotUpdate();

    callUpdateSchedulers();
}

void Canvas::clearAllPathsSelection() {
    mCurrentBoxesGroup->clearBoxesSelection();
    if(mLastPressedBox != NULL) {
        mLastPressedBox->deselect();
        mLastPressedBox = NULL;
    }
}

void Canvas::clearAllPointsSelection() {
    mCurrentBoxesGroup->clearPointsSelection();
    if(mLastPressedPoint != NULL) {
        mLastPressedPoint->deselect();
        mLastPressedPoint = NULL;
    }
    setCurrentEndPoint(NULL);
}

void Canvas::setCurrentEndPoint(PathPoint *point)
{
    if(mCurrentEndPoint != NULL) {
        mCurrentEndPoint->deselect();
    }
    if(point != NULL) {
        point->select();
    }
    mCurrentEndPoint = point;
}

void Canvas::selectOnlyLastPressedBox() {
    mCurrentBoxesGroup->clearBoxesSelection();
    if(mLastPressedBox == NULL) {
        return;
    }
    mCurrentBoxesGroup->addBoxToSelection(mLastPressedBox);
}

void Canvas::selectOnlyLastPressedPoint() {
    mCurrentBoxesGroup->clearPointsSelection();
    if(mLastPressedPoint == NULL) {
        return;
    }
    mCurrentBoxesGroup->addPointToSelection(mLastPressedPoint);
}

void Canvas::resetTransormation() {
    mCombinedTransformMatrix.reset();
    mVisibleHeight = mHeight;
    mVisibleWidth = mWidth;
    updateAfterCombinedTransformationChanged();

}

void Canvas::fitCanvasToSize() {
    mCombinedTransformMatrix.reset();
    mVisibleHeight = mHeight + 20;
    mVisibleWidth = mWidth + 20;
    updateAfterCombinedTransformationChanged();
    qreal widthScale = width()/mVisibleWidth;
    qreal heightScale = height()/mVisibleHeight;
    scale(qMin(heightScale, widthScale), QPointF(0.f, 0.f));
    mVisibleHeight = mCombinedTransformMatrix.m22()*mHeight;
    mVisibleWidth = mCombinedTransformMatrix.m11()*mWidth;
    moveBy(QPointF( (width() - mVisibleWidth)*0.5f,
                    (height() - mVisibleHeight)*0.5f) );
    updateAfterCombinedTransformationChanged();

}

void Canvas::moveBy(QPointF trans)
{
    trans = mapAbsPosToRel(trans) -
            mapAbsPosToRel(QPointF(0, 0));

    mLastPressPos = mapAbsPosToRel(mLastPressPos);

    mCombinedTransformMatrix.translate(trans.x(), trans.y());

    mLastPressPos = mCombinedTransformMatrix.map(mLastPressPos);
    updateAfterCombinedTransformationChanged();
    schedulePivotUpdate();
}

void Canvas::updateAfterFrameChanged(int currentFrame)
{
    foreach(BoundingBox *box, mChildren) {
        box->updateAfterFrameChanged(currentFrame);
    }
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
    if(pointSrc->getParentPath() == pointDest->getParentPath()) {
        return;
    }
    VectorPath *pathSrc = pointSrc->getParentPath();
    VectorPath *pathDest = pointDest->getParentPath();
    startNewUndoRedoSet();
    setCurrentEndPoint(pointDest);
    if(pointSrc->hasNextPoint()) {
        PathPoint *point = pointSrc;
        bool mirror = pointDest->hasNextPoint();
        while(point != NULL) {
            QPointF startCtrlPtPos;
            QPointF endCtrlPtPos;
            getMirroredCtrlPtAbsPos(mirror, point,
                                    &startCtrlPtPos, &endCtrlPtPos);
            PathPoint *newPoint = new PathPoint(pathDest);
            newPoint->setAbsolutePos(point->getAbsolutePos());
            newPoint->moveStartCtrlPtToAbsPos(startCtrlPtPos);
            newPoint->moveEndCtrlPtToAbsPos(endCtrlPtPos);

            setCurrentEndPoint(mCurrentEndPoint->addPoint(newPoint) );
            point = point->getNextPoint();
        }
    } else {
        PathPoint *point = pointSrc;
        bool mirror = pointDest->hasPreviousPoint();
        while(point != NULL) {
            QPointF startCtrlPtPos;
            QPointF endCtrlPtPos;
            getMirroredCtrlPtAbsPos(mirror, point,
                                    &startCtrlPtPos, &endCtrlPtPos);

            PathPoint *newPoint = new PathPoint(pathDest);
            newPoint->setAbsolutePos(point->getAbsolutePos());
            newPoint->moveStartCtrlPtToAbsPos(startCtrlPtPos);
            newPoint->moveEndCtrlPtToAbsPos(endCtrlPtPos);

            setCurrentEndPoint(mCurrentEndPoint->addPoint(newPoint) );
            point = point->getPreviousPoint();
        }
    }
    mCurrentBoxesGroup->removeChild(pathSrc);

    finishUndoRedoSet();
}
