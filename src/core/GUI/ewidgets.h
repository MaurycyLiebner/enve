#ifndef EWIDGETS_H
#define EWIDGETS_H

#include <functional>

class QObject;
class QWidget;

class QColor;
class ColorSetting;
class ColorAnimator;

class eWidgets {
    template <typename T> using Func = std::function<T>;
    static eWidgets* sInstance;
protected:
    eWidgets();
public:
    static QWidget* sColorWidget(QWidget* const parent,
                                 const QColor& iniColor,
                                 QObject* const receiver,
                                 const Func<void(const ColorSetting&)>& slot) {
        return sInstance->colorWidget(parent, iniColor, receiver, slot);
    }

    static QWidget* sColorWidget(QWidget* const parent,
                                 ColorAnimator* const target) {
        return sInstance->colorWidget(parent, target);
    }
protected:
    virtual QWidget* colorWidget(QWidget* const parent,
                                 const QColor& iniColor,
                                 QObject* const receiver,
                                 const Func<void(const ColorSetting&)>& slot) = 0;
    virtual QWidget* colorWidget(QWidget* const parent,
                                 ColorAnimator* const target) = 0;
};

#endif // EWIDGETS_H
