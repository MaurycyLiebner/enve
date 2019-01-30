#include "namedcontainer.h"

NamedContainer::NamedContainer(const QString& name, QWidget *widget,
                               const bool &vertical,
                               QWidget *parent) : QWidget(parent) {
    mNameLabel = new QLabel("<center>" + name + "</center>", this);
    mNameLabel->setObjectName("dockLabel");
    if(vertical) {
        mLayout = new QVBoxLayout(this);
    } else {
        mLayout = new QHBoxLayout(this);
    }
    mLayout->setSpacing(0);
    mLayout->addWidget(mNameLabel);
    mLayout->addWidget(widget);
    setLayout(mLayout);
}
