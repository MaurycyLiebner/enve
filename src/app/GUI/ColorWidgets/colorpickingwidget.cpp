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

#include "colorpickingwidget.h"
#include <QMouseEvent>
#include <QPainter>
#include "GUI/mainwindow.h"
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include "colorsettingswidget.h"
#include <QDebug>
#include "GUI/global.h"
#include <QWindow>

ColorPickingWidget::ColorPickingWidget(QWidget * const parent)
    : QWidget(parent) {
    QScreen * screen = nullptr;
    if(const auto window = windowHandle())
        screen = window->screen();
    else screen = QGuiApplication::primaryScreen();
    if(!screen) return;

    mScreenshot = screen->grabWindow(0).toImage();

    QPixmap picker(":/cursors/cursor_color_picker.png");
    setCursor(QCursor(picker, 2, 20));

    setMouseTracking(true);

    setWindowFlag(Qt::SplashScreen);
    setAttribute(Qt::WA_DeleteOnClose);

    showFullScreen();
    updateBox(QCursor::pos());
    setFocus();
}

void ColorPickingWidget::mouseReleaseEvent(QMouseEvent *e) {
    if(e->button() == Qt::RightButton) {
        close();
    } else if(e->button() == Qt::LeftButton) {
        emit colorSelected(mCurrentColor);
        Document::sInstance->actionFinished();
        close();
    }
}

void ColorPickingWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.drawImage(0, 0, mScreenshot);
    p.fillRect(mCursorX + 2*eSizesUI::widget - 4,
               mCursorY - eSizesUI::widget - 4,
               eSizesUI::widget + 8,
               eSizesUI::widget + 8, Qt::black);
    p.fillRect(mCursorX + 2*eSizesUI::widget - 2,
               mCursorY - eSizesUI::widget - 2,
               eSizesUI::widget + 4,
               eSizesUI::widget + 4, Qt::white);
    p.fillRect(mCursorX + 2*eSizesUI::widget,
               mCursorY - eSizesUI::widget,
               eSizesUI::widget, eSizesUI::widget,
               mCurrentColor);
    p.end();
}

void ColorPickingWidget::keyPressEvent(QKeyEvent *e) {
    if(e->isAutoRepeat()) return;
    close();
}

void ColorPickingWidget::mouseMoveEvent(QMouseEvent *e) {
    updateBox(e->globalPos());
}

void ColorPickingWidget::focusOutEvent(QFocusEvent*) {
    close();
}

QColor ColorPickingWidget::colorFromPoint(const int x, const int y) {
    const QRgb rgb = mScreenshot.pixel(x, y);
    return QColor::fromRgb(rgb);
}

void ColorPickingWidget::updateBox(const QPoint& globalPos) {
    mCursorX = globalPos.x();
    mCursorY = globalPos.y();
    mCurrentColor = colorFromPoint(globalPos.x(), globalPos.y());
    update();
}

