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

Canvas::Canvas(FillStrokeSettingsWidget *fillStrokeSettings,
               CanvasWidget *canvasWidget,
               int canvasWidth, int canvasHeight) :
    BoxesGroup(fillStrokeSettings)
{
    mEffectsPaintEnabled = true;
    mResolutionPercent = 1.;

    mWidth = canvasWidth;
    mHeight = canvasHeight;
    mVisibleWidth = mWidth;
    mVisibleHeight = mHeight;
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
    InternalLinkCanvas *linkGroup =
                        new InternalLinkCanvas(this, parent);
    foreach(BoundingBox *box, mChildBoxes) {
        box->createSameTransformationLink(linkGroup);
    }
    return linkGroup;
}

void Canvas::setHighQualityPaint(const bool &bT) {
    mHighQualityPaint = bT;
}

void Canvas::updateAllBoxes() {
    BoxesGroup::updateAllBoxes();
    callUpdateSchedulers();
}

bool Canvas::highQualityPaint() {
    return mHighQualityPaint;
}

void Canvas::setEffectsPaintEnabled(const bool &bT) {
    mEffectsPaintEnabled = bT;
}

bool Canvas::effectsPaintEnabled()
{
    return mEffectsPaintEnabled;
}

qreal Canvas::getResolutionPercent()
{
    return Canvas::mResolutionPercent;
}

void Canvas::setResolutionPercent(const qreal &percent)
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
    return false;
}

