#ifndef COLORANIMATORBUTTON_H
#define COLORANIMATORBUTTON_H
#include "boxeslistactionbutton.h"
class ColorAnimator;

class ColorAnimatorButton : public BoxesListActionButton {
    Q_OBJECT
public:
    ColorAnimatorButton(ColorAnimator *colorTarget,
                        QWidget *parent = nullptr);


    void openColorSettingsDialog();
protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    ColorAnimator *mColorTarget = nullptr;
};

#endif // COLORANIMATORBUTTON_H
