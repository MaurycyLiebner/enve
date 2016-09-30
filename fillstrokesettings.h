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

class Gradient : public ComplexAnimator
{
public:
    Gradient(Color color1, Color color2,
             GradientWidget *gradientWidget);

    Gradient(Gradient *fromGradient,
             GradientWidget *gradientWidget);

    Gradient(int sqlIdT,
             GradientWidget *gradientWidget);

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
    void setSqlId(int id);

    void addColorToList(Color color);

    Color getCurrentColorAt(int id);

    int getColorCount();

    QColor getLastQGradientStopQColor();
    QColor getFirstQGradientStopQColor();

    QGradientStops getQGradientStops();
private:
    int mSqlId = -1;
    GradientWidget *mGradientWidget;
    QGradientStops mQGradientStops;
    QList<ColorAnimator*> mColors;
    QList<VectorPath*> mAffectedPaths;
};

class PaintSettings : public ComplexAnimator {
public:
    PaintSettings();

    PaintSettings(Color colorT,
                  PaintType paintTypeT,
                  Gradient *gradientT = NULL);

    PaintSettings(int sqlId, GradientWidget *gradientWidget);

    virtual int saveToSql();

    Color getCurrentColor() const {
        return mColor.getCurrentValue();
    }

    PaintType getPaintType() const {
        return mPaintType;
    }

    Gradient *getGradient() const {
        return mGradient;
    }

    void setGradient(Gradient *gradient) {
        mGradient = gradient;
    }

    void setCurrentColor(Color color) {
        mColor.setCurrentValue(color);
    }

    void setPaintType(PaintType paintType) {
        if(mPaintType == GRADIENTPAINT && paintType != GRADIENTPAINT) {
            removeChildAnimator(mGradient);
        } else if(paintType == GRADIENTPAINT && mPaintType != GRADIENTPAINT) {
            addChildAnimator(mGradient);
        }
        if(mPaintType == FLATPAINT && paintType != FLATPAINT) {
            removeChildAnimator(&mColor);
        } else if(paintType == FLATPAINT && mPaintType != FLATPAINT) {
            addChildAnimator(&mColor);
        }
        mPaintType = paintType;
    }

    ColorAnimator *getColorAnimator() {
        return &mColor;
    }

private:
    ColorAnimator mColor;
    PaintType mPaintType = FLATPAINT;
    Gradient *mGradient = NULL;
};

class StrokeSettings : public PaintSettings
{
public:
    StrokeSettings();

    StrokeSettings(Color colorT,
                   PaintType paintTypeT,
                   Gradient *gradientT = NULL);

    int saveToSql();

    void setCurrentStrokeWidth(qreal newWidth);

    void setCapStyle(Qt::PenCapStyle capStyle);

    void setJoinStyle(Qt::PenJoinStyle joinStyle);

    void setStrokerSettings(QPainterPathStroker *stroker);
    StrokeSettings(int strokeSqlId, int paintSqlId, GradientWidget *gradientWidget);
    static StrokeSettings createStrokeSettingsFromSql(int strokeSqlId,
                                              GradientWidget *gradientWidget);

    qreal getCurrentStrokeWidth() const {
        return mLineWidth.getCurrentValue();
    }

    Qt::PenCapStyle getCapStyle() const {
        return mCapStyle;
    }

    Qt::PenJoinStyle getJoinStyle() const {
        return mJoinStyle;
    }

    QrealAnimator *getStrokeWidthAnimator() {
        return &mLineWidth;
    }

private:
    QrealAnimator mLineWidth;
    Qt::PenCapStyle mCapStyle = Qt::RoundCap;
    Qt::PenJoinStyle mJoinStyle = Qt::RoundJoin;
};

class MainWindow;
class Canvas;

class FillStrokeSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FillStrokeSettingsWidget(MainWindow *parent = 0);

    void setCurrentSettings(const PaintSettings *fillPaintSettings,
                            const StrokeSettings *strokePaintSettings);
    void updateAfterTargetChanged();
    void setCurrentPaintType(PaintType paintType);

    void setCurrentColor(GLfloat h, GLfloat s, GLfloat v, GLfloat a);
    void setCurrentColor(Color color);

    void saveGradientsToQuery();
    void loadAllGradientsFromSql();
    GradientWidget *getGradientWidget();

    void clearAll();
    void saveGradientsToSqlIfPathSelected();
    void loadSettingsFromPath(VectorPath *path);
    void setFillValuesFromFillSettings(const PaintSettings *settings);
    void setStrokeValuesFromStrokeSettings(const StrokeSettings *settings);

    void setCanvasPtr(Canvas *canvas);
public slots:
    void emitColorSettingsChangedTMP();
    void emitColorSettingsChanged();
    void emitPaintTypeChanged();
    void emitStrokeWidthChanged();
    void emitStrokeWidthChangedTMP();
    void emitCapStyleChanged();
    void emitJoinStyleChanged();
    void emitGradientChangedTMP();
    void emitGradientChanged();
    void emitFlatColorChangedTMP();
    void emitFlatColorChanged();
private slots:
    void colorChangedTMP(GLfloat h, GLfloat s, GLfloat v, GLfloat a);
    void setStrokeWidth(qreal width);

    void colorTypeSet(int id);
    void setFillTarget();
    void setStrokeTarget();

    void flatColorSet(GLfloat h, GLfloat s, GLfloat v, GLfloat a);
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
private:
    Canvas *mCanvas;
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

    PaintType getCurrentPaintTypeVal() {
        if(mTargetId == 0) {
            return mCurrentFillPaintType;
        } else {
            return mCurrentStrokePaintType;
        }
    }

    void setCurrentPaintTypeVal(PaintType paintType) {
        if(mTargetId == 0) {
            mCurrentFillPaintType = paintType;
        } else {
            mCurrentStrokePaintType = paintType;
        }
    }

    Color getCurrentColorVal() {
        if(mTargetId == 0) {
            return mCurrentFillColor;
        } else {
            return mCurrentStrokeColor;
        }
    }

    void setCurrentColorVal(Color color) {
        if(mTargetId == 0) {
            mCurrentFillColor = color;
        } else {
            mCurrentStrokeColor = color;
        }
    }

    Gradient *getCurrentGradientVal() {
        if(mTargetId == 0) {
            return mCurrentFillGradient;
        } else {
            return mCurrentStrokeGradient;
        }
    }

    void setCurrentGradientVal(Gradient *gradient) {
        if(mTargetId == 0) {
            mCurrentFillGradient = gradient;
        } else {
            mCurrentStrokeGradient = gradient;
        }
    }

    PaintType mCurrentFillPaintType;
    PaintType mCurrentStrokePaintType;
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
    void setTransformFinishEmitter(const char *slot);
    void emitStrokeFlatColorChanged();
    void emitStrokeFlatColorChangedTMP();
    void emitStrokeGradientChanged();
    void emitStrokeGradientChangedTMP();
    void emitStrokePaintTypeChanged();
    void emitFillFlatColorChanged();
    void emitFillFlatColorChangedTMP();
    void emitFillGradientChanged();
    void emitFillGradientChangedTMP();
    void emitFillPaintTypeChanged();
};

#endif // FILLSTROKESETTINGS_H
