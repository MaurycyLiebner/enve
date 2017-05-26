#ifndef CANVASWINDOW_H
#define CANVASWINDOW_H

#include <QWidget>
#include "glwindow.h"
#include "BoxesList/OptimalScrollArea/singlewidgettarget.h"
class Canvas;
enum ColorMode : short;
enum CanvasMode : short;
class Color;
class Gradient;
class BoundingBox;
class BoxesGroup;
class PaintControler;
class SoundComposition;
class PaintSetting;

#include <QSqlQuery>
#include <QAudioOutput>

class CanvasWindow : public GLWindow, public SingleWidgetTarget {
    Q_OBJECT
public:
    explicit CanvasWindow(QWidget *parent);
    ~CanvasWindow();

    Canvas *getCurrentCanvas();
    const QList<Canvas*> &getCanvasList() {
        return mCanvasList;
    }

    void setCurrentCanvas(Canvas *canvas);
    void addCanvasToList(Canvas *canvas);
    void removeCanvas(const int &id);
    void addCanvasToListAndSetAsCurrent(Canvas *canvas);
    void renameCanvas(Canvas *canvas, const QString &newName);
    void renameCanvas(const int &id, const QString &newName);
    bool hasNoCanvas();
    void setCanvasMode(const CanvasMode &mode);

    void callUpdateSchedulers();
    bool processUnfilteredKeyEvent(QKeyEvent *event);
    bool processFilteredKeyEvent(QKeyEvent *event);

    void startSelectedStrokeWidthTransform();
    void startSelectedStrokeColorTransform();
    void startSelectedFillColorTransform();

    void strokeCapStyleChanged(const Qt::PenCapStyle &capStyle);
    void strokeJoinStyleChanged(const Qt::PenJoinStyle &joinStyle);
    void strokeWidthChanged(const qreal &strokeWidth, const bool &finish);
    void strokeFlatColorChanged(const Color &color, const bool &finish);
    void fillFlatColorChanged(const Color &color, const bool &finish);
    void fillGradientChanged(Gradient *gradient, const bool &finish);
    void strokeGradientChanged(Gradient *gradient, const bool &finish);
    void pickPathForSettings();
    void updateDisplayedFillStrokeSettings();

    void setResolutionFraction(const qreal &percent);
    void updatePivotIfNeeded();
    void schedulePivotUpdate();

    BoxesGroup *getCurrentGroup();
    SingleWidgetAbstraction *SWT_getAbstractionForWidget(
            ScrollWidgetVisiblePart *visiblePartWidget);
    void applyPaintSettingToSelected(const PaintSetting &setting);
    void setSelectedFillColorMode(const ColorMode &mode);
    void setSelectedStrokeColorMode(const ColorMode &mode);

    void updateAfterFrameChanged(const int &currentFrame);
    void clearAll();
    void createAnimationBoxForPaths(const QStringList &importPaths);
    void createLinkToFileWithPath(const QString &path);
    void saveToSql(QSqlQuery *query);
    void createVideoForPath(const QString &path);
    int getCurrentFrame();
    int getMaxFrame();
    void addBoxAwaitingUpdate(BoundingBox *box);
    void SWT_addChildrenAbstractions(SingleWidgetAbstraction *abstraction,
                                     ScrollWidgetVisiblePart *visiblePartWidget);
    void createImageForPath(const QString &path);
    void createSoundForPath(const QString &path);
    void loadCanvasesFromSql();
    void saveCanvasesFromSql(QSqlQuery *query);
    void updateHoveredElements();

    void switchLocalPivot();
    bool getLocalPivot();
    void setLocalPivot(const bool &bT);

    void importFile(const QString &path);

    QWidget *getWidgetContainer() {
        return mWidgetContainer;
    }

    void grabMouse() {
        //mWidgetContainer->grabMouse();
    }

//    bool hasFocus() {
//        return mWidgetContainer->hasFocus();
//    }

    void repaint() {
        mWidgetContainer->update();
    }

    QRect rect() {
        return mWidgetContainer->rect();
    }

    void releaseMouse() {
//        QWidget *grabber = mWidgetContainer->mouseGrabber();
//        mWidgetContainer->releaseMouse();
//        grabber = mWidgetContainer->mouseGrabber();
    }

    bool isMouseGrabber() {
        return mWidgetContainer->mouseGrabber() == mWidgetContainer;
    }

    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
protected:
    QWidget *mWidgetContainer;
    void setRendering(const bool &bT);
    void setPreviewing(const bool &bT);

    QTimer *mPreviewFPSTimer = NULL;
    QThread *mPaintControlerThread;
    PaintControler *mPaintControler;

    qreal mSavedResolutionFraction = 100.;
    int mSavedCurrentFrame = 0;
    bool mPreviewing = false;
    bool mRendering = false;
    bool mNoBoxesAwaitUpdate = true;
    bool mCancelLastBoxUpdate = false;
    BoundingBox *mLastUpdatedBox = NULL;
    QList<BoundingBox*> mBoxesAwaitingUpdate;

    QString mOutputString;
    int mCurrentRenderFrame;

    void (CanvasWindow::*mBoxesUpdateFinishedFunction)(void) = NULL;
    Canvas *mCurrentCanvas = NULL;
    QList<Canvas*> mCanvasList;

    //void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);
    void nextCurrentRenderFrame();


    // AUDIO
    void initializeAudio();
    void startAudio();
    void stopAudio();
    void volumeChanged(int value);

    QAudioDeviceInfo mAudioDevice;
    SoundComposition *mCurrentSoundComposition;
    QAudioOutput *mAudioOutput;
    QIODevice *mAudioIOOutput; // not owned
    QAudioFormat mAudioFormat;

    QByteArray mAudioBuffer;
    // AUDIO

    void qRender(QPainter *p);
    void render(SkCanvas *canvas);
signals:
    void updateBoxPixmaps(BoundingBox*);
    void changeCurrentFrame(int);
    void changeFrameRange(int, int);
public slots:
    void setMovePathMode();
    void setMovePointMode();
    void setAddPointMode();
    void setRectangleMode();
    void setCircleMode();
    void setTextMode();
    void setParticleBoxMode();
    void setParticleEmitterMode();

    void raiseAction();
    void lowerAction();
    void raiseToTopAction();
    void lowerToBottomAction();

    void objectsToPathAction();
    void strokeToPathAction();

    void setFontFamilyAndStyle(QString family, QString style);
    void setFontSize(qreal size);

    void connectPointsSlot();
    void disconnectPointsSlot();
    void mergePointsSlot();

    void makePointCtrlsSymmetric();
    void makePointCtrlsSmooth();
    void makePointCtrlsCorner();

    void makeSegmentLine();
    void makeSegmentCurve();

    void pathsUnionAction();
    void pathsDifferenceAction();
    void pathsIntersectionAction();
    void pathsDivisionAction();
    void pathsExclusionAction();

    void renameCurrentCanvas(const QString &newName);
    void setCurrentCanvas(const int &id);

    void setEffectsPaintEnabled(const bool &bT);

    void interruptPreview();
    void outOfMemory();
    void interruptRendering();

    void playPreview();
    void stopPreview();
    void pausePreview();
    void resumePreview();
    void renderPreview();
    void renderOutput();

    void importFile();
private slots:
    void sendNextBoxForUpdate();
    void nextSaveOutputFrame();
    void nextPreviewRenderFrame();
    void saveOutput(const QString &renderDest,
                    const qreal &resolutionFraction);

    void pushTimerExpired();
};

#endif // CANVASWINDOW_H
