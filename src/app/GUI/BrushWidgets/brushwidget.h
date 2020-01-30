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

#ifndef BRUSHWIDGET_H
#define BRUSHWIDGET_H

#include <QWidget>
#include "Paint/brushcontexedwrapper.h"

class BrushWidget : public QWidget {
    Q_OBJECT
public:
    explicit BrushWidget(BrushContexedWrapper * const brushCWrapper,
                         QWidget * const parent = nullptr);

    BrushContexedWrapper* getBrush() const
    { return mBrushCWrapper; }
protected:
    void paintEvent(QPaintEvent*);
    void enterEvent(QEvent*) {
        mHovered = true;
        update();
    }
    void leaveEvent(QEvent*) {
        mHovered = false;
        update();
    }

    void mousePressEvent(QMouseEvent* e);
signals:
    void selected(BrushContexedWrapper*);
    void triggered(BrushContexedWrapper*);
private:
    void selectionChanged(const bool sel) {
        if(sel) emit selected(mBrushCWrapper);
        update();
    }

    bool isSelected() const {
        return mBrushCWrapper->selected();
    }

    void deselect() {
        mBrushCWrapper->setSelected(false);
    }

    const qptr<BrushContexedWrapper> mBrushCWrapper;
    bool mHovered = false;
};

#endif // BRUSHWIDGET_H
