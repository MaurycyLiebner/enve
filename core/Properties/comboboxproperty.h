#ifndef COMBOBOXPROPERTY_H
#define COMBOBOXPROPERTY_H
#include "Properties/property.h"
class ComboBoxProperty : public Property {
    Q_OBJECT
public:
    ComboBoxProperty(const QString& name,
                     const QStringList &list);
    const int &getCurrentValue() {
        return mCurrentValue;
    }

    const QStringList &getValueNames() {
        return mValueNames;
    }

    QString getCurrentValueName() {
        if(mCurrentValue >= mValueNames.count()) return "null";
        return mValueNames.at(mCurrentValue);
    }
    bool SWT_isComboBoxProperty() const { return true; }
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);
signals:
    void valueChanged(int);
public slots:
    void setCurrentValue(const int &id) {
        if(mCurrentValue == id) return;
        mCurrentValue = id;
        emit valueChanged(id);
        prp_afterWholeInfluenceRangeChanged();
    }
private:
    int mCurrentValue = 0;
    QStringList mValueNames;
};

#endif // COMBOBOXPROPERTY_H
