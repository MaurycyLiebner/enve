#include "colorvaluespin.h"
#include "qrealanimatorvalueslider.h"

ColorValueSpin::ColorValueSpin(int min_val_t, int max_val_t, int val_t, QWidget *parent)
    : QObject(parent)
{
    mSpinBox = new QrealAnimatorValueSlider(min_val_t, max_val_t, 1., parent);
    //spin_box->setRange(min_val_t, max_val_t);
    mSpinBox->setValue(val_t);
    //spin_box->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(mSpinBox, SIGNAL(valueChanged(double) ),
            this, SLOT(spinBoxValSet(double)) );
}

ColorValueSpin::~ColorValueSpin()
{
    delete mSpinBox;
}

QrealAnimatorValueSlider *ColorValueSpin::getSpinBox()
{
    return mSpinBox;
}

void ColorValueSpin::setVal(GLfloat val_t) {
    if(mBlockValue || mSpinBox->hasTargetAnimator()) return;
    mEmit = false;
//    mSpinBox->setValue(
//                qRound(val_t*(mSpinBox->maximum() - mSpinBox->minimum()) +
//                       mSpinBox->minimum()) );
    mSpinBox->setValue(val_t);
    mEmit = true;
}

void ColorValueSpin::spinBoxValSet(double spin_box_val_t)
{
    if(mEmit) {
        mBlockValue = true;
        emit valSet( ( (GLfloat)spin_box_val_t)/*/mSpinBox->maximum()*/ );
        mBlockValue = false;
    }
}
