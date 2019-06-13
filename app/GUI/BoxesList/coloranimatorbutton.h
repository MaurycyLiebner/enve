#ifndef COLORANIMATORBUTTON_H
#define COLORANIMATORBUTTON_H
#include "boxeslistactionbutton.h"
#include "smartPointers/selfref.h"
class ColorAnimator;

class ColorAnimatorButton : public BoxesListActionButton {
    ColorAnimatorButton(QWidget * const parent = nullptr);
public:
    ColorAnimatorButton(ColorAnimator * const colorTarget,
                        QWidget * const parent = nullptr);
    ColorAnimatorButton(const QColor& color,
                        QWidget * const parent = nullptr);

    void setColorTarget(ColorAnimator * const target);
    void openColorSettingsDialog();

    void setColor(const QColor& color) { mColor = color; }
    QColor color() const;
protected:
    void paintEvent(QPaintEvent *);
private:
    QColor mColor;
    qptr<ColorAnimator> mColorTarget;
};

#endif // COLORANIMATORBUTTON_H
