#include "canvaswidget.h"
#include "canvas.h"
#include <QComboBox>
#include "mainwindow.h"
#include "BoxesList/boxscrollwidgetvisiblepart.h"
#include "BoxesList/OptimalScrollArea/singlewidgetabstraction.h"
#include "paintcontroler.h"
#include "renderoutputwidget.h"
#include "Sound/soundcomposition.h"

CanvasWidget::CanvasWidget(QWidget *parent) : QWidget(parent) {
    //setAttribute(Qt::WA_OpaquePaintEvent, true);
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(500, 500);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setMouseTracking(true);

    mPaintControlerThread = new QThread(this);
    mPaintControler = new PaintControler();
    mPaintControler->moveToThread(mPaintControlerThread);
    connect(mPaintControler, SIGNAL(finishedUpdatingLastBox()),
            this, SLOT(sendNextBoxForUpdate()) );
    connect(this, SIGNAL(updateBoxPixmaps(BoundingBox*)),
            mPaintControler, SLOT(updateBoxPixmaps(BoundingBox*)) );

    mPaintControlerThread->start();

    mPreviewFPSTimer = new QTimer(this);
    mPreviewFPSTimer->setInterval(1000/24.);

    initializeAudio();
}

CanvasWidget::~CanvasWidget() {
    mPaintControlerThread->quit();
    mPaintControlerThread->wait();
}

Canvas *CanvasWidget::getCurrentCanvas() {
    return mCurrentCanvas;
}

SingleWidgetAbstraction* CanvasWidget::SWT_getAbstractionForWidget(
            ScrollWidgetVisiblePart *visiblePartWidget) {
    foreach(SingleWidgetAbstraction *abs, mSWT_allAbstractions) {
        if(abs->getParentVisiblePartWidget() == visiblePartWidget) {
            return abs;
        }
    }
    SingleWidgetAbstraction *abs = SWT_createAbstraction(visiblePartWidget);
    return abs;
}

void CanvasWidget::SWT_addChildrenAbstractions(
        SingleWidgetAbstraction *abstraction,
        ScrollWidgetVisiblePart *visiblePartWidget) {
    foreach(Canvas *child, mCanvasList) {
        abstraction->addChildAbstraction(
                    child->SWT_getAbstractionForWidget(visiblePartWidget));
    }
}

void CanvasWidget::setCurrentCanvas(const int &id) {
    if(id < 0 || id >= mCanvasList.count()) {
        setCurrentCanvas((Canvas*)NULL);
    } else {
        setCurrentCanvas(mCanvasList.at(id));
    }
}

void CanvasWidget::setCurrentCanvas(Canvas *canvas) {
    if(mCurrentCanvas != NULL) {
        mCurrentCanvas->setIsCurrentCanvas(false);
        disconnect(mPreviewFPSTimer, SIGNAL(timeout()),
                   mCurrentCanvas, SLOT(nextPreviewFrame()) );
    }
    mCurrentCanvas = canvas;
    if(mCurrentCanvas == NULL) {
        mCurrentSoundComposition = NULL;
    } else {
        mCurrentSoundComposition = mCurrentCanvas->getSoundComposition();
        connect(mPreviewFPSTimer, SIGNAL(timeout()),
                mCurrentCanvas, SLOT(nextPreviewFrame()) );

        mCurrentCanvas->setIsCurrentCanvas(true);

        setCanvasMode(mCurrentCanvas->getCurrentCanvasMode());

        emit changeFrameRange(getMinFrame(), getMaxFrame());
        emit changeCurrentFrame(getCurrentFrame());
    }
    SWT_scheduleWidgetsContentUpdateWithTarget(
                mCurrentCanvas,
                SWT_CurrentCanvas);
    updateDisplayedFillStrokeSettings();
    MainWindow::getInstance()->updateSettingsForCurrentCanvas();
    callUpdateSchedulers();
}

void CanvasWidget::addCanvasToList(Canvas *canvas) {
    canvas->incNumberPointers();
    mCanvasList << canvas;
    SWT_addChildAbstractionForTargetToAll(canvas);
}

