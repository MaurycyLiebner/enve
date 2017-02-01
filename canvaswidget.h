#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QWidget>
class Canvas;
enum CanvasMode : short;
class Color;
class Gradient;

#include "fillstrokesettings.h"

class CanvasWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CanvasWidget(QWidget *parent = 0);

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
protected:
    Canvas *mCurrentCanvas = NULL;
    QList<Canvas*> mCanvasList;

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);
signals:

public slots:
    void setMovePathMode();
    void setMovePointMode();
    void setAddPointMode();
    void setRectangleMode();
    void setCircleMode();
    void setTextMode();

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
    void setHighQualityView(const bool &bT);
};

#endif // CANVASWIDGET_H
