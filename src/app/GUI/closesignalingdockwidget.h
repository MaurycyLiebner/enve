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

#ifndef CLOSESIGNALINGDOCKWIDGET_H
#define CLOSESIGNALINGDOCKWIDGET_H
#include <QDockWidget>

class CloseSignalingDockWidget : public QDockWidget {
    Q_OBJECT
public:
    CloseSignalingDockWidget(const QString& name, QWidget *parent);
protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
signals:
    void madeVisible(const bool visible);
};

#endif // CLOSESIGNALINGDOCKWIDGET_H