void CanvasWidget::removeCanvas(const int &id) {
    Canvas *canvas = mCanvasList.takeAt(id);
    SWT_removeChildAbstractionForTargetFromAll(canvas);
    canvas->decNumberPointers();
    if(mCanvasList.isEmpty()) {
        setCurrentCanvas((Canvas*)NULL);
    } else if(id < mCanvasList.count()) {
        setCurrentCanvas(id);
    } else {
        setCurrentCanvas(id - 1);
    }
}

void CanvasWidget::setCanvasMode(const CanvasMode &mode) {
    if(hasNoCanvas()) {
        setCursor(QCursor(Qt::ArrowCursor) );
        return;
    }

    if(mode == MOVE_PATH) {
        setCursor(QCursor(Qt::ArrowCursor) );
    } else if(mode == MOVE_POINT) {
        setCursor(QCursor(QPixmap(":/cursors/cursor-node.xpm"), 0, 0) );
    } else if(mode == PICK_PATH_SETTINGS) {
        setCursor(QCursor(QPixmap(":/cursors/cursor_color_picker.png"), 2, 20) );
    } else if(mode == ADD_CIRCLE) {
        setCursor(QCursor(QPixmap(":/cursors/cursor-ellipse.xpm"), 4, 4) );
    } else if(mode == ADD_RECTANGLE || mode == ADD_PARTICLE_BOX) {
        setCursor(QCursor(QPixmap(":/cursors/cursor-rect.xpm"), 4, 4) );
    } else if(mode == ADD_TEXT) {
        setCursor(QCursor(QPixmap(":/cursors/cursor-text.xpm"), 4, 4) );
    } else {
        setCursor(QCursor(QPixmap(":/cursors/cursor-pen.xpm"), 4, 4) );
    }

    mCurrentCanvas->setCanvasMode(mode);
    MainWindow::getInstance()->updateCanvasModeButtonsChecked();
    callUpdateSchedulers();
}

void CanvasWidget::callUpdateSchedulers() {
    MainWindow::getInstance()->callUpdateSchedulers();
}

void CanvasWidget::setMovePathMode() {
    setCanvasMode(MOVE_PATH);
}

void CanvasWidget::setMovePointMode() {
    setCanvasMode(MOVE_POINT);
}

void CanvasWidget::setAddPointMode() {
    setCanvasMode(ADD_POINT);
}

void CanvasWidget::setRectangleMode() {
    setCanvasMode(ADD_RECTANGLE);
}

void CanvasWidget::setCircleMode() {
    setCanvasMode(ADD_CIRCLE);
}

void CanvasWidget::setTextMode() {
    setCanvasMode(ADD_TEXT);
}

void CanvasWidget::setParticleBoxMode() {
    setCanvasMode(ADD_PARTICLE_BOX);
}

void CanvasWidget::setParticleEmitterMode() {
    setCanvasMode(ADD_PARTICLE_EMITTER);
}

void CanvasWidget::addCanvasToListAndSetAsCurrent(Canvas *canvas) {
    addCanvasToList(canvas);
    setCurrentCanvas(canvas);
}

void CanvasWidget::renameCanvas(Canvas *canvas, const QString &newName) {
    canvas->setName(newName);
}

void CanvasWidget::renameCanvas(const int &id, const QString &newName) {
    renameCanvas(mCanvasList.at(id), newName);
}

bool CanvasWidget::hasNoCanvas() {
    return mCurrentCanvas == NULL;
}

void CanvasWidget::renameCurrentCanvas(const QString &newName) {
    if(mCurrentCanvas == NULL) return;
    renameCanvas(mCurrentCanvas, newName);
}

void CanvasWidget::paintEvent(QPaintEvent *) {
    if(mCurrentCanvas == NULL) return;
    QPainter p(this);
    mCurrentCanvas->paintEvent(&p);
    p.end();
}

void CanvasWidget::mousePressEvent(QMouseEvent *event) {
    if(mCurrentCanvas == NULL) return;
    mCurrentCanvas->mousePressEvent(event);
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event) {
    if(mCurrentCanvas == NULL) return;
    mCurrentCanvas->mouseReleaseEvent(event);
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event) {
    if(mCurrentCanvas == NULL) return;
    mCurrentCanvas->mouseMoveEvent(event);
}

