#include "coloranimatorbutton.h"
#include "Animators/coloranimator.h"
#include "GUI/ColorWidgets/colorsettingswidget.h"
#include <QVBoxLayout>
#include <QDialog>
#include "GUI/mainwindow.h"

ColorAnimatorButton::ColorAnimatorButton(QWidget * const parent) :
    BoxesListActionButton(parent) {
    connect(this, &BoxesListActionButton::pressed,
            this, &ColorAnimatorButton::openColorSettingsDialog);
}

ColorAnimatorButton::ColorAnimatorButton(ColorAnimator * const colorTarget,
                                         QWidget * const parent) :
    ColorAnimatorButton(parent) {
    setColorTarget(colorTarget);
}

ColorAnimatorButton::ColorAnimatorButton(const QColor &color,
                                         QWidget * const parent) :
    ColorAnimatorButton(parent) {
    mColor = color;
}

void ColorAnimatorButton::setColorTarget(ColorAnimator * const target) {
    if(mColorTarget) {
        disconnect(mColorTarget->getVal1Animator(), nullptr,
                   this, nullptr);
        disconnect(mColorTarget->getVal2Animator(), nullptr,
                   this, nullptr);
        disconnect(mColorTarget->getVal3Animator(), nullptr,
                   this, nullptr);
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
    const QColor color = mColorTarget ?
                mColorTarget->getColor() : mColor;
    QPainter p(this);
    if(mHover) p.setPen(Qt::red);
    else p.setPen(Qt::white);
    p.setBrush(color);
    p.drawRect(0, 0, width() - 1, height() - 1);
}

void ColorAnimatorButton::openColorSettingsDialog() {
    const auto dialog = new QDialog(MainWindow::getInstance());
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setLayout(new QVBoxLayout(dialog));
    const auto colorWidget = new ColorSettingsWidget(dialog);

    dialog->layout()->addWidget(colorWidget);

    if(mColorTarget) {
        colorWidget->setTarget(mColorTarget);
        connect(MainWindow::getInstance(), &MainWindow::updateAll,
                dialog, qOverload<>(&QDialog::update));
    } else {
        colorWidget->setCurrentColor(mColor);
        connect(colorWidget, &ColorSettingsWidget::colorSettingSignal,
                this, [this](const ColorSetting& setting) {
            mColor = setting.getColor();
            update();
        });
    }

    dialog->raise();
    dialog->show();
}

QColor ColorAnimatorButton::color() const {
    if(mColorTarget) return mColorTarget->getColor();
    return mColor;
}
