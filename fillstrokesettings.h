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

class GradientWidget;

enum PaintType {
    NOPAINT,
    FLATPAINT,
    GRADIENTPAINT
};

class Gradient {
public:
    Gradient(Color color1, Color color2) {
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

    void updateQGradientStops();

    QGradientStops qgradientStops;
    QList<Color> colors;
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
        stroker = new QPainterPathStroker();
        color.setQColor(Qt::black);
    }

    StrokeSettings(Color colorT,
                   PaintType paintTypeT,
                   Gradient *gradientT = NULL) : PaintSettings(colorT,
                                                               paintTypeT,
                                                               gradientT)
    {
        stroker = new QPainterPathStroker();
    }

    void setLineWidth(qreal newWidth) {
        stroker->setWidth(newWidth);
    }

    qreal lineWidth() {
        return stroker->width();
    }

    void setCapStyle(Qt::PenCapStyle capStyle) {
        stroker->setCapStyle(capStyle);
    }

    Qt::PenCapStyle capStyle() {
        return stroker->capStyle();
    }

    void setJoinStyle(Qt::PenJoinStyle joinStyle) {
        stroker->setJoinStyle(joinStyle);
    }

    Qt::PenJoinStyle joinStyle() {
        return stroker->joinStyle();
    }

    QPainterPathStroker *stroker;
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

signals:
    void fillSettingsChanged(PaintSettings, bool);
    void strokeSettingsChanged(StrokeSettings, bool);
    void finishFillSettingsTransform();
    void finishStrokeSettingsTransform();
    void startFillSettingsTransform();
    void startStrokeSettingsTransform();
private slots:
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
