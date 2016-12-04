#include "colorvaluespin.h"
#include "qdoubleslider.h"

ColorValueSpin::ColorValueSpin(int min_val_t, int max_val_t, int val_t, QWidget *parent)
    : QObject(parent)
{
    spin_box = new QDoubleSlider(min_val_t, max_val_t, 1., parent);
    //spin_box->setRange(min_val_t, max_val_t);
    spin_box->setValue(val_t);
    //spin_box->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(spin_box, SIGNAL(valueChanged(double) ), this, SLOT(spinBoxValSet(double)) );
}

ColorValueSpin::~ColorValueSpin()
{
    delete spin_box;
}

QDoubleSlider *ColorValueSpin::getSpinBox()
{
    return spin_box;
}

void ColorValueSpin::setVal(GLfloat val_t)
{
    if(mBlockValue) return;
    mEmit = false;
    spin_box->setValue(qRound(val_t*(spin_box->maximum() - spin_box->minimum()) + spin_box->minimum()) );
    mEmit = true;
}

void ColorValueSpin::spinBoxValSet(double spin_box_val_t)
{
    if(mEmit) {
        mBlockValue = true;
        emit valSet( ( (GLfloat)spin_box_val_t)/spin_box->maximum() );
        mBlockValue = false;
    }
}
