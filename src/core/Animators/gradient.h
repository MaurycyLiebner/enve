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

#ifndef GRADIENT_H
#define GRADIENT_H
#include <QGradientStops>
#include "Animators/dynamiccomplexanimator.h"
#include "coloranimator.h"

enum class GradientType : short { LINEAR, RADIAL };

class Gradient : public DynamicComplexAnimator<ColorAnimator> {
    Q_OBJECT
    e_OBJECT
protected:
    Gradient();
public:
    bool SWT_isGradient() const { return true; }

    void prp_writeProperty(eWriteStream &dst) const;
    void prp_readProperty(eReadStream &src);

    void addColor(const QColor &color);
    void replaceColor(const int id, const QColor &color);

    void updateQGradientStops();

    QColor getColorAt(const int id);
    QColor getLastQGradientStopQColor();
    QColor getFirstQGradientStopQColor();

    QGradientStops getQGradientStops() { return mQGradientStops; }
    void startColorIdTransform(const int id);

    QGradientStops getQGradientStopsAtAbsFrame(const qreal absFrame);
signals:
    void removed();
private:
    QGradientStops mQGradientStops;
};

#endif // GRADIENT_H
