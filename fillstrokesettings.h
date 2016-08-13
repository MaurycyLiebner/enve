#ifndef FILLSTROKESETTINGS_H
#define FILLSTROKESETTINGS_H

#include <QWidget>
#include "Colors/ColorWidgets/colorsettingswidget.h"
#include <QTabWidget>
#include <QPushButton>
#include <QTabBar>
#include <QPen>

class GradientWidget;

enum PaintType {
    NOPAINT,
    FLATPAINT,
    GRADIENTPAINT
};

struct Gradient {
    Gradient(Color color1, Color color2) {
        colors << color1;
        colors << color2;
    }

    void swapColors(int id1, int id2) {
        colors.swap(id1, id2);
    }

    void removeColor(int id) {
        colors.removeAt(id);
    }

    void addColor(Color color) {
        colors << color;
    }

    void replaceColor(int id, Color color) {
        colors.replace(id, color);
    }

    QList<Color> colors;
};

struct PaintSettings {
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

struct StrokeSettings {

    void updateQPen() {
        qpen.setWidthF(lineWidth);
        qpen.setColor(paintSettings.color.qcol);
    }

    void setLineWidth(qreal newWidth) {
        lineWidth = newWidth;
        updateQPen();
    }

    void setPainSettings(PaintSettings settings) {
        paintSettings = settings;
        updateQPen();
    }

    QPen qpen;
    qreal lineWidth = 1.f;
    PaintSettings paintSettings = PaintSettings(Color(0, 0, 0), FLATPAINT);
};

class FillStrokeSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FillStrokeSettingsWidget(QWidget *parent = 0);

    void setCurrentSettings(PaintSettings fillPaintSettings,
                            StrokeSettings strokePaintSettings);
    void setCurrentDisplayedSettings(PaintSettings settings);
    void setCurrentPaintType(PaintType paintType);
signals:
    void fillSettingsChanged(PaintSettings);
    void strokeSettingsChanged(StrokeSettings);
private slots:
    void setStrokeWidth(qreal width);
    void colorTypeSet(int id);
    void setFillTarget();
    void setStrokeTarget();

    void colorSet(GLfloat h, GLfloat s, GLfloat v, GLfloat a);
private:
    PaintSettings *getCurrentTargetPaintSettings();

    void emitTargetSettingsChanged();

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
    QHBoxLayout *mLineWidthLayout = new QHBoxLayout();
    QLabel *mLineWidthLabel = new QLabel("Width:");
    QDoubleSpinBox *mLineWidthSpin;

    ColorSettingsWidget *mColorsSettingsWidget;

    GradientWidget *mGradientWidget;
};

#endif // FILLSTROKESETTINGS_H
