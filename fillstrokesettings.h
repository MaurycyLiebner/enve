#ifndef FILLSTROKESETTINGS_H
#define FILLSTROKESETTINGS_H

#include <QWidget>
#include "Colors/ColorWidgets/colorsettingswidget.h"
#include <QTabWidget>
#include <QPushButton>
#include <QTabBar>
#include <QPen>
#include <QGradient>
#include <QDebug>
#include <QTimer>
#include <QPainterPathStroker>
#include "connectedtomainwindow.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include "coloranimator.h"

class GradientWidget;

enum PaintType {
    NOPAINT,
    FLATPAINT,
    GRADIENTPAINT
};

class VectorPath;

class ChangeGradientColorsUndoRedo;

class Gradient : public ConnectedToMainWindow
{
public:
    Gradient(Color color1, Color color2,
             GradientWidget *gradientWidget, MainWindow *parent);

    Gradient(Gradient *fromGradient,
             GradientWidget *gradientWidget, MainWindow *parent);

    Gradient(int sqlIdT,
             GradientWidget *gradientWidget, MainWindow *parent);

    int saveToSql();

    void saveToSqlIfPathSelected();

    void swapColors(int id1, int id2);

    void removeColor(int id);

    void addColor(Color color);

    void replaceColor(int id, Color color);

    void setColors(QList<Color> newColors);

    void startTransform();

    bool isInPaths(VectorPath *path);

    void addPath(VectorPath *path);

    void removePath(VectorPath *path);

    bool affectsPaths();

    void updatePaths();

    void finishTransform();

    void updateQGradientStops();

    int getSqlId();

    int sqlId = -1;
    bool transformPending = false;
    GradientWidget *mGradientWidget;
    QGradientStops qgradientStops;
    QList<Color> colors;
    QList<Color> savedColors;
    QList<VectorPath*> mAffectedPaths;
};

class PaintSettings {
public:
    PaintSettings();

    PaintSettings(Color colorT,
                  PaintType paintTypeT,
                  Gradient *gradientT = NULL);

    PaintSettings(int sqlId, GradientWidget *gradientWidget);

    virtual int saveToSql();

    ColorAnimator color;
    PaintType paintType = FLATPAINT;
    Gradient *gradient = NULL;
};

class StrokeSettings : public PaintSettings
{
public:
    StrokeSettings();

    StrokeSettings(Color colorT,
                   PaintType paintTypeT,
                   Gradient *gradientT = NULL);

    int saveToSql();

    void setLineWidth(qreal newWidth);

    qreal lineWidth();

    void setCapStyle(Qt::PenCapStyle capStyle);

    Qt::PenCapStyle capStyle();

    void setJoinStyle(Qt::PenJoinStyle joinStyle);

    Qt::PenJoinStyle joinStyle();

    void setStrokerSettings(QPainterPathStroker *stroker);
    StrokeSettings(int strokeSqlId, int paintSqlId, GradientWidget *gradientWidget);
    static StrokeSettings createStrokeSettingsFromSql(int strokeSqlId,
                                              GradientWidget *gradientWidget);
private:
    QrealAnimator mLineWidth;
    Qt::PenCapStyle mCapStyle = Qt::RoundCap;
    Qt::PenJoinStyle mJoinStyle = Qt::RoundJoin;
};

class MainWindow;

class FillStrokeSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FillStrokeSettingsWidget(MainWindow *parent = 0);

    void setCurrentSettings(PaintSettings fillPaintSettings,
                            StrokeSettings strokePaintSettings);
    void setCurrentDisplayedSettings(PaintSettings *settings);
    void setCurrentPaintType(PaintType paintType);

    void setCurrentColor(GLfloat h, GLfloat s, GLfloat v, GLfloat a);
    void setCurrentColor(Color color);

    void saveGradientsToQuery();
    void loadAllGradientsFromSql();
    GradientWidget *getGradientWidget();

    void clearAll();
    void saveGradientsToSqlIfPathSelected();
    void loadSettingsFromPath(VectorPath *path);
signals:
    void fillSettingsChanged(PaintSettings, bool);
    void strokeSettingsChanged(StrokeSettings, bool);
    void finishFillSettingsTransform();
    void finishStrokeSettingsTransform();
    void startFillSettingsTransform();
    void startStrokeSettingsTransform();
private slots:
    void colorChangedTMP(GLfloat h, GLfloat s, GLfloat v, GLfloat a);
    void setStrokeWidth(qreal width);

    void colorTypeSet(int id);
    void setFillTarget();
    void setStrokeTarget();

    void flatColorSet(GLfloat h, GLfloat s, GLfloat v, GLfloat a);
    void emitTargetSettingsChanged();
    void setGradient(Gradient* gradient);

    void setBevelJoinStyle();
    void setMiterJoinStyle();
    void setRoundJoinStyle();

    void setFlatCapStyle();
    void setSquareCapStyle();
    void setRoundCapStyle();

    void waitToSaveChanges();

    void finishTransform();
    void startTransform();
    void emitTargetSettingsChangedTMP();

    void startLoadingFillFromPath();
    void startLoadingStrokeFromPath();
    void startLoadingSettingsFromPath();
private:
    bool mLoadFillFromPath = false;
    bool mLoadStrokeFromPath = false;

    MainWindow *mMainWindow;
    bool mTransormStarted = false;

    QTimer *mUndoRedoSaveTimer;

    void connectGradient();
    void disconnectGradient();

    void setJoinStyle(Qt::PenJoinStyle joinStyle);

    void setCapStyle(Qt::PenCapStyle capStyle);

    PaintSettings *getCurrentTargetPaintSettings();


    int mTargetId = 0;

    PaintSettings mFillPaintSettings;
    StrokeSettings mStrokePaintSettings;

    void setNoPaintType();
    void setFlatPaintType();
    void setGradientPaintType();

    QVBoxLayout *mMainLayout = new QVBoxLayout();

    QHBoxLayout *mTargetLayout = new QHBoxLayout();
    QPushButton *mFillTargetButton;
    QPushButton *mStrokeTargetButton;

    QTabBar *mColorTypeBar;

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
    QDoubleSpinBox *mLineWidthSpin;

    ColorSettingsWidget *mColorsSettingsWidget;

    GradientWidget *mGradientWidget;

    QHBoxLayout *mPickersLayout = new QHBoxLayout();
    QPushButton *mFillPickerButton;
    QPushButton *mStrokePickerButton;
    QPushButton *mFillStrokePickerButton;
};

#endif // FILLSTROKESETTINGS_H
