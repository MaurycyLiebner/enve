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

#ifndef BOXTARGETPROPERTY_H
#define BOXTARGETPROPERTY_H
#include <QtCore>
#include "Properties/property.h"
class BoundingBox;
class QrealAnimator;

class BoxTargetProperty;

class BoxTargetProperty : public Property {
    Q_OBJECT
public:
    BoxTargetProperty(const QString& name);

    BoundingBox *getTarget() const;
    void setTarget(BoundingBox * const box);

    bool SWT_dropSupport(const QMimeData* const data);

    bool SWT_drop(const QMimeData* const data);

    bool SWT_isBoxTargetProperty() const { return true; }
    void writeProperty(eWriteStream &dst) const;
    void readProperty(eReadStream& src);
signals:
    void targetSet(BoundingBox*);
private:
    qptr<BoundingBox> mTarget_d;
};

#endif // BOXTARGETPROPERTY_H
