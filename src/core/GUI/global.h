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

#ifndef GLOBAL_H
#define GLOBAL_H

extern int FONT_HEIGHT;
extern int MIN_WIDGET_DIM;
extern int BUTTON_DIM;
extern int KEY_RECT_SIZE;

#include <QPixmap>

extern QPixmap* ALPHA_MESH_PIX;

#include <QString>

QString gSingleLineTooltip(const QString& text);
QString gSingleLineTooltip(const QString& text, const QString& shortcut);

#endif // GLOBAL_H
