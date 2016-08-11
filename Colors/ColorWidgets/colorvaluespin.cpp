#include "colorvaluespin.h"

ColorValueSpin::ColorValueSpin(int min_val_t, int max_val_t, int val_t, QWidget *parent)
    : QObject(parent)
{
    spin_box = new QSpinBox(parent);
    spin_box->setRange(min_val_t, max_val_t);
    spin_box->setValue(val_t);
    spin_box->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(spin_box, SIGNAL(valueChanged(int) ), this, SLOT(spinBoxValSet(int)) );
}

ColorValueSpin::~ColorValueSpin()
{
    delete spin_box;
}

QSpinBox *ColorValueSpin::getSpinBox()
{
    return spin_box;
}

void ColorValueSpin::setVal(GLfloat val_t)
{
    disconnect(spin_box, SIGNAL(valueChanged(int) ), this, SLOT(spinBoxValSet(int)) );
    spin_box->setValue(val_t*(spin_box->maximum() - spin_box->minimum()) + spin_box->minimum() );
    connect(spin_box, SIGNAL(valueChanged(int) ), this, SLOT(spinBoxValSet(int)) );
}

void ColorValueSpin::spinBoxValSet(int spin_box_val_t)
{
    emit valSet( ( (GLfloat)spin_box_val_t)/spin_box->maximum() );
}
