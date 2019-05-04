#include "coloranimatorbutton.h"
#include "Animators/coloranimator.h"
#include "GUI/ColorWidgets/colorsettingswidget.h"
#include <QVBoxLayout>
#include <QDialog>
#include "GUI/mainwindow.h"

ColorAnimatorButton::ColorAnimatorButton(ColorAnimator * const colorTarget,
                                         QWidget * const parent) :
    BoxesListActionButton(parent) {
    setColorTarget(colorTarget);

    connect(this, &BoxesListActionButton::pressed,
            this, &ColorAnimatorButton::openColorSettingsDialog);
}

void ColorAnimatorButton::setColorTarget(ColorAnimator * const target) {
    if(mColorTarget) {
        disconnect(mColorTarget->getVal1Animator(),
                   &QrealAnimator::valueChangedSignal,
                   this, qOverload<>(&ColorAnimatorButton::update));
        disconnect(mColorTarget->getVal2Animator(),
                  &QrealAnimator::valueChangedSignal,
                   this, qOverload<>(&ColorAnimatorButton::update));
        disconnect(mColorTarget->getVal3Animator(),
                   &QrealAnimator::valueChangedSignal,
                   this, qOverload<>(&ColorAnimatorButton::update));
    }
    mColorTarget = target;
    if(target) {
        connect(target->getVal1Animator(),
                &QrealAnimator::valueChangedSignal,
                this, qOverload<>(&ColorAnimatorButton::update));
        connect(target->getVal2Animator(),
                &QrealAnimator::valueChangedSignal,
                this, qOverload<>(&ColorAnimatorButton::update));
        connect(target->getVal3Animator(),
                &QrealAnimator::valueChangedSignal,
                this, qOverload<>(&ColorAnimatorButton::update));
    }
}

void ColorAnimatorButton::paintEvent(QPaintEvent *) {
    if(!mColorTarget) return;
    QPainter p(this);
    if(mHover) p.setPen(Qt::red);
    else p.setPen(Qt::white);
    p.setBrush(mColorTarget->getCurrentColor());
    p.drawRect(0, 0, width() - 1, height() - 1);
}

void ColorAnimatorButton::openColorSettingsDialog() {
    if(!mColorTarget) return;
    const auto dialog = new QDialog(MainWindow::getInstance());
    dialog->setLayout(new QVBoxLayout(dialog));
    const auto colorSettingsWidget = new ColorSettingsWidget(dialog);
    colorSettingsWidget->setColorAnimatorTarget(mColorTarget);
    dialog->layout()->addWidget(colorSettingsWidget);
    connect(MainWindow::getInstance(), &MainWindow::updateAll,
            dialog, qOverload<>(&QDialog::update));
    dialog->raise();
    dialog->exec();
}
