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

Canvas::Canvas(FillStrokeSettingsWidget *fillStrokeSettings,
               MainWindow *parent) :
    QWidget(parent),
    BoxesGroup(fillStrokeSettings, parent)
{
    incNumberPointers();

    mPreviewFPSTimer = new QTimer(this);
    mPreviewFPSTimer->setInterval(1000/24.);
    connect(mPreviewFPSTimer, SIGNAL(timeout()),
            this, SLOT(nextPreviewFrame()) );

    connect(mFillStrokeSettingsWidget, SIGNAL(fillSettingsChanged(PaintSettings, bool)),
            this, SLOT(fillSettingsChanged(PaintSettings, bool)) );
    connect(mFillStrokeSettingsWidget, SIGNAL(strokeSettingsChanged(StrokeSettings, bool)),
            this, SLOT(strokeSettingsChanged(StrokeSettings, bool)) );

    connect(mFillStrokeSettingsWidget, SIGNAL(startFillSettingsTransform()),
            this, SLOT(startFillSettingsTransform()) );
    connect(mFillStrokeSettingsWidget, SIGNAL(startStrokeSettingsTransform()),
            this, SLOT(startStrokeSettingsTransform()) );
    connect(mFillStrokeSettingsWidget, SIGNAL(finishFillSettingsTransform()),
            this, SLOT(finishFillSettingsTransform()) );
    connect(mFillStrokeSettingsWidget, SIGNAL(finishStrokeSettingsTransform()),
            this, SLOT(finishStrokeSettingsTransform()) );

    mCurrentBoxesGroup = this;
    mIsCurrentGroup = true;
    setFocusPolicy(Qt::StrongFocus);
    mRotPivot = new PathPivot(this);
    mRotPivot->hide();
    setMinimumSize(500, 500);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    setCanvasMode(MOVE_PATH);
}

Canvas::~Canvas()
{
    delete mRotPivot;
}

QRectF Canvas::getBoundingRect()
{
    QPointF absPos = getAbsolutePos();
    return QRectF(absPos, absPos + QPointF(mVisibleWidth, mVisibleHeight));
}

void Canvas::pickPathForSettings() {
    setCanvasMode(PICK_PATH_SETTINGS);
}

void Canvas::scale(qreal scaleXBy, qreal scaleYBy, QPointF absOrigin)
{
    QPointF transPoint = -getCombinedTransform().inverted().map(absOrigin);

    mCombinedTransformMatrix.translate(-transPoint.x(), -transPoint.y());
    mCombinedTransformMatrix.scale(scaleXBy, scaleYBy);
    mCombinedTransformMatrix.translate(transPoint.x(), transPoint.y());
    updateAfterCombinedTransformationChanged();
}

void Canvas::scale(qreal scaleBy, QPointF absOrigin)
{
    scale(scaleBy, scaleBy, absOrigin);
}

bool Canvas::processFilteredKeyEvent(QKeyEvent *event) {
    if(!hasFocus()) return false;
    if(event->key() == Qt::Key_F1) {
        setCanvasMode(CanvasMode::MOVE_PATH);
    } else if(event->key() == Qt::Key_F2) {
        setCanvasMode(CanvasMode::MOVE_POINT);
    } else if(event->key() == Qt::Key_F3) {
        setCanvasMode(CanvasMode::ADD_POINT);
    } else if(isCtrlPressed() && event->key() == Qt::Key_G) {
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
        mCurrentBoxesGroup->moveSelectedBoxesUp();
    } else if(event->key() == Qt::Key_PageDown) {
        mCurrentBoxesGroup->moveSelectedBoxesDown();
    } else if(event->key() == Qt::Key_End) {
        mCurrentBoxesGroup->bringSelectedBoxesToEnd();
    } else if(event->key() == Qt::Key_Home) {
        mCurrentBoxesGroup->bringSelectedBoxesToFront();
    } else {
        return false;
    }
    clearAllPointsSelection();

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
    BoxesGroup *container = mCurrentBoxesGroup->loadChildrenFromSql("NULL",
                                                                    loadInBox);
    container->attachToBoneFromSqlZId();
}

