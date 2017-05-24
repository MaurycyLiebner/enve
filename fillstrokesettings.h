#ifndef FILLSTROKESETTINGS_H
#define FILLSTROKESETTINGS_H

#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <QTabBar>
#include <QPen>
#include <QGradient>
#include <QDebug>
#include <QTimer>
#include <QPainterPathStroker>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QLabel>
#include <QHBoxLayout>
#include "Animators/coloranimator.h"
#include "Animators/paintsettings.h"

class GradientWidget;
class MainWindow;
class CanvasWindow;
class ColorSettingsWidget;
class QrealAnimatorValueSlider;

class FillStrokeSettingsWidget : public QWidget {
    Q_OBJECT
public:
    explicit FillStrokeSettingsWidget(MainWindow *parent = 0);

    void setCurrentSettings(PaintSettings *fillPaintSettings,
                            StrokeSettings *strokePaintSettings);
    void updateAfterTargetChanged();
    void setCurrentPaintType(PaintType paintType);

    void saveGradientsToQuery(QSqlQuery *query);
    GradientWidget *getGradientWidget();

    void clearAll();
    void saveGradientsToSqlIfPathSelected(QSqlQuery *query);
    void loadSettingsFromPath(PathBox *path);
    void setFillValuesFromFillSettings(PaintSettings *settings);
    void setStrokeValuesFromStrokeSettings(StrokeSettings *settings);

    void setCanvasWindowPtr(CanvasWindow *canvasWidget);
    void updateColorAnimator();
public slots:
    void emitStrokeWidthChanged();
    void emitStrokeWidthChangedTMP();
    void emitCapStyleChanged();
    void emitJoinStyleChanged();
private slots:
    void setStrokeWidth(qreal width);

    void colorTypeSet(int id);
    void setFillTarget();
    void setStrokeTarget();

    void setGradient(Gradient* gradient);

    void setBevelJoinStyle();
    void setMiterJoinStyle();
    void setRoundJoinStyle();

    void setFlatCapStyle();
    void setSquareCapStyle();
    void setRoundCapStyle();

    void waitToSaveChanges();

    void finishTransform();
    void startTransform(const char *slot);

    void startLoadingFillFromPath();
    void startLoadingStrokeFromPath();
    void startLoadingSettingsFromPath();
    void setGradientFill();
    void setFlatFill();
    void setNoneFill();

    void setColorAnimatorTarget(ColorAnimator *animator);
    void colorSettingReceived(const ColorSetting &colorSetting);
    void setCurrentColorMode(const ColorMode &mode);
private:

    CanvasWindow *mCanvasWindow;

    bool mLoadFillFromPath = false;
    bool mLoadStrokeFromPath = false;

    MainWindow *mMainWindow;
    bool mTransormStarted = false;

    QTimer *mUndoRedoSaveTimer;

    void connectGradient();
    void disconnectGradient();

    void setJoinStyle(Qt::PenJoinStyle joinStyle);

    void setCapStyle(Qt::PenCapStyle capStyle);


    int mTargetId = 0;

    //

    PaintType getCurrentPaintTypeVal();

    void setCurrentPaintTypeVal(PaintType paintType);

    Color getCurrentColorVal();

    void setCurrentColorVal(Color color);

    Gradient *getCurrentGradientVal();

    void setCurrentGradientVal(Gradient *gradient);


    ColorAnimator *mCurrentFillColorAnimator = NULL;
    ColorAnimator *mCurrentStrokeColorAnimator = NULL;
    PaintType mCurrentFillPaintType = NOPAINT;
    PaintType mCurrentStrokePaintType = NOPAINT;
    Color mCurrentFillColor;
    Color mCurrentStrokeColor;
    Gradient *mCurrentStrokeGradient = NULL;
    Gradient *mCurrentFillGradient = NULL;
    Qt::PenCapStyle mCurrentCapStyle;
    Qt::PenJoinStyle mCurrentJoinStyle;
    qreal mCurrentStrokeWidth;

    //

    void setNoPaintType();
    void setFlatPaintType();
    void setGradientPaintType();

    QVBoxLayout *mMainLayout = new QVBoxLayout();

    QHBoxLayout *mTargetLayout = new QHBoxLayout();
    QPushButton *mFillTargetButton;
    QPushButton *mStrokeTargetButton;

    QHBoxLayout *mColorTypeLayout;
    QPushButton *mFillNoneButton = NULL;
    QPushButton *mFillFlatButton = NULL;
    QPushButton *mFillGradientButton = NULL;

    QWidget *mStrokeSettingsWidget;
    QVBoxLayout *mStrokeSettingsLayout = new QVBoxLayout();

    QHBoxLayout *mJoinStyleLayout = new QHBoxLayout();
    QPushButton *mBevelJoinStyleButton;
    QPushButton *mMiterJointStyleButton;
    QPushButton *mRoundJoinStyleButton;

    QHBoxLayout *mCapStyleLayout = new QHBoxLayout();
    QPushButton *mFlatCapStyleButton;
    QPushButton *mSquareCapStyleButton;
    QPushButton *mRoundCapStyleButton;

    QHBoxLayout *mLineWidthLayout = new QHBoxLayout();
    QLabel *mLineWidthLabel = new QLabel("Width:");
    QrealAnimatorValueSlider *mLineWidthSpin;

    ColorSettingsWidget *mColorsSettingsWidget;

    GradientWidget *mGradientWidget;

    QHBoxLayout *mPickersLayout = new QHBoxLayout();
    QPushButton *mFillPickerButton;
    QPushButton *mStrokePickerButton;
    QPushButton *mFillStrokePickerButton;
    void setTransformFinishEmitter(const char *slot);
};

#endif // FILLSTROKESETTINGS_H