void CanvasWidget::wheelEvent(QWheelEvent *event) {
    if(mCurrentCanvas == NULL) return;
    mCurrentCanvas->wheelEvent(event);
}

void CanvasWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    if(mCurrentCanvas == NULL) return;
    mCurrentCanvas->mouseDoubleClickEvent(event);
}

void CanvasWidget::keyPressEvent(QKeyEvent *event) {
    if(mCurrentCanvas == NULL) return;
    mCurrentCanvas->keyPressEvent(event);
}

bool CanvasWidget::processUnfilteredKeyEvent(QKeyEvent *event) {
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
    } else if(event->key() == Qt::Key_F7) {
        setCanvasMode(CanvasMode::ADD_PARTICLE_BOX);
    } else {
        return false;
    }
    return true;
}

bool CanvasWidget::processFilteredKeyEvent(QKeyEvent *event) {
    if(processUnfilteredKeyEvent(event)) return true;
    if(hasNoCanvas()) return false;
    return mCurrentCanvas->processFilteredKeyEvent(event);
}

void CanvasWidget::raiseAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->raiseSelectedBoxes();
    callUpdateSchedulers();
}

void CanvasWidget::lowerAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->lowerSelectedBoxes();
    callUpdateSchedulers();
}

void CanvasWidget::raiseToTopAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->raiseSelectedBoxesToTop();
    callUpdateSchedulers();
}

void CanvasWidget::lowerToBottomAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->lowerSelectedBoxesToBottom();
    callUpdateSchedulers();
}

void CanvasWidget::objectsToPathAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->convertSelectedBoxesToPath();
    callUpdateSchedulers();
}

void CanvasWidget::pathsUnionAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->selectedPathsUnion();
    callUpdateSchedulers();
}

void CanvasWidget::pathsDifferenceAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->selectedPathsDifference();
    callUpdateSchedulers();
}

void CanvasWidget::pathsIntersectionAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->selectedPathsIntersection();
    callUpdateSchedulers();
}

void CanvasWidget::pathsDivisionAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->selectedPathsDivision();
    callUpdateSchedulers();
}

void CanvasWidget::pathsExclusionAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->selectedPathsExclusion();
    callUpdateSchedulers();
}

void CanvasWidget::setFontFamilyAndStyle(QString family, QString style) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedFontFamilyAndStyle(family, style);
    callUpdateSchedulers();
}

void CanvasWidget::setFontSize(qreal size) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedFontSize(size);
    callUpdateSchedulers();
}

void CanvasWidget::connectPointsSlot() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->connectPoints();
    callUpdateSchedulers();
}

void CanvasWidget::disconnectPointsSlot() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->disconnectPoints();
    callUpdateSchedulers();
}

void CanvasWidget::mergePointsSlot() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->mergePoints();
    callUpdateSchedulers();
}

void CanvasWidget::makePointCtrlsSymmetric() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->makePointCtrlsSymmetric();
    callUpdateSchedulers();
}

void CanvasWidget::makePointCtrlsSmooth() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->makePointCtrlsSmooth();
    callUpdateSchedulers();
}

void CanvasWidget::makePointCtrlsCorner() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->makePointCtrlsCorner();
    callUpdateSchedulers();
}

void CanvasWidget::makeSegmentLine() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->makeSegmentLine();
    callUpdateSchedulers();
}

void CanvasWidget::makeSegmentCurve() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->makeSegmentCurve();
    callUpdateSchedulers();
}

void CanvasWidget::startSelectedStrokeWidthTransform() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->startSelectedStrokeWidthTransform();
    callUpdateSchedulers();
}

void CanvasWidget::startSelectedStrokeColorTransform() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->startSelectedStrokeColorTransform();
    callUpdateSchedulers();
}

void CanvasWidget::startSelectedFillColorTransform() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->startSelectedFillColorTransform();
    callUpdateSchedulers();
}

void CanvasWidget::fillPaintTypeChanged(const PaintType &paintType,
                                        const Color &color,
                                        Gradient *gradient) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedFillPaintType(paintType,
                                         color,
                                         gradient);
    callUpdateSchedulers();
}

