// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef COMBOBOXPROPERTY_H
#define COMBOBOXPROPERTY_H
#include "property.h"
class ComboBoxProperty : public Property {
    Q_OBJECT
    e_OBJECT
protected:
    ComboBoxProperty(const QString& name,
                     const QStringList &list);
public:
    bool SWT_isComboBoxProperty() const { return true; }
    void prp_writeProperty(eWriteStream& dst) const;
    void prp_readProperty(eReadStream& src);

    const QStringList &getValueNames() {
        return mValueNames;
    }

    QString getCurrentValueName() {
        if(mCurrentValue >= mValueNames.count()) return "null";
        return mValueNames.at(mCurrentValue);
    }

    int getCurrentValue() {
        return mCurrentValue;
    }
    void setCurrentValue(const int id) {
        if(mCurrentValue == id) return;
        mCurrentValue = id;
        emit valueChanged(id);
        prp_afterWholeInfluenceRangeChanged();
    }
signals:
    void valueChanged(int);
private:
    int mCurrentValue = 0;
    QStringList mValueNames;
};

#endif // COMBOBOXPROPERTY_H
