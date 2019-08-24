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

#ifndef DISPLACYEDGRADIENTSWIDGET_H
#define DISPLACYEDGRADIENTSWIDGET_H

#include "GUI/ColorWidgets/glwidget.h"
#include "GUI/global.h"
class Gradient;
class GradientWidget;

class DisplayedGradientsWidget : public GLWidget {
public:
    explicit DisplayedGradientsWidget(GradientWidget *gradientWidget,
                                      QWidget *parent = nullptr);

    void incTop(const int inc);
    void setTop(const int top);
    void updateTopGradientId();
    void setNumberGradients(const int n);
protected:
    void paintGL();
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent *);

    int mContextMenuGradientId = -1;
    int mDisplayedTop = 0;
    int mTopGradientId = 0;
    int mHoveredGradientId = -1;

    int mNumberVisibleGradients = 6;

    GradientWidget *mGradientWidget;
};

#endif // DISPLACYEDGRADIENTSWIDGET_H
