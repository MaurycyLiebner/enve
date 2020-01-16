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

#include "currentgradientwidget.h"
#include "GUI/GradientWidgets/gradientwidget.h"
#include "GUI/global.h"
#include "GUI/ColorWidgets/colorwidgetshaders.h"
#include "Animators/gradient.h"

CurrentGradientWidget::CurrentGradientWidget(GradientWidget *gradientWidget,
                                             QWidget *parent) :
    GLWidget(parent) {
    setMouseTracking(true);
    mGradientWidget = gradientWidget;
    setFixedHeight(MIN_WIDGET_DIM);
}

void CurrentGradientWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(PLAIN_PROGRAM.fID);
    Gradient *gradient = mGradientWidget->getCurrentGradient();
    const int nColors = gradient->ca_getNumberOfChildren();
    int currentColorId = mGradientWidget->getColorId();
    mGradientWidget->getColor();
    qreal xT = 0;
    const qreal xInc = static_cast<qreal>(width())/nColors;
    int hoveredColorId = qFloor(mHoveredX/xInc);

    glUniform2f(PLAIN_PROGRAM.fMeshSizeLoc,
                height()/static_cast<float>(3*xInc), 1.f/3);
    for(int j = 0; j < nColors; j++) {
        QColor currentColor = gradient->getColorAt(j);
        glViewport(qRound(xT), 0, qCeil(xInc), height());


        glUniform4f(PLAIN_PROGRAM.fRGBAColorLoc,
                    currentColor.redF(), currentColor.greenF(),
                    currentColor.blueF(), currentColor.alphaF());

        glBindVertexArray(mPlainSquareVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        if(j == currentColorId) {
            glUseProgram(DOUBLE_BORDER_PROGRAM.fID);
            glUniform2f(DOUBLE_BORDER_PROGRAM.fInnerBorderSizeLoc,
                        1.f/xInc, 1.f/height());
            glUniform2f(DOUBLE_BORDER_PROGRAM.fOuterBorderSizeLoc,
                        1.f/xInc, 1.f/height());
            if(shouldValPointerBeLightHSV(currentColor.hueF(),
                                          currentColor.hsvSaturationF(),
                                          currentColor.valueF())) {
                glUniform4f(DOUBLE_BORDER_PROGRAM.fInnerBorderColorLoc,
                            1, 1, 1, 1);
                glUniform4f(DOUBLE_BORDER_PROGRAM.fOuterBorderColorLoc,
                            0, 0, 0, 1);
            } else {
                glUniform4f(DOUBLE_BORDER_PROGRAM.fInnerBorderColorLoc,
                            0, 0, 0, 1);
                glUniform4f(DOUBLE_BORDER_PROGRAM.fOuterBorderColorLoc,
                            1, 1, 1, 1);
            }
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glUseProgram(PLAIN_PROGRAM.fID);
        }
        if(j == hoveredColorId) {
            glUseProgram(BORDER_PROGRAM.fID);
            glUniform2f(BORDER_PROGRAM.fBorderSizeLoc,
                        1.f/xInc, 1.f/height());
            if(shouldValPointerBeLightHSV(currentColor.hueF(),
                                          currentColor.hsvSaturationF(),
                                          currentColor.valueF())) {
                glUniform4f(BORDER_PROGRAM.fBorderColorLoc,
                            1, 1, 1, 1);
            } else {
                glUniform4f(BORDER_PROGRAM.fBorderColorLoc,
                            0, 0, 0, 1);
            }
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glUseProgram(PLAIN_PROGRAM.fID);
        }
        xT = qRound(xT) + xInc;
    }
}

void CurrentGradientWidget::mousePressEvent(QMouseEvent *event) {
    if(event->button() == Qt::RightButton) {
        mGradientWidget->colorRightPress(event->x(), event->globalPos());
    } else if(event->button() == Qt::LeftButton) {
        mGradientWidget->colorLeftPress(event->x());
    }
}

void CurrentGradientWidget::mouseMoveEvent(QMouseEvent *event) {
    if(event->buttons() & Qt::LeftButton) {
        mGradientWidget->moveColor(event->x());
    }
    mHoveredX = event->x();
    update();
}

void CurrentGradientWidget::leaveEvent(QEvent *) {
    mHoveredX = -1;
    update();
}
