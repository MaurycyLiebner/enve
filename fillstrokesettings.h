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


class GradientWidget;

enum PaintType {
    NOPAINT,
    FLATPAINT,
    GRADIENTPAINT
};

class VectorPath;

class ChangeGradientColorsUndoRedo;

class Gradient : ConnectedToMainWindow
{
public:
    Gradient(Color color1, Color color2,
             GradientWidget *gradientWidget, MainWindow *parent) :
        ConnectedToMainWindow(parent)
    {
        mGradientWidget = gradientWidget;
        colors << color1;
        colors << color2;
        updateQGradientStops();
    }

    void swapColors(int id1, int id2) {
        colors.swap(id1, id2);
        updateQGradientStops();
    }

    void removeColor(int id) {
        colors.removeAt(id);
        updateQGradientStops();
    }

    void addColor(Color color) {
        colors << color;
        updateQGradientStops();
    }

    void replaceColor(int id, Color color) {
        colors.replace(id, color);
        updateQGradientStops();
    }

    void setColors(QList<Color> newColors);

    void startTransform() {
        if(transformPending) return;
        transformPending = true;
        savedColors = colors;
    }

    bool isInPaths(VectorPath *path) {
        return mAffectedPaths.contains(path);
    }

    void addPath(VectorPath *path) {
        mAffectedPaths << path;
    }

    void removePath(VectorPath *path) {
        mAffectedPaths << path;
    }

    void updatePaths();

    void finishTransform();

    void updateQGradientStops();

    bool transformPending = false;
    GradientWidget *mGradientWidget;
    QGradientStops qgradientStops;
    QList<Color> colors;
    QList<Color> savedColors;
    QList<VectorPath*> mAffectedPaths;
};

class PaintSettings {
public:
    PaintSettings() {

    }

    PaintSettings(Color colorT,
                  PaintType paintTypeT,
                  Gradient *gradientT = NULL) {
        color = colorT;
        paintType = paintTypeT;
        gradient = gradientT;
    }

    Color color;
    PaintType paintType = FLATPAINT;
    Gradient *gradient = NULL;
};

class StrokeSettings : public PaintSettings
{
public:
    StrokeSettings() : PaintSettings() {
        color.setQColor(Qt::black);
    }

    StrokeSettings(Color colorT,
                   PaintType paintTypeT,
                   Gradient *gradientT = NULL) : PaintSettings(colorT,
                                                               paintTypeT,
                                                               gradientT)
    {
    }

    void setLineWidth(qreal newWidth) {
        mLineWidth = newWidth;
    }

    qreal lineWidth() {
        return mLineWidth;
    }

    void setCapStyle(Qt::PenCapStyle capStyle) {
        mCapStyle = capStyle;
    }

    Qt::PenCapStyle capStyle() {
        return mCapStyle;
    }

    void setJoinStyle(Qt::PenJoinStyle joinStyle) {
        mJoinStyle = joinStyle;
    }

    Qt::PenJoinStyle joinStyle() {
        return mJoinStyle;
    }

    void setStrokerSettings(QPainterPathStroker *stroker, qreal scale) {
        stroker->setWidth(mLineWidth*scale);
        stroker->setCapStyle(mCapStyle);
        stroker->setJoinStyle(mJoinStyle);
    }
private:
    qreal mLineWidth = 1.f;
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
private:
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
};

#endif // FILLSTROKESETTINGS_H
