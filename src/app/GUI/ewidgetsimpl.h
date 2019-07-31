#ifndef EWIDGETSIMPL_H
#define EWIDGETSIMPL_H
#include "GUI/ewidgets.h"

class eWidgetsImpl : private eWidgets {
    template <typename T> using Func = std::function<T>;
public:
    eWidgetsImpl();
protected:
    QWidget* colorWidget(QWidget* const parent,
                         const QColor& iniColor,
                         QObject* const receiver,
                         const Func<void(const ColorSetting&)>& slot);
    QWidget* colorWidget(QWidget* const parent,
                         ColorAnimator* const target);
};

#endif // EWIDGETSIMPL_H
