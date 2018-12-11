#ifndef DISPLACYEDGRADIENTSWIDGET_H
#define DISPLACYEDGRADIENTSWIDGET_H

#include "GUI/ColorWidgets/glwidget.h"
#include "global.h"
class Gradient;
class GradientWidget;

class DisplayedGradientsWidget : public GLWidget {
    Q_OBJECT
public:
    explicit DisplayedGradientsWidget(GradientWidget *gradientWidget,
                                      QWidget *parent = nullptr);

    void incTop(const int &inc);
    void setTop(const int &top);
    void updateTopGradientId();
    void setNumberGradients(const int &n);
    void paintGL();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent *);

    int mContextMenuGradientId = -1;
    int mDisplayedTop = 0;
    int mTopGradientId = 0;
    int mHoveredGradientId = -1;

    int mNumberVisibleGradients = 6;

    GradientWidget *mGradientWidget;
signals:

public slots:
};

#endif // DISPLACYEDGRADIENTSWIDGET_H
