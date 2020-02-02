// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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

#ifndef QREALANIMATORVALUESLIDER_H
#define QREALANIMATORVALUESLIDER_H
#include "qdoubleslider.h"
#include "smartPointers/ememory.h"
#include "conncontextptr.h"
class QrealAnimator;
class Property;

class QrealAnimatorValueSlider : public QDoubleSlider {
    Q_OBJECT
public:
    QrealAnimatorValueSlider(QString name, qreal minVal,
                             qreal maxVal, qreal prefferedStep,
                             QWidget *parent);
    QrealAnimatorValueSlider(qreal minVal, qreal maxVal, qreal prefferedStep,
                             QWidget *parent);
    QrealAnimatorValueSlider(qreal minVal, qreal maxVal, qreal prefferedStep,
                             QrealAnimator* animator, QWidget *parent = nullptr);
    QrealAnimatorValueSlider(QrealAnimator* animator, QWidget *parent = nullptr);
    QrealAnimatorValueSlider(QWidget *parent = nullptr);

    void paint(QPainter *p);
    void openContextMenu(const QPoint &globalPos);

    void setTarget(QrealAnimator * const animator);

    bool hasTarget();

    bool isTargetDisabled();


    void setValueExternal(qreal value);

    void emitEditingStarted(qreal value);
    void emitValueChangedExternal(qreal value);
    void emitEditingFinished(qreal value);
    void emitEditingCanceled();

    void setValueFromAnimator(qreal val);
protected:
    void emitValueChanged(qreal value);
    void mouseMoveEvent(QMouseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
private:
    QrealAnimator *getQPointFAnimatorSibling();

    ConnContextQPtr<Property> mTarget;
signals:
    void displayedValueChanged(qreal);
};

#endif // QREALANIMATORVALUESLIDER_H
