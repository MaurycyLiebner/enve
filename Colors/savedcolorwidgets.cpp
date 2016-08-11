#include "savedcolorwidgets.h"
#include "savedcolorswidget.h"

SavedColorWidgets::SavedColorWidgets(QWidget *parent) : QWidget(parent)
{
    main_layout = new QVBoxLayout(this);
    setLayout(main_layout);

    widgets_layout = new QVBoxLayout();
    main_layout->addLayout(widgets_layout);

    add_collection_button = new QPushButton("New Color Collection", this);
    connect(add_collection_button, SIGNAL(pressed()), this, SLOT(addCollection()) );
    main_layout->addWidget(add_collection_button);
}

void SavedColorWidgets::addCollection()
{
    widgets_layout->addWidget(new SavedColorsWidget(this) );
}

