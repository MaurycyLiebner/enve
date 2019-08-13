#ifndef CURRENTGRADIENTWIDGET_H
#define CURRENTGRADIENTWIDGET_H

#include "GUI/ColorWidgets/glwidget.h"
class GradientWidget;

class CurrentGradientWidget : public GLWidget {
    Q_OBJECT
public:
    explicit CurrentGradientWidget(GradientWidget *gradientWidget,
                                   QWidget *parent = nullptr);

    void paintGL();
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent *);
    int mHoveredX = 0;
    GradientWidget *mGradientWidget;
signals:

public slots:
};

#endif // CURRENTGRADIENTWIDGET_H
