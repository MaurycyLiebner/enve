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

#include "canvaswindow.h"
#include "canvas.h"
#include "GUI/global.h"
#include "Private/document.h"

QPointF CanvasWindow::mapToCanvasCoord(const QPointF& windowCoord) {
    return mViewTransform.inverted().map(windowCoord);
}

void CanvasWindow::translateView(const QPointF &trans) {
    if(!mCurrentCanvas) return;
    mViewTransform.translate(trans.x(), trans.y());
}

void CanvasWindow::zoomView(const qreal scaleBy, const QPointF &absOrigin) {
    if(!mCurrentCanvas) return;
    const QPointF transPoint = -mapToCanvasCoord(absOrigin);

    mViewTransform.translate(-transPoint.x(), -transPoint.y());
    mViewTransform.scale(scaleBy, scaleBy);
    mViewTransform.translate(transPoint.x(), transPoint.y());
}

#include <QResizeEvent>
void CanvasWindow::resizeEvent(QResizeEvent *e) {
    if(e->size().isValid()) {
        if(mOldSize.isValid()) {
            const auto dSize = e->size() - mOldSize;
            const qreal div = 2*mViewTransform.m11();
            const QPointF trans{dSize.width()/div, dSize.height()/div};
            translateView(trans);
        }
        // e->oldSize() returns {-1, -1} after chaining parent
        mOldSize = e->size();
    }
    GLWindow::resizeEvent(e);
}

void CanvasWindow::fitCanvasToSize() {
    if(mFitToSizeBlocked) {
        mFitToSizeBlocked = false;
        return;
    }
    if(!mCurrentCanvas) return;
    mViewTransform.reset();
    const auto canvasSize = mCurrentCanvas->getCanvasSize();
    const qreal widWidth = width();
    const qreal widHeight = height();
    const qreal widthScale = (widWidth - eSizesUI::widget)/canvasSize.width();
    const qreal heightScale = (widHeight - eSizesUI::widget)/canvasSize.height();
    const qreal minScale = qMin(widthScale, heightScale);
    translateView({(widWidth - canvasSize.width()*minScale)*0.5,
                   (widHeight - canvasSize.height()*minScale)*0.5});
    mViewTransform.scale(minScale, minScale);
}

void CanvasWindow::zoomInView() {
    if(!mCurrentCanvas) return;
    const auto canvasSize = mCurrentCanvas->getCanvasSize();
    mViewTransform.translate(canvasSize.width()*0.5, canvasSize.height()*0.5);
    mViewTransform.scale(1.1, 1.1);
    mViewTransform.translate(-canvasSize.width()*0.5, -canvasSize.height()*0.5);
}

void CanvasWindow::zoomOutView() {
    if(!mCurrentCanvas) return;
    const auto canvasSize = mCurrentCanvas->getCanvasSize();
    mViewTransform.translate(canvasSize.width()*0.5, canvasSize.height()*0.5);
    mViewTransform.scale(0.9, 0.9);
    mViewTransform.translate(-canvasSize.width()*0.5, -canvasSize.height()*0.5);
}

#include <QEvent>

bool CanvasWindow::event(QEvent *e) {
    if(e->type() == QEvent::ShowToParent) fitCanvasToSize();
    else if(e->type() == QEvent::Show) KFT_setFocus();
    return QWidget::event(e);
}

void CanvasWindow::hideEvent(QHideEvent *e) {
    GLWindow::hideEvent(e);
    if(mCurrentCanvas)
        mDocument.removeVisibleScene(mCurrentCanvas);
}

void CanvasWindow::showEvent(QShowEvent *e) {
    GLWindow::showEvent(e);
    if(mCurrentCanvas)
        mDocument.addVisibleScene(mCurrentCanvas);
}

void CanvasWindow::resetTransormation() {
    if(!mCurrentCanvas) return;
    mViewTransform.reset();
    translateView({(width() - mCurrentCanvas->getCanvasWidth())*0.5,
                   (height() - mCurrentCanvas->getCanvasHeight())*0.5});
}
