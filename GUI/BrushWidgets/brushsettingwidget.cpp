#include "brushsettingwidget.h"
#include <QtMath>

BrushSettingWidget::BrushSettingWidget(QVBoxLayout *label_layout_t,
                                       QVBoxLayout *rest_layout_t,
                                       BrushSetting setting_id_t,
                                       QWidget *parent)
    : QObject(parent) {
    mTargetSetting = setting_id_t;
    mSettingInfo = Brush::getBrushSettingInfo(mTargetSetting);
    float min_t = mSettingInfo->min;
    float max_t = mSettingInfo->max;
    mNameLabel = new QLabel(mSettingInfo->name);
    mValSpinBox = new QDoubleSpinBox();
    mValSpinBox->setRange(min_t, max_t);
    float single_step_t = (max_t - min_t)*0.05;
    mValSpinBox->setSingleStep( single_step_t );
    mValSpinBox->setDecimals( qCeil(qLn(single_step_t )/qLn(0.1) ) );
    mValSlider = new QSlider(Qt::Horizontal);
    mValSlider->setSizePolicy(QSizePolicy::MinimumExpanding,
                              QSizePolicy::Maximum);
    mValSlider->setRange(min_t*1000, max_t*1000);
    mDefaultButton = new QPushButton("X");
    mDefaultButton->setFixedWidth(30);
    label_layout_t->addWidget(mNameLabel, Qt::AlignLeft);
    mMainLayout = new QHBoxLayout();
    mMainLayout->addWidget(mValSpinBox);
    mMainLayout->addWidget(mValSlider);
    mMainLayout->addWidget(mDefaultButton);
    rest_layout_t->addLayout(mMainLayout);
    connect(mValSlider, SIGNAL(valueChanged(int)),
            this, SLOT(setSpinVal(int)) );
    connect(mValSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(setSliderVal(double)) );
    connect(mDefaultButton, SIGNAL(pressed()),
            this, SLOT(revertToDefaultVal() ));
}

qreal BrushSettingWidget::getVal() {
    return mValSlider->value()*0.001;
}

void BrushSettingWidget::setVal(const qreal &val_t,
                                const bool &edited_t) {
    mValSpinBox->setValue(val_t);
    mValSlider->setValue(val_t*1000);
    mDefaultButton->setEnabled(edited_t);
}

void BrushSettingWidget::setSpinVal(const int &val_t) {
    mValSpinBox->setValue(val_t*0.001);
    valChanged();
}

void BrushSettingWidget::setSliderVal(double val_t) {
    mValSlider->setValue(val_t*1000);
    valChanged();
}

void BrushSettingWidget::incVal(const qreal &inc_t) {
    qreal val_t = getVal() + inc_t;
    mValSpinBox->setValue(val_t);
    mValSlider->setValue(val_t*1000);
    valChanged();
}

void BrushSettingWidget::revertToDefaultVal() {
    //window_vars->revertToDefault(setting_id);
}

void BrushSettingWidget::valChanged() {
     emit setBrushSetting(mTargetSetting, getVal());
     mDefaultButton->setEnabled(false);
}

void BrushSettingWidget::hide() {
    mNameLabel->hide();
    mValSpinBox->hide();
    mValSlider->hide();
    mDefaultButton->hide();
}

void BrushSettingWidget::show() {
    mNameLabel->show();
    mValSpinBox->show();
    mValSlider->show();
    mDefaultButton->show();
}

void BrushSettingWidget::setVisible(const bool &b_t) {
    if(b_t) {
        show();
    } else {
        hide();
    }
}

void BrushSettingWidget::setDefaultButtonEnabled(const bool &b_t) {
    mDefaultButton->setEnabled(b_t);
}

