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

#include "brushlabel.h"
#include "Paint/brushcontexedwrapper.h"
#include "GUI/global.h"
#include "brushselectionwidget.h"
#include "Private/document.h"
#include <QMenu>

BrushLabel::BrushLabel(BrushesContext* const ctxt) : TriggerLabel(nullptr) {
    connect(this, &TriggerLabel::requestContextMenu,
            this, [this, ctxt](const QPoint& pos) {
        const auto brush = Document::sInstance->fBrush;
        if(!brush) return;
        QMenu menu(this);
        menu.addAction("Bookmark");
        const auto act = menu.exec(pos);
        if(act) {
            if(act->text() == "Bookmark") {
                const auto wrapper = ctxt->brushWrapper(brush);
                if(wrapper) wrapper->bookmark();
            }
        }
    });
    setStyleSheet("QWidget {"
                      "background: white;"
                  "}");
    setFixedSize(48, 48);
}

void BrushLabel::setBrush(BrushContexedWrapper * const brush) {
    if(!brush) {
        QPixmap pix(width(), height());
        pix.fill(Qt::white);
        setPixmap(pix);
        return;
    }
    const auto& icon = brush->getBrushData().fIcon;
    const auto img = icon.scaled(width(), height(), Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation);
    setPixmap(QPixmap::fromImage(img));
}
