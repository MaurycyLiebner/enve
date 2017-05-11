#ifndef SINGLEGRADIENTWIDGET_H
#define SINGLEGRADIENTWIDGET_H

#include "Colors/ColorWidgets/glwidget.h"
class Gradient;

class SingleGradientWidget : public GLWidget
{
    Q_OBJECT
public:
    explicit SingleGradientWidget(QWidget *parent = 0);

    void paintGL();

    void setSelected(const bool &bT);
private:
    Gradient *mTargetGradient;
    bool mSelected = false;
signals:

public slots:
};

#endif // SINGLEGRADIENTWIDGET_H