bool Canvas::processFilteredKeyEvent(QKeyEvent *event) {
    if(processUnfilteredKeyEvent(event)) return true;
    if(!mCanvasWidget->hasFocus()) return false;
    if(isCtrlPressed() && event->key() == Qt::Key_G) {
        if(isShiftPressed()) {
            mCurrentBoxesGroup->ungroupSelected();
        } else {
            BoxesGroup *newGroup = mCurrentBoxesGroup->groupSelectedBoxes();
//            if(newGroup != NULL) {
//                setCurrentBoxesGroup(newGroup);
//            }
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
#include "BoxesList/boxscrollwidget.h"
void Canvas::setCurrentBoxesGroup(BoxesGroup *group) {
    mCurrentBoxesGroup->setIsCurrentGroup(false);
    clearAllPathsSelection();
    clearAllPointsSelection();
    mCurrentBoxesGroup = group;
    group->setIsCurrentGroup(true);

    mMainWindow->getObjectSettingsList()->setMainTarget(mCurrentBoxesGroup);
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
    foreach(BoundingBox *box, mChildBoxes) {
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

void Canvas::createImageBox(const QString &path) {
    mCurrentBoxesGroup->addChild(new ImageBox(mCurrentBoxesGroup, path));
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
        qreal reversedRes = 1./mResolutionPercent;
        p->translate(getRenderRect().topLeft());
        p->scale(reversedRes, reversedRes);
        if(mCurrentPreviewImg != NULL) {
            p->drawImage(QPointF(0., 0.), *mCurrentPreviewImg);
        }
        p->restore();
    } else {
        p->fillRect(0, 0, mCanvasWidget->width() + 1, mCanvasWidget->height() + 1, QColor(75, 75, 75));
        p->fillRect(viewRect, Qt::white);

        foreach(BoundingBox *box, mChildBoxes){
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

void Canvas::setPreviewing(bool bT) {
    mPreviewing = bT;
    mCanvasWidget->setAttribute(Qt::WA_OpaquePaintEvent, !bT);
}

void Canvas::playPreview()
{
    if(mPreviewFrames.isEmpty() ) return;
    mCurrentPreviewFrameId = 0;
    mCurrentPreviewImg = mPreviewFrames.first();
    setPreviewing(true);
    mCanvasWidget->repaint();

    mPreviewFPSTimer->start();
}

void Canvas::clearPreview() {
    if(!mPreviewFrames.isEmpty()) {
        mPreviewFPSTimer->stop();
        setPreviewing(false);
        mCurrentPreviewImg = NULL;
        for(int i = 0; i < mPreviewFrames.length(); i++) {
            delete mPreviewFrames.at(i);
        }
        mPreviewFrames.clear();
        mMainWindow->previewFinished();
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

void Canvas::pathsUnionAction() {
    mCurrentBoxesGroup->selectedPathsUnion();
}

void Canvas::pathsDifferenceAction() {
    mCurrentBoxesGroup->selectedPathsDifference();
}

void Canvas::pathsIntersectionAction() {
    mCurrentBoxesGroup->selectedPathsIntersection();
}

void Canvas::pathsDivisionAction() {
    mCurrentBoxesGroup->selectedPathsDivision();
}

void Canvas::pathsExclusionAction() {
    mCurrentBoxesGroup->selectedPathsExclusion();
}

QRectF Canvas::getRenderRect() {
    return mRenderRect;
    QRectF rectT = mRenderRect;
    rectT.setSize(mRenderRect.size()*mResolutionPercent);
    rectT.moveTo(rectT.topLeft()*mResolutionPercent);
    return rectT;
}

void Canvas::updateRenderRect() {
    mCanvasRect = QRectF(qMax(mCombinedTransformMatrix.dx(),
                              mCombinedTransformMatrix.dx()*
                              mResolutionPercent),
                         qMax(mCombinedTransformMatrix.dy(),
                              mCombinedTransformMatrix.dy()*
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
                               QImage::Format_ARGB32);
    image->fill(Qt::transparent);
    renderCurrentFrameToQImage(image);
    mPreviewFrames << image;
    mCurrentPreviewImg = image;
}

void Canvas::renderCurrentFrameToOutput(QString renderDest) {
    QImage *image = new QImage(mWidth, mHeight,
                               QImage::Format_ARGB32);
    image->fill(Qt::transparent);
    renderFinalCurrentFrameToQImage(image);
    image->save(renderDest + QString::number(getCurrentFrame()) + ".png");
    delete image;
}

void Canvas::drawPreviewPixmap(QPainter *p) {
    if(mVisible) {
        p->save();
        //p->setTransform(QTransform(mCombinedTransformMatrix.inverted()), true);
        foreach(BoundingBox *box, mChildBoxes){
            box->drawPreviewPixmap(p);
        }

        p->restore();
    }
}

void Canvas::renderFinal(QPainter *p) {
    if(mVisible) {
        p->save();

        foreach(BoundingBox *box, mChildBoxes){
            box->renderFinal(p);
        }

        p->restore();
    }
}
#include "Boxes/animationbox.h"
void Canvas::createAnimationBoxForPaths(const QStringList &paths) {
    new AnimationBox(mCurrentBoxesGroup, paths);
}
#include "Boxes/linkbox.h"
void Canvas::createLinkToFileWithPath(const QString &path) {
    new ExternalLinkBox(path, mCurrentBoxesGroup);
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

    foreach(BoundingBox *box, mChildBoxes) {
        box->decNumberPointers();
    }
    mChildBoxes.clear();

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

void Canvas::makeSegmentLine() {
    mCurrentBoxesGroup->makeSelectedPointsSegmentsLines();
}

void Canvas::makeSegmentCurve() {
    mCurrentBoxesGroup->makeSelectedPointsSegmentsCurves();
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

    clearAllPointsSelection();
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
    mCanvasWidget->setMouseTracking(true);
    mCanvasWidget->grabMouse();
}

void Canvas::setFontFamilyAndStyle(QString family, QString style) {
    mCurrentBoxesGroup->setSelectedFontFamilyAndStyle(family, style);
}

void Canvas::setFontSize(qreal size) {
    mCurrentBoxesGroup->setSelectedFontSize(size);
}

void Canvas::releaseMouseAndDontTrack() {
    mIsMouseGrabbing = false;
    mCanvasWidget->setMouseTracking(false);
    mCanvasWidget->releaseMouse();
}

void Canvas::groupSelectedBoxesAction() {
    BoxesGroup *newGroup = mCurrentBoxesGroup->groupSelectedBoxes();
//    if(newGroup != NULL) {
//        setCurrentBoxesGroup(newGroup);
//    }
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
    foreach(BoundingBox *box, mChildBoxes) {
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

bool Canvas::SWT_satisfiesRule(const SWT_RulesCollection &rules,
                               const bool &parentSatisfies) {
    Q_UNUSED(parentSatisfies);
    const SWT_Rule &rule = rules.rule;
    const bool &alwaysShowChildren = rules.alwaysShowChildren;
    if(alwaysShowChildren) {
        return false;
    } else {
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
