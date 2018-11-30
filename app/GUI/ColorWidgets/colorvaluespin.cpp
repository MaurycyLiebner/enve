#include "colorvaluespin.h"
#include "GUI/qrealanimatorvalueslider.h"

ColorValueSpin::ColorValueSpin(QWidget *parent)
    : QObject(parent)
{
    mSpinBox = new QrealAnimatorValueSlider(0., 1., 0.1, parent);
    //spin_box->setRange(min_val_t, max_val_t);
    //spin_box->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(mSpinBox, SIGNAL(valueChanged(double) ),
            this, SLOT(spinBoxValSet(double)) );
    connect(mSpinBox, SIGNAL(displayedValueChanged(qreal)),
            this, SLOT(spinBoxDisplayedValueChanged(qreal)));
}

ColorValueSpin::~ColorValueSpin()
{
    delete mSpinBox;
}

QrealAnimatorValueSlider *ColorValueSpin::getSpinBox()
{
    return mSpinBox;
}

void ColorValueSpin::setDisplayedValue(GLfloat val_t) {
    if(mBlockValue/* || mSpinBox->hasTargetAnimator()*/) return;
    //mEmit = false;
//    mSpinBox->setValue(
//                qRound(val_t*(mSpinBox->maximum() - mSpinBox->minimum()) +
//                       mSpinBox->minimum()) );
    mSpinBox->setDisplayedValue(val_t);
    //mEmit = true;
}

void ColorValueSpin::setValueExternal(const GLfloat &val) {
    mSpinBox->setValueExternal(val);
}

void ColorValueSpin::spinBoxDisplayedValueChanged(const qreal &val) {
    emit displayedValueChanged(val);
}

void ColorValueSpin::spinBoxValSet(double spin_box_val_t)
{
    mBlockValue = true;
    emit valSet( ( (GLfloat)spin_box_val_t)/*/mSpinBox->maximum()*/ );
    mBlockValue = false;
}
