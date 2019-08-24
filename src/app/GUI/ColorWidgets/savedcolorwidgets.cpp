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

#include "savedcolorwidgets.h"
#include "savedcolorswidget.h"

SavedColorWidgets::SavedColorWidgets(QWidget *parent) : QWidget(parent)
{
    main_layout = new QVBoxLayout(this);
    setLayout(main_layout);

    widgets_layout = new QVBoxLayout();
    main_layout->addLayout(widgets_layout);

    add_collection_button = new QPushButton("New QColor Collection", this);
    connect(add_collection_button, SIGNAL(pressed()), this, SLOT(addCollection()) );
    main_layout->addWidget(add_collection_button);
}

void SavedColorWidgets::addCollection()
{
    widgets_layout->addWidget(new SavedColorsWidget(this) );
}

