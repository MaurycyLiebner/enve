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

#ifndef ACTIONBUTTON_H
#define ACTIONBUTTON_H

#include <QWidget>

class ActionButton : public QWidget
{
    Q_OBJECT
public:
    explicit ActionButton(const QString &notCheckedPix,
                          const QString &toolTip,
                          QWidget *parent = nullptr);
    void setIcon(const QString &notCheckedPix);
    void setCheckable(const QString &checkedPix);
    void setChecked(bool checked);
    void toggle();
    bool checked() { return mChecked; }
protected:
    void mouseReleaseEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

    void paintEvent(QPaintEvent *);

    bool mHover = false;
    bool mChecked = false;
    bool mCheckable = false;

    QImage mCheckedPixmap;
    QImage mNotCheckedPixmap;
signals:
    void released();
    void pressed();
    void toggled(bool);
public slots:
};

#endif // ACTIONBUTTON_H
