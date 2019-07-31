#include "ewidgetsimpl.h"

#include "ColorWidgets/colorsettingswidget.h"

eWidgetsImpl::eWidgetsImpl() {}

QWidget *eWidgetsImpl::colorWidget(QWidget * const parent,
                                   const QColor &iniColor,
                                   QObject * const receiver,
                                   const Func<void (const ColorSetting &)> &slot) {
    const auto wid = new ColorSettingsWidget(parent);
    wid->setCurrentColor(iniColor);
    if(slot) {
        QObject::connect(wid, &ColorSettingsWidget::colorSettingSignal,
                         receiver, slot);
    }
    return wid;
}

QWidget *eWidgetsImpl::colorWidget(QWidget * const parent,
                                   ColorAnimator * const target) {
    const auto wid = new ColorSettingsWidget(parent);
    wid->setTarget(target);
    return wid;
}
