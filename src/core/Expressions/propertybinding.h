#ifndef PROPERTYBINDING_H
#define PROPERTYBINDING_H

#include <QObject>
#include <QJSValue>

#include "smartPointers/selfref.h"
#include "conncontextptr.h"
#include "framerange.h"
#include "simpletask.h"
#include "smartPointers/ememory.h"

#include "Properties/property.h"

class PropertyBinding : public QObject {
    Q_OBJECT
public:
    using Validator = std::function<bool(Property*)>;
protected:
    PropertyBinding(const Validator& validator,
                    const Property* const context);
public:
    static qsptr<PropertyBinding> sCreate(const QString& binding,
                                          const Validator& validator,
                                          const Property* const context);
    template <class T>
    static bool sValidateClass(const Validator& validator,
                               Property* const prop);

    template <class T>
    static Validator sWrapValidatorForClass(const Validator& validator);

    virtual QJSValue getJSValue(QJSEngine& e) = 0;
    virtual QJSValue getJSValue(QJSEngine& e, const qreal relFrame) = 0;

    virtual void updateValue() = 0;

    void setPath(const QString& path);
    const QString& path() const { return mPath; }
    Property* getBindProperty() const { return mBindProperty.get(); }
    bool isBindPathValid() const { return mBindPathValid; }

    bool setAbsFrame(const qreal absFrame);
    bool dependsOn(const Property* const prop);
    FrameRange identicalRange(const qreal absFrame);
signals:
    void relRangeChanged(const FrameRange& range);
    void currentValueChanged();
protected:
    void updateValueIfNeeded();
private:
    static Property* sFindPropertyToBind(const QString& binding,
                                         const Validator& validator,
                                         const Property* const context);
    void reloadBindProperty();
    void updateBindPath();
    bool bindProperty(Property * const newBinding);
    void setBindPathValid(const bool valid);
    SimpleTaskScheduler pathChanged;

    bool mValueUpToDate = false;
    qreal mRelFrame = 123456789.123456789;

    bool mBindPathValid = false;
    QString mPath;
    ConnContextQPtr<Property> mBindProperty;
    const Validator mValidator;
    const qptr<const Property> mContext;
};

template <class T>
bool PropertyBinding::sValidateClass(const Validator& validator,
                                     Property* const prop) {
    return enve_cast<T*>(prop) && (!validator || validator(prop));
}

template <class T>
PropertyBinding::Validator
PropertyBinding::sWrapValidatorForClass(const Validator& validator) {
    return [validator](Property* const prop) {
        return sValidateClass<T>(validator, prop);
    };
}

#endif // PROPERTYBINDING_H
