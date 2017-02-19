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
#include "Animators/coloranimator.h"
#include "actionbutton.h"

class GradientWidget;

enum PaintType {
    NOPAINT,
    FLATPAINT,
    GRADIENTPAINT
};

class PathBox;

class ChangeGradientColorsUndoRedo;

class GradientPoints;

class Gradient : public ComplexAnimator
{
public:
    Gradient(Color color1, Color color2,
             GradientWidget *gradientWidget);

    Gradient(Gradient *fromGradient,
             GradientWidget *gradientWidget);

    Gradient(int sqlIdT,
             GradientWidget *gradientWidget);

    int saveToSql(QSqlQuery *query);

    void saveToSqlIfPathSelected(QSqlQuery *query);

    void swapColors(int id1, int id2);

    void removeColor(int id);

    void addColor(Color color);

    void replaceColor(int id, Color color);

    void setColors(QList<Color> newColors);

    void startTransform();

    bool isInPaths(PathBox *path);

    void addPath(PathBox *path);

    void removePath(PathBox *path);

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
    void scheduleQGradientStopsUpdate();
    void updateQGradientStopsIfNeeded();
    void startColorIdTransform(int id);
    void addColorToList(ColorAnimator *newColorAnimator);
    ColorAnimator *getColorAnimatorAt(int id);
private:
    int mSqlId = -1;
    GradientWidget *mGradientWidget;
    QGradientStops mQGradientStops;
    QList<ColorAnimator*> mColors;
    QList<PathBox*> mAffectedPaths;

    bool mQGradientStopsUpdateNeeded = false;
};

class PaintSettings : public ComplexAnimator {
public:
    PaintSettings();

    PaintSettings(Color colorT,
                  PaintType paintTypeT,
                  Gradient *gradientT = NULL);

    virtual int saveToSql(QSqlQuery *query);

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
        if(gradient == mGradient) return;
        if(mPaintType == GRADIENTPAINT && mGradient != NULL) {
            removeChildAnimator(mGradient);
            removeChildAnimator((QrealAnimator*) mGradientPoints);
        }
        mGradient = gradient;
        if(mPaintType == GRADIENTPAINT && mGradient != NULL) {
            addChildAnimator(mGradient);
            addChildAnimator((QrealAnimator*) mGradientPoints);
        }
    }

    void setCurrentColor(Color color) {
        mColor.setCurrentValue(color);
    }

    void setPaintType(PaintType paintType) {
        if(mPaintType == GRADIENTPAINT && paintType != GRADIENTPAINT) {
            removeChildAnimator(mGradient);
            removeChildAnimator((QrealAnimator*) mGradientPoints);
        } else if(paintType == GRADIENTPAINT && mPaintType != GRADIENTPAINT) {
            addChildAnimator(mGradient);
            addChildAnimator((QrealAnimator*) mGradientPoints);
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

    void setGradientPoints(GradientPoints *gradientPoints) {
        mGradientPoints = gradientPoints;
    }

    virtual void loadFromSql(int sqlId, GradientWidget *gradientWidget);
    void setPaintPathTarget(PathBox *path);

    void makeDuplicate(QrealAnimator *target) {
        PaintSettings *paintSettingsTarget = (PaintSettings*)target;
        paintSettingsTarget->duplicateColorAnimatorFrom(&mColor);
        paintSettingsTarget->setGradient(mGradient);
        paintSettingsTarget->setPaintType(mPaintType);
    }

    void duplicateColorAnimatorFrom(ColorAnimator *source) {
        source->makeDuplicate(&mColor);
    }

private:
    GradientPoints *mGradientPoints = NULL;
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

    int saveToSql(QSqlQuery *query);

    void setCurrentStrokeWidth(qreal newWidth);

    void setCapStyle(Qt::PenCapStyle capStyle);

    void setJoinStyle(Qt::PenJoinStyle joinStyle);

    void setStrokerSettings(QPainterPathStroker *stroker);
    StrokeSettings(int strokeSqlId, int paintSqlId, GradientWidget *gradientWidget);

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

    void setOutlineCompositionMode(QPainter::CompositionMode compositionMode) {
        mOutlineCompositionMode = compositionMode;
    }

    QPainter::CompositionMode getOutlineCompositionMode() {
        return mOutlineCompositionMode;
    }

    void setLineWidthUpdaterTarget(PathBox *path);
    void loadFromSql(int strokeSqlId, int paintSqlId, GradientWidget *gradientWidget);
    void loadFromSql(int strokeSqlId, GradientWidget *gradientWidget);
    bool nonZeroLineWidth();

    void makeDuplicate(QrealAnimator *target) {
        PaintSettings::makeDuplicate(target);
        StrokeSettings *strokeSettingsTarget = (StrokeSettings*)target;
        strokeSettingsTarget->duplicateLineWidthFrom(&mLineWidth);
        strokeSettingsTarget->setCapStyle(mCapStyle);
        strokeSettingsTarget->setJoinStyle(mJoinStyle);
        strokeSettingsTarget->setOutlineCompositionMode(mOutlineCompositionMode);
    }

    void duplicateLineWidthFrom(QrealAnimator *source) {
        source->makeDuplicate(&mLineWidth);
    }

    QrealAnimator *getLineWidthAnimator() {
        return &mLineWidth;
    }

private:
    QrealAnimator mLineWidth;
    Qt::PenCapStyle mCapStyle = Qt::RoundCap;
    Qt::PenJoinStyle mJoinStyle = Qt::RoundJoin;
    QPainter::CompositionMode mOutlineCompositionMode =
            QPainter::CompositionMode_Source;
};

class MainWindow;
class Canvas;
class QDoubleSlider;
class CanvasWidget;

class FillStrokeSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FillStrokeSettingsWidget(MainWindow *parent = 0);

    void setCurrentSettings(PaintSettings *fillPaintSettings,
                            StrokeSettings *strokePaintSettings);
    void updateAfterTargetChanged();
    void setCurrentPaintType(PaintType paintType);

    void saveGradientsToQuery(QSqlQuery *query);
    void loadAllGradientsFromSql();
    GradientWidget *getGradientWidget();

    void clearAll();
    void saveGradientsToSqlIfPathSelected(QSqlQuery *query);
    void loadSettingsFromPath(PathBox *path);
    void setFillValuesFromFillSettings(PaintSettings *settings);
    void setStrokeValuesFromStrokeSettings(StrokeSettings *settings);

    void setCanvasWidgetPtr(CanvasWidget *canvasWidget);
    void updateColorAnimator();
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
    void setGradientFill();
    void setFlatFill();
    void setNoneFill();

    void setColorAnimatorTarget(ColorAnimator *animator);
private:
    CanvasWidget *mCanvasWidget;

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
    ActionButton *mFillNoneButton = NULL;
    ActionButton *mFillFlatButton = NULL;
    ActionButton *mFillGradientButton = NULL;

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