void CanvasWidget::strokePaintTypeChanged(const PaintType &paintType,
                                          const Color &color,
                                          Gradient *gradient) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedStrokePaintType(paintType,
                                           color,
                                           gradient);
    callUpdateSchedulers();
}

void CanvasWidget::strokeCapStyleChanged(const Qt::PenCapStyle &capStyle) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedCapStyle(capStyle);
    callUpdateSchedulers();
}

void CanvasWidget::strokeJoinStyleChanged(const Qt::PenJoinStyle &joinStyle) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedJoinStyle(joinStyle);
    callUpdateSchedulers();
}

void CanvasWidget::strokeWidthChanged(const qreal &strokeWidth,
                                      const bool &finish) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedStrokeWidth(strokeWidth, finish);
    callUpdateSchedulers();
}

void CanvasWidget::applyPaintSettingToSelected(
        const PaintSetting &setting) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->applyPaintSettingToSelected(setting);
}

void CanvasWidget::setSelectedFillColorMode(const ColorMode &mode) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedFillColorMode(mode);
}

void CanvasWidget::setSelectedStrokeColorMode(const ColorMode &mode) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedStrokeColorMode(mode);
}

void CanvasWidget::updateAfterFrameChanged(const int &currentFrame) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->updateAfterFrameChanged(currentFrame);
}

void CanvasWidget::strokeFlatColorChanged(const Color &color,
                                      const bool &finish) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedStrokeFlatColor(color, finish);
    callUpdateSchedulers();
}

void CanvasWidget::fillFlatColorChanged(const Color &color,
                                        const bool &finish) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedFillFlatColor(color, finish);
    callUpdateSchedulers();
}

void CanvasWidget::fillGradientChanged(Gradient *gradient,
                                       const bool &finish) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedFillGradient(gradient, finish);
    callUpdateSchedulers();
}

void CanvasWidget::strokeGradientChanged(Gradient *gradient,
                                       const bool &finish) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedStrokeGradient(gradient, finish);
    callUpdateSchedulers();
}

void CanvasWidget::pickPathForSettings() {
    if(hasNoCanvas()) return;
    setCanvasMode(PICK_PATH_SETTINGS);
}

void CanvasWidget::updateDisplayedFillStrokeSettings() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setDisplayedFillStrokeSettingsFromLastSelected();
}

void CanvasWidget::setEffectsPaintEnabled(const bool &bT) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setEffectsPaintEnabled(bT);
    mCurrentCanvas->updateAllBoxes();
    callUpdateSchedulers();
}

void CanvasWidget::setResolutionPercent(const qreal &percent) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setResolutionPercent(percent);
    mCurrentCanvas->clearCache();
    mCurrentCanvas->updateAllBoxes();
    callUpdateSchedulers();
}

void CanvasWidget::updatePivotIfNeeded() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->updatePivotIfNeeded();
}

void CanvasWidget::schedulePivotUpdate() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->schedulePivotUpdate();
}

BoxesGroup *CanvasWidget::getCurrentGroup() {
    if(hasNoCanvas()) return NULL;
    return mCurrentCanvas->getCurrentBoxesGroup();
}

void CanvasWidget::renderOutput() {
    RenderOutputWidget *dialog = new RenderOutputWidget(this);
    connect(dialog, SIGNAL(render(QString)),
            this, SLOT(saveOutput(QString)));
    dialog->exec();
}

void CanvasWidget::playPreview() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->updateRenderRect();
    mBoxesUpdateFinishedFunction = &CanvasWidget::nextPlayPreviewFrame;
    mSavedCurrentFrame = getCurrentFrame();

    mRendering = true;
    mPreviewInterrupted = false;
    mCurrentRenderFrame = mSavedCurrentFrame;
    mCurrentCanvas->updateAfterFrameChanged(mSavedCurrentFrame);
    mCurrentCanvas->setPreviewing(true);
    mCurrentCanvas->updateAllBoxes();
    if(mNoBoxesAwaitUpdate) {
        nextPlayPreviewFrame();
    }
}