void Canvas::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    p.setRenderHint(QPainter::Antialiasing);

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

        p.drawImage(absPos, *mCurrentPreviewImg);
    } else {
        p.fillRect(0, 0, width() + 1, height() + 1, QColor(75, 75, 75));
        p.fillRect(viewRect, Qt::white);

        foreach(BoundingBox *box, mChildren){
            box->draw(&p);
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

void Canvas::renderCurrentFrameToQImage(QImage *frame)
{
    QPainter p(frame);
    p.setRenderHint(QPainter::Antialiasing);

    BoxesGroup::render(&p);

    p.end();
}

bool Canvas::isMovingPath() {
    return mCurrentMode == CanvasMode::MOVE_PATH;
}

qreal Canvas::getCurrentCanvasScale()
{
    return mCombinedTransformMatrix.m11();
}

QSize Canvas::getCanvasSize()
{
    return QSize(mWidth, mHeight);
}

void Canvas::playPreview()
{
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

void Canvas::renderCurrentFrameToPreview()
{
    QImage *image = new QImage(mVisibleWidth, mVisibleHeight,
                               QImage::Format_ARGB32);
    image->fill(Qt::transparent);
    renderCurrentFrameToQImage(image);
    mPreviewFrames << image;
}

QMatrix Canvas::getCombinedRenderTransform()
{
    QMatrix matrix;
    matrix.scale(mCombinedTransformMatrix.m11(),
                 mCombinedTransformMatrix.m22() );
    return matrix;
}

void Canvas::schedulePivotUpdate()
{
    mPivotUpdateNeeded = true;
}

void Canvas::clearAll() {
    setCurrentBoxesGroup(this);
    BoxesGroup::clearAll();
    resetTransormation();
}

void Canvas::updatePivotIfNeeded()
{
    if(mPivotUpdateNeeded) {
        mPivotUpdateNeeded = false;
        updatePivot();
    }
}

void Canvas::fillSettingsChanged(PaintSettings fillSettings, bool saveUndoRedo)
{
    mCurrentBoxesGroup->setSelectedFillSettings(fillSettings, saveUndoRedo);
    callUpdateSchedulers();
}

void Canvas::strokeSettingsChanged(StrokeSettings strokeSettings, bool saveUndoRedo)
{
    mCurrentBoxesGroup->setSelectedStrokeSettings(strokeSettings, saveUndoRedo);
    callUpdateSchedulers();
}

void Canvas::startStrokeSettingsTransform()
{
    mCurrentBoxesGroup->startSelectedStrokeTransform();
}

void Canvas::startFillSettingsTransform()
{
    mCurrentBoxesGroup->startSelectedFillTransform();
}

void Canvas::finishStrokeSettingsTransform()
{
    mCurrentBoxesGroup->finishSelectedStrokeTransform();
}

void Canvas::finishFillSettingsTransform()
{
    mCurrentBoxesGroup->finishSelectedFillTransform();
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

void Canvas::scheduleRepaint()
{
    if(mRepaintNeeded) {
        return;
    }
    mRepaintNeeded = true;
}

void Canvas::repaintIfNeeded()
{
    if(mRepaintNeeded) {
        repaint();
        mRepaintNeeded = false;
    }
}

void Canvas::moveSecondSelectionPoint(QPointF pos) {
    mSelectionRect.setBottomRight(pos);
    scheduleRepaint();
}

void Canvas::startSelectionAtPoint(QPointF pos) {
    mSelectionRect.setTopLeft(pos);
    mSelectionRect.setBottomRight(pos);
    scheduleRepaint();
}

void Canvas::updatePivot() {
    if(mCurrentMode != MOVE_PATH) {
        return;
    }
    if(mCurrentBoxesGroup->isSelectionEmpty() ) {
        mRotPivot->hide();
    } else {
        mRotPivot->show();
        mRotPivot->setAbsolutePos(mCurrentBoxesGroup->getSelectedPivotPos(), false);
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
    } else {
        setCursor(QCursor(QPixmap("pixmaps/cursor-pen.xpm"), 4, 4) );
    }
    if(mCurrentMode == MOVE_PATH) {
        schedulePivotUpdate();
    }
    scheduleRepaint();
}

void Canvas::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_0) {
        fitCanvasToSize();
    } else if(event->key() == Qt::Key_1) {
        resetTransormation();
    } else if(event->key() == Qt::Key_Delete) {
       if(mCurrentMode == MOVE_POINT) {
           mCurrentBoxesGroup->removeSelectedPointsAndClearList();
       } else if(mCurrentMode == MOVE_PATH) {
           mCurrentBoxesGroup->removeSelectedBoxesAndClearList();
       }
    } else if(isCtrlPressed() && event->key() == Qt::Key_G) {
       if(isShiftPressed()) {
           mCurrentBoxesGroup->ungroupSelected();
       } else {
           BoxesGroup *newGroup = mCurrentBoxesGroup->groupSelectedBoxes();
           if(newGroup != NULL) {
               setCurrentBoxesGroup(newGroup);
           }
       }

    } else if(event->key() == Qt::Key_PageUp) {
       mCurrentBoxesGroup->moveSelectedBoxesUp();
    } else if(event->key() == Qt::Key_PageDown) {
       mCurrentBoxesGroup->moveSelectedBoxesDown();
    } else if(event->key() == Qt::Key_End) {
       mCurrentBoxesGroup->bringSelectedBoxesToEnd();
    } else if(event->key() == Qt::Key_Home) {
       mCurrentBoxesGroup->bringSelectedBoxesToFront();
    } else if(event->key() == Qt::Key_B) {
        if(isAltPressed()) {
            mCurrentBoxesGroup->detachFromBone(mCurrentMode);
        } else if(isCtrlPressed()) {
            Bone *bone = mCurrentBoxesGroup->getBoneAt(
                        mapFromGlobal(QCursor::pos()));
            if(bone == NULL) return;
            mCurrentBoxesGroup->attachToBone(bone, mCurrentMode);
        } else {
            startNewUndoRedoSet();
            Bone *newBone = new Bone(mCurrentBoxesGroup);
            newBone->addCircle(mapFromGlobal(QCursor::pos()) );
            finishUndoRedoSet();
            setCanvasMode(MOVE_PATH);
            mCurrentBoxesGroup->clearBoxesSelection();
            mCurrentBoxesGroup->addBoxToSelection(newBone);
            setCanvasMode(MOVE_POINT);
        }
    } else if(event->key() == Qt::Key_R && isMovingPath()) {
       mLastMouseEventPos = mapFromGlobal(QCursor::pos());
       mLastPressPos = mLastMouseEventPos;
       mRotPivot->startRotating();
       mIsMouseGrabbing = true;
       mDoubleClick = false;
       mFirstMouseMove = true;
       setMouseTracking(true);
    } else if(event->key() == Qt::Key_S && isMovingPath()) {
       mLastMouseEventPos = mapFromGlobal(QCursor::pos());
       mLastPressPos = mLastMouseEventPos;
       mRotPivot->startScaling();
       mIsMouseGrabbing = true;
       mDoubleClick = false;
       mFirstMouseMove = true;
       setMouseTracking(true);
    } else if(event->key() == Qt::Key_G && (isMovingPath() ||
                                            mCurrentMode == MOVE_POINT)) {
        mLastMouseEventPos = mapFromGlobal(QCursor::pos());
        mLastPressPos = mLastMouseEventPos;
        mIsMouseGrabbing = true;
        mDoubleClick = false;
        mFirstMouseMove = true;
        setMouseTracking(true);
     } else if(event->key() == Qt::Key_A && isCtrlPressed()) {
       if(isShiftPressed()) {
           mCurrentBoxesGroup->deselectAllBoxes();
       } else {
           mCurrentBoxesGroup->selectAllBoxes();
       }
    }
    //schedulePivotUpdate();

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
    scheduleRepaint();
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
    scheduleRepaint();
}

void Canvas::moveBy(QPointF trans)
{
    trans = getCombinedTransform().inverted().map(trans) -
            getCombinedTransform().inverted().map(QPointF(0, 0));

    mCombinedTransformMatrix.translate(trans.x(), trans.y());
    updateCombinedTransform();
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
            setCurrentEndPoint(mCurrentEndPoint->addPoint(
                                new PathPoint(point->getAbsolutePos(),
                                              startCtrlPtPos,
                                              endCtrlPtPos,
                                              pathDest)) );
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
            setCurrentEndPoint(mCurrentEndPoint->addPoint(
                                new PathPoint(point->getAbsolutePos(),
                                              startCtrlPtPos,
                                              endCtrlPtPos,
                                              pathDest)) );
            point = point->getPreviousPoint();
        }
    }
    mCurrentBoxesGroup->removeChild(pathSrc);

    finishUndoRedoSet();
}
