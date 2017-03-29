#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QWidget>
class Canvas;
enum CanvasMode : short;
class Color;
class Gradient;
class BoxesGroup;
class PaintControler;
class SoundComposition;

#include "fillstrokesettings.h"
#include "boxeslistanimationdockwidget.h"
#include <QAudioOutput>

class CanvasWidget : public QWidget, public SingleWidgetTarget
{
    Q_OBJECT
public:
    explicit CanvasWidget(QWidget *parent = 0);
    ~CanvasWidget();

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
    void fillPaintTypeChanged(const PaintType &paintType,
                              const Color &color,
                              Gradient *gradient);
    void strokePaintTypeChanged(const PaintType &paintType,
                                const Color &color,
                                Gradient *gradient);
    void strokeCapStyleChanged(const Qt::PenCapStyle &capStyle);
    void strokeJoinStyleChanged(const Qt::PenJoinStyle &joinStyle);
    void strokeWidthChanged(const qreal &strokeWidth, const bool &finish);
    void strokeFlatColorChanged(const Color &color, const bool &finish);
    void fillFlatColorChanged(const Color &color, const bool &finish);
    void fillGradientChanged(Gradient *gradient, const bool &finish);
    void strokeGradientChanged(Gradient *gradient, const bool &finish);
    void pickPathForSettings();
    void updateDisplayedFillStrokeSettings();

    void setResolutionPercent(const qreal &percent);
    void updatePivotIfNeeded();
    void schedulePivotUpdate();

    SWT_Type SWT_getType() {
        return SWT_Canvas;
    }

    BoxesGroup *getCurrentGroup();
    SingleWidgetAbstraction *SWT_getAbstractionForWidget(
            ScrollWidgetVisiblePart *visiblePartWidget);
    void applyPaintSettingToSelected(const PaintSetting &setting);
    void setSelectedFillColorMode(const ColorMode &mode);
    void setSelectedStrokeColorMode(const ColorMode &mode);

    void updateAfterFrameChanged(const int &currentFrame);
    void saveSelectedToSql(QSqlQuery *query);
    void clearAll();
    void createAnimationBoxForPaths(const QStringList &importPaths);
    void createLinkToFileWithPath(const QString &path);
    void saveToSql(QSqlQuery *query);
    void createVideoForPath(const QString &path);
    int getCurrentFrame();
    int getMaxFrame();
    int getMinFrame();
    void addBoxAwaitingUpdate(BoundingBox *box);
    void SWT_addChildrenAbstractions(SingleWidgetAbstraction *abstraction,
                                     ScrollWidgetVisiblePart *visiblePartWidget);
    void createImageForPath(const QString &path);
    void createSoundForPath(const QString &path);
    void loadCanvasesFromSql();
    void saveCanvasesFromSql(QSqlQuery *query);
protected:
    QTimer *mPreviewFPSTimer = NULL;
    QThread *mPaintControlerThread;
    PaintControler *mPaintControler;

    int mSavedCurrentFrame = 0;
    bool mRendering = false;
    bool mNoBoxesAwaitUpdate = true;
    bool mCancelLastBoxUpdate = false;
    BoundingBox *mLastUpdatedBox = NULL;
    QList<BoundingBox*> mBoxesAwaitingUpdate;

    QString mOutputString;
    int mCurrentRenderFrame;

    bool mPreviewInterrupted = false;

    void (CanvasWidget::*mBoxesUpdateFinishedFunction)(void) = NULL;
    Canvas *mCurrentCanvas = NULL;
    QList<Canvas*> mCanvasList;

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);

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

    void stopPreview();
    void playPreview();
    void renderOutput();
private slots:
    void sendNextBoxForUpdate();
    void nextSaveOutputFrame();
    void nextPlayPreviewFrame();
    void saveOutput(QString renderDest);

    void pushTimerExpired();
};

#endif // CANVASWIDGET_H