void CanvasWidget::addBoxAwaitingUpdate(BoundingBox *box) {
    if(mNoBoxesAwaitUpdate) {
        mNoBoxesAwaitUpdate = false;
        mLastUpdatedBox = box;
        mLastUpdatedBox->beforeUpdate();
        emit updateBoxPixmaps(box);
    } else {
        mBoxesAwaitingUpdate << box;
    }
}

void CanvasWidget::sendNextBoxForUpdate() {
    if(mLastUpdatedBox != NULL) {
        mLastUpdatedBox->afterUpdate();
//        mLastUpdatedBox->setAwaitingUpdate(false);
//        if(mLastUpdatedBox->shouldRedoUpdate()) {
//            mLastUpdatedBox->setRedoUpdateToFalse();
//            mLastUpdatedBox->awaitUpdate();
//        }
    }
    if(mBoxesAwaitingUpdate.isEmpty()) {
        mNoBoxesAwaitUpdate = true;
        mLastUpdatedBox = NULL;
        callUpdateSchedulers();
        if(mBoxesUpdateFinishedFunction != NULL) {
            (*this.*mBoxesUpdateFinishedFunction)();
        }
        //callUpdateSchedulers();
    } else {
        mLastUpdatedBox = mBoxesAwaitingUpdate.takeFirst();
        mLastUpdatedBox->beforeUpdate();
        emit updateBoxPixmaps(mLastUpdatedBox);
    }
}

void CanvasWidget::interruptPreview() {
    mPreviewInterrupted = true;
    mCurrentCanvas->clearPreview();
}

void CanvasWidget::stopPreview() {
    if(!mRendering) {
        mCurrentRenderFrame = getMaxFrame();
        mPreviewFPSTimer->stop();
        stopAudio();
        repaint();
        MainWindow::getInstance()->previewFinished();
    }
}

void CanvasWidget::nextPlayPreviewFrame() {
    mCurrentCanvas->renderCurrentFrameToPreview();
    if(mCurrentRenderFrame >= getMaxFrame() || mPreviewInterrupted) {
        mRendering = false;
        emit changeCurrentFrame(mSavedCurrentFrame);
        mBoxesUpdateFinishedFunction = NULL;
            mCurrentCanvas->playPreview();
            mCurrentSoundComposition->generateData(mSavedCurrentFrame,
                                                   mCurrentRenderFrame,
                                                   24);
            startAudio();
            mPreviewFPSTimer->start();
    } else {
        mCurrentRenderFrame++;
        emit changeCurrentFrame(mCurrentRenderFrame);
        if(mNoBoxesAwaitUpdate) {
            nextPlayPreviewFrame();
        }
    }
}

void CanvasWidget::nextSaveOutputFrame() {
    mCurrentCanvas->renderCurrentFrameToOutput(mOutputString);
    if(mCurrentRenderFrame >= getMaxFrame()) {
        emit changeCurrentFrame(mSavedCurrentFrame);
        mBoxesUpdateFinishedFunction = NULL;
    } else {
        mCurrentRenderFrame++;
        emit changeCurrentFrame(mCurrentRenderFrame);
        if(mNoBoxesAwaitUpdate) {
            nextSaveOutputFrame();
        }
    }
}

void CanvasWidget::loadCanvasesFromSql() {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM canvas";
    if(query.exec(queryStr)) {
        while(query.next()) {
            int idWidth = query.record().indexOf("width");
            int idHeight = query.record().indexOf("height");
            int idFrameCount = query.record().indexOf("framecount");
            int idBoundingBoxId = query.record().indexOf("boundingboxid");

            int width = query.value(idWidth).toInt();
            int height = query.value(idHeight).toInt();
            int frameCount = query.value(idFrameCount).toInt();
            int boundingBoxId = query.value(idBoundingBoxId).toInt();

            Canvas *canvas =
                    new Canvas(MainWindow::getInstance()->getFillStrokeSettings(),
                               this,
                               width, height,
                               frameCount);
            canvas->prp_loadFromSql(boundingBoxId);
            MainWindow::getInstance()->addCanvas(canvas);
        }
    } else {
        qDebug() << "Could not load canvases";
    }
}

void CanvasWidget::saveCanvasesFromSql(QSqlQuery *query) {
    foreach(Canvas *canvas, mCanvasList) {
        canvas->prp_saveToSql(query);
    }
}

