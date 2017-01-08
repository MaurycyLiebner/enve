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

bool Canvas::mEffectsPaintEnabled = true;
qreal Canvas::mResolutionPercent = 1.;

Canvas::Canvas(FillStrokeSettingsWidget *fillStrokeSettings,
               CanvasWidget *canvasWidget) :
    BoxesGroup(fillStrokeSettings)
{
    mCanvasWidget = canvasWidget;
    incNumberPointers();

    mPreviewFPSTimer = new QTimer(this);
    mPreviewFPSTimer->setInterval(1000/24.);
    connect(mPreviewFPSTimer, SIGNAL(timeout()),
            this, SLOT(nextPreviewFrame()) );

    mCurrentBoxesGroup = this;
    mIsCurrentGroup = true;

    mRotPivot = new PathPivot(this);
    mRotPivot->hide();

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

qreal Canvas::getResolutionPercent()
{
    return Canvas::mResolutionPercent;
}

void Canvas::setResolutionPercent(qreal percent)
{
    mResolutionPercent = percent;
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
    if(!mCanvasWidget->hasFocus()) return false;
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

void Canvas::rotatePointsBy(qreal rotChange, QPointF absOrigin, bool startTrans)
{
    mCurrentBoxesGroup->rotateSelectedPointsBy(rotChange, absOrigin, startTrans);
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

void Canvas::scalePointsBy(qreal scaleXBy, qreal scaleYBy, QPointF absOrigin,
                            bool startTrans)
{
    mCurrentBoxesGroup->scaleSelectedPointsBy(scaleXBy, scaleYBy, absOrigin,
                                              startTrans);
}

void Canvas::saveToSql(QSqlQuery *query)
{
    foreach(BoundingBox *box, mChildren) {
        box->saveToSql(query, 0);
    }
}

void Canvas::saveSelectedToSqlForCurrentBox(QSqlQuery *query) {
    mCurrentBoxesGroup->saveSelectedToSql(query);
}

void Canvas::loadAllBoxesFromSql(bool loadInBox) {
    BoxesGroup *container = mCurrentBoxesGroup->loadChildrenFromSql(0,
                                                                    loadInBox);
}

void Canvas::paintEvent(QPainter *p)
{
    p->setRenderHint(QPainter::Antialiasing);
    p->setRenderHint(QPainter::SmoothPixmapTransform);

    QPointF absPos = getAbsolutePos();
    QRectF viewRect = QRectF(absPos,
                             absPos + QPointF(mVisibleWidth, mVisibleHeight));

    if(mPreviewing) {
        QPainterPath path;
        path.addRect(0, 0, mCanvasWidget->width() + 1, mCanvasWidget->height() + 1);
        QPainterPath viewRectPath;
        viewRectPath.addRect(viewRect);
        p->setBrush(QColor(0, 0, 0));
        p->setPen(Qt::NoPen);
        p->drawPath(path.subtracted(viewRectPath));

        p->save();
        qreal reversedRes = 1/mResolutionPercent;
        p->translate(mRenderRect.topLeft());
        p->scale(reversedRes, reversedRes);
        p->drawImage(QPointF(0., 0.), *mCurrentPreviewImg);
        p->restore();
    } else {
        p->fillRect(0, 0, mCanvasWidget->width() + 1, mCanvasWidget->height() + 1, QColor(75, 75, 75));
        p->fillRect(viewRect, Qt::white);

        foreach(BoundingBox *box, mChildren){
            box->drawPixmap(p);
        }
        mCurrentBoxesGroup->drawSelected(p, mCurrentMode);

        p->setPen(QPen(Qt::black, 2.));
        p->setBrush(Qt::NoBrush);
        p->drawRect(viewRect.adjusted(-1., -1., 1., 1.));

        p->setPen(QPen(QColor(0, 0, 255, 125), 2., Qt::DotLine));
        if(mSelecting) {
            p->drawRect(mSelectionRect);
        }
        if(mCurrentMode == CanvasMode::MOVE_PATH ||
           mCurrentMode == CanvasMode::MOVE_POINT) {
            mRotPivot->draw(p);
        }

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
    mCanvasWidget->setAttribute(Qt::WA_OpaquePaintEvent, false);
    if(mPreviewFrames.isEmpty() ) return;
    mCurrentPreviewFrameId = 0;
    mCurrentPreviewImg = mPreviewFrames.first();
    mPreviewing = true;
    mCanvasWidget->repaint();

    mPreviewFPSTimer->start();
}

void Canvas::clearPreview() {
    if(!mPreviewFrames.isEmpty()) {
        mPreviewFPSTimer->stop();
        mPreviewing = false;
        for(int i = 0; i < mPreviewFrames.length(); i++) {
            delete mPreviewFrames.at(i);
        }
        mPreviewFrames.clear();
        mMainWindow->previewFinished();

        mCanvasWidget->setAttribute(Qt::WA_OpaquePaintEvent, true);
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
    mCanvasWidget->repaint();
}

void Canvas::raiseAction() {
    mCurrentBoxesGroup->raiseSelectedBoxes();
}

void Canvas::lowerAction() {
    mCurrentBoxesGroup->lowerSelectedBoxes();
}

void Canvas::raiseToTopAction() {
    mCurrentBoxesGroup->raiseSelectedBoxesToTop();
}

void Canvas::lowerToBottomAction() {
    mCurrentBoxesGroup->lowerSelectedBoxesToBottom();
}

void Canvas::objectsToPathAction() {
    mCurrentBoxesGroup->convertSelectedBoxesToPath();
}

void Canvas::updateRenderRect() {
    QRectF canvasRect = QRectF(qMax(mCombinedTransformMatrix.dx()*mResolutionPercent,
                                    mCombinedTransformMatrix.dx()),
                               qMax(mCombinedTransformMatrix.dy()*mResolutionPercent,
                                                                   mCombinedTransformMatrix.dy()),
                               mVisibleWidth*mResolutionPercent, mVisibleHeight*mResolutionPercent);
    QRectF canvasWidgetRect = QRectF(0., 0.,
                                     (qreal)mCanvasWidget->width(),
                                     (qreal)mCanvasWidget->height());
    mRenderRect = canvasWidgetRect.intersected(canvasRect);
}

void Canvas::renderCurrentFrameToPreview() {
    QImage *image = new QImage(mRenderRect.size().toSize(),
                               QImage::Format_ARGB32);
    image->fill(Qt::transparent);
    renderCurrentFrameToQImage(image);
    mPreviewFrames << image;
}

void Canvas::renderCurrentFrameToOutput(QString renderDest) {
    QImage *image = new QImage(mWidth, mHeight,
                               QImage::Format_ARGB32);
    image->fill(Qt::transparent);
    renderFinalCurrentFrameToQImage(image);
    image->save(renderDest + QString::number(getCurrentFrame()) + ".png");
    delete image;
}

void Canvas::render(QPainter *p)
{
    if(mVisible) {
        p->save();
        //p->setTransform(QTransform(mCombinedTransformMatrix.inverted()), true);
        foreach(BoundingBox *box, mChildren){
            box->render(p);
        }

        p->restore();
    }
}

void Canvas::renderFinal(QPainter *p)
{
    if(mVisible) {
        p->save();

        foreach(BoundingBox *box, mChildren){
            box->renderFinal(p);
        }

        p->restore();
    }
}
#include "Boxes/animationbox.h"
void Canvas::createAnimationBoxForPaths(const QStringList &paths) {
    new AnimationBox(mCurrentBoxesGroup, paths);
}

void Canvas::renderCurrentFrameToQImage(QImage *frame)
{
    QPainter p(frame);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    p.scale(mResolutionPercent, mResolutionPercent);
    p.translate(getAbsolutePos() - mRenderRect.topLeft());

    Canvas::render(&p);

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
    if(mRotPivot->isRotating() || mRotPivot->isScaling()) return;
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
    callUpdateSchedulers();
}

void Canvas::disconnectPointsSlot()
{
    mCurrentBoxesGroup->disconnectPoints();
    callUpdateSchedulers();
}

void Canvas::mergePointsSlot()
{
    mCurrentBoxesGroup->mergePoints();
    callUpdateSchedulers();
}

void Canvas::makePointCtrlsSymmetric()
{
    mCurrentBoxesGroup->setPointCtrlsMode(CtrlsMode::CTRLS_SYMMETRIC);
    callUpdateSchedulers();
}

void Canvas::makePointCtrlsSmooth()
{
    mCurrentBoxesGroup->setPointCtrlsMode(CtrlsMode::CTRLS_SMOOTH);
    callUpdateSchedulers();
}

void Canvas::makePointCtrlsCorner()
{
    mCurrentBoxesGroup->setPointCtrlsMode(CtrlsMode::CTRLS_CORNER);
    callUpdateSchedulers();
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
        if(mCurrentBoxesGroup->isPointsSelectionEmpty() || !mPivotVisibleDuringPointEdit) {
            mRotPivot->hide();
        } else {
            mRotPivot->show();
        }
        if(mCurrentBoxesGroup->getPointsSelectionCount() == 1) {
                    mRotPivot->setAbsolutePos(mCurrentBoxesGroup->getSelectedPointsAbsPivotPos() + QPointF(0., 20.),
                                              false);
        } else {
            mRotPivot->setAbsolutePos(mCurrentBoxesGroup->getSelectedPointsAbsPivotPos(),
                                      false);
        }
    } else if(mCurrentMode == MOVE_PATH) {
        if(mCurrentBoxesGroup->isSelectionEmpty() ) {
            mRotPivot->hide();
        } else {
            mRotPivot->show();
            mRotPivot->setAbsolutePos(mCurrentBoxesGroup->getSelectedBoxesAbsPivotPos(),
                                      false);
        }
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
        mCanvasWidget->setCursor(QCursor(Qt::ArrowCursor) );
    } else if(mCurrentMode == MOVE_POINT) {
        mCanvasWidget->setCursor(QCursor(QPixmap("pixmaps/cursor-node.xpm"), 0, 0) );
    } else if(mCurrentMode == PICK_PATH_SETTINGS) {
        mCanvasWidget->setCursor(QCursor(QPixmap("pixmaps/cursor_color_picker.png"), 2, 20) );
    } else if(mCurrentMode == ADD_CIRCLE) {
        mCanvasWidget->setCursor(QCursor(QPixmap("pixmaps/cursor-ellipse.xpm"), 4, 4) );
    } else if(mCurrentMode == ADD_RECTANGLE) {
        mCanvasWidget->setCursor(QCursor(QPixmap("pixmaps/cursor-rect.xpm"), 4, 4) );
    } else if(mCurrentMode == ADD_TEXT) {
        mCanvasWidget->setCursor(QCursor(QPixmap("pixmaps/cursor-text.xpm"), 4, 4) );
    } else {
        mCanvasWidget->setCursor(QCursor(QPixmap("pixmaps/cursor-pen.xpm"), 4, 4) );
    }
    clearAllPointsSelection();
    if(mCurrentMode == MOVE_PATH || mCurrentMode == MOVE_POINT) {
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
        mFirstMouseMove = false;
        mInputTransformationEnabled = true;
        mInputTransformationValue = mInputText.toDouble();
    }

    updateTransformation();
}

void Canvas::grabMouseAndTrack() {
    mIsMouseGrabbing = true;
    mCanvasWidget->setMouseTracking(true);
    mCanvasWidget->grabMouse();
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
    mCanvasWidget->setMouseTracking(false);
    mCanvasWidget->releaseMouse();
}

void Canvas::groupSelectedBoxesAction() {
    BoxesGroup *newGroup = mCurrentBoxesGroup->groupSelectedBoxes();
    if(newGroup != NULL) {
        setCurrentBoxesGroup(newGroup);
    }
}

bool Canvas::handleKeyPressEventWhileMouseGrabbing(QKeyEvent *event) {
    if(event->key() == Qt::Key_Escape) {
        cancelCurrentTransform();
    } else if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        mTransformationFinishedBeforeMouseRelease = true;
        handleMouseRelease(mLastMouseEventPos);
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

void Canvas::keyPressEvent(QKeyEvent *event)
{
    if(mPreviewing) return;

    bool isGrabbingMouse = mCanvasWidget->mouseGrabber() == mCanvasWidget;
    if(isGrabbingMouse ? !handleKeyPressEventWhileMouseGrabbing(event) : true) {
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
        } else if(event->key() == Qt::Key_R && (isMovingPath() ||
                  mCurrentMode == MOVE_POINT) && !isGrabbingMouse) {
           mTransformationFinishedBeforeMouseRelease = false;
           mLastMouseEventPos = mCanvasWidget->mapFromGlobal(QCursor::pos());
           mLastPressPos = mLastMouseEventPos;
           mRotPivot->startRotating();
           mDoubleClick = false;
           mFirstMouseMove = true;

           grabMouseAndTrack();
        } else if(event->key() == Qt::Key_S && (isMovingPath() ||
                  mCurrentMode == MOVE_POINT) && !isGrabbingMouse) {
           mTransformationFinishedBeforeMouseRelease = false;
           mXOnlyTransform = false;
           mYOnlyTransform = false;

           mLastMouseEventPos = mCanvasWidget->mapFromGlobal(QCursor::pos());
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

            mLastMouseEventPos = mCanvasWidget->mapFromGlobal(QCursor::pos());
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
        } else if(event->key() == Qt::Key_P) {
            mPivotVisibleDuringPointEdit = !mPivotVisibleDuringPointEdit;
        }
        schedulePivotUpdate();
    }

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
    qreal widthScale = mCanvasWidget->width()/mVisibleWidth;
    qreal heightScale = mCanvasWidget->height()/mVisibleHeight;
    scale(qMin(heightScale, widthScale), QPointF(0., 0.));
    mVisibleHeight = mCombinedTransformMatrix.m22()*mHeight;
    mVisibleWidth = mCombinedTransformMatrix.m11()*mWidth;
    moveBy(QPointF( (mCanvasWidget->width() - mVisibleWidth)*0.5,
                    (mCanvasWidget->height() - mVisibleHeight)*0.5) );
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
}