void CanvasWidget::saveOutput(QString renderDest) {
    mOutputString = renderDest;
    mBoxesUpdateFinishedFunction = &CanvasWidget::nextSaveOutputFrame;
    mSavedCurrentFrame = getCurrentFrame();

    mCurrentRenderFrame = getMinFrame();
    emit changeCurrentFrame(getMinFrame());
    if(mNoBoxesAwaitUpdate) {
        nextSaveOutputFrame();
    }
}

void CanvasWidget::clearAll() {
    SWT_clearAll();
    foreach(Canvas *canvas, mCanvasList) {
        canvas->decNumberPointers();
    }
    mCanvasList.clear();
    setCurrentCanvas((Canvas*)NULL);
}

void CanvasWidget::saveSelectedToSql(QSqlQuery *query) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->saveSelectedToSql(query);
}

void CanvasWidget::createLinkToFileWithPath(const QString &path) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->createLinkToFileWithPath(path);
}

void CanvasWidget::createAnimationBoxForPaths(
        const QStringList &importPaths) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->createAnimationBoxForPaths(importPaths);
}

void CanvasWidget::createVideoForPath(const QString &path) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->createVideoForPath(path);
}

void CanvasWidget::createImageForPath(const QString &path) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->createImageBox(path);
}

void CanvasWidget::createSoundForPath(const QString &path) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->createSoundForPath(path);
}

void CanvasWidget::saveToSql(QSqlQuery *query) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->prp_saveToSql(query);
}

int CanvasWidget::getCurrentFrame() {
    if(hasNoCanvas()) return 0;
    return mCurrentCanvas->getCurrentFrame();
}

int CanvasWidget::getMaxFrame() {
    if(hasNoCanvas()) return 0;
    return mCurrentCanvas->getMaxFrame();
}

int CanvasWidget::getMinFrame() {
    if(hasNoCanvas()) return 0;
    return mCurrentCanvas->getMinFrame();
}

const int BufferSize = 32768;

void CanvasWidget::initializeAudio()
{
    mAudioBuffer = QByteArray(BufferSize, 0);
    connect(mPreviewFPSTimer, SIGNAL(timeout()),
            this, SLOT(pushTimerExpired()));

    mAudioDevice = QAudioDeviceInfo::defaultOutputDevice();
    mAudioFormat.setSampleRate(SAMPLERATE);
    mAudioFormat.setChannelCount(1);
    mAudioFormat.setSampleSize(32);
    mAudioFormat.setCodec("audio/pcm");
    mAudioFormat.setByteOrder(QAudioFormat::LittleEndian);
    mAudioFormat.setSampleType(QAudioFormat::Float);

    QAudioDeviceInfo info(mAudioDevice);
    if (!info.isFormatSupported(mAudioFormat)) {
        qWarning() << "Default format not supported - trying to use nearest";
        mAudioFormat = info.nearestFormat(mAudioFormat);
    }

    mAudioOutput = new QAudioOutput(mAudioDevice, mAudioFormat, this);
}

void CanvasWidget::startAudio() {
    mCurrentSoundComposition->start();
    mAudioIOOutput = mAudioOutput->start();
}

void CanvasWidget::stopAudio() {
    //mAudioOutput->suspend();
    //mCurrentSoundComposition->stop();
    mAudioIOOutput = NULL;
    mAudioOutput->stop();
    mAudioOutput->reset();
    mCurrentSoundComposition->stop();
}

void CanvasWidget::volumeChanged(int value) {
    if(mAudioOutput) {
        mAudioOutput->setVolume(qreal(value/100.));
    }
}

void CanvasWidget::pushTimerExpired() {
    if(mAudioOutput && mAudioOutput->state() != QAudio::StoppedState) {
        int chunks = mAudioOutput->bytesFree()/mAudioOutput->periodSize();
        while(chunks) {
           const qint64 len = mCurrentSoundComposition->read(
                                                mAudioBuffer.data(),
                                                mAudioOutput->periodSize());
           if(len) {
               mAudioIOOutput->write(mAudioBuffer.data(), len);
           }
           if(len != mAudioOutput->periodSize()) {
               break;
           }
           --chunks;
        }
    }
}
