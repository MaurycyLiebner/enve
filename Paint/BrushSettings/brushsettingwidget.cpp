#include "brushsettingwidget.h"
#include "../windowvariables.h"
#include <QtMath>

BrushSettingWidget::BrushSettingWidget(QVBoxLayout *label_layout_t,
                                       QVBoxLayout *rest_layout_t,
                                       BrushSetting setting_id_t,
                                       WindowVariables *window_vars_t,
                                       QWidget *parent)
    : QObject(parent) {
    window_vars = window_vars_t;
    setting_id = setting_id_t;
    setting_info = Brush::getBrushSettingInfo(setting_id);
    float min_t = setting_info->min;
    float max_t = setting_info->max;
    name_label = new QLabel(setting_info->name);
    spin_box = new QDoubleSpinBox();
    spin_box->setRange(min_t, max_t);
    float single_step_t = (max_t - min_t)*0.05;
    spin_box->setSingleStep( single_step_t );
    spin_box->setDecimals( qCeil(qLn(single_step_t )/qLn(0.1) ) );
    val_slider = new QSlider(Qt::Horizontal);
    val_slider->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    val_slider->setRange(min_t*1000, max_t*1000);
    default_button = new QPushButton("X");
    default_button->setFixedWidth(30);
    label_layout_t->addWidget(name_label, Qt::AlignLeft);
    h_layout = new QHBoxLayout();
    h_layout->addWidget(spin_box);
    h_layout->addWidget(val_slider);
    h_layout->addWidget(default_button);
    rest_layout_t->addLayout(h_layout);
    connect(val_slider, SIGNAL(valueChanged(int)), this, SLOT(setSpinVal(int)) );
    connect(spin_box, SIGNAL(valueChanged(double)), this, SLOT(setSliderVal(double)) );
    connect(default_button, SIGNAL(pressed()), this, SLOT(revertToDefaultVal() ));
}

float BrushSettingWidget::getVal()
{
    return val_slider->value()*0.001;
}

void BrushSettingWidget::setVal(float val_t, bool edited_t)
{
    spin_box->setValue(val_t);
    val_slider->setValue(val_t*1000);
    default_button->setEnabled(edited_t);
}

void BrushSettingWidget::setSpinVal(int val_t)
{
    spin_box->setValue(val_t*0.001);
    valChanged();
}

void BrushSettingWidget::setSliderVal(double val_t)
{
    val_slider->setValue(val_t*1000);
    valChanged();
}

void BrushSettingWidget::incVal(float inc_t)
{
    float val_t = getVal() + inc_t;
    spin_box->setValue(val_t);
    val_slider->setValue(val_t*1000);
    valChanged();
}

void BrushSettingWidget::revertToDefaultVal()
{
    window_vars->revertToDefault(setting_id);
}

void BrushSettingWidget::valChanged()
{
     window_vars->setBrushSetting(setting_id, getVal());
     default_button->setEnabled(window_vars->hasSettingDefaultVal(setting_id) );
}

void BrushSettingWidget::hide()
{
    name_label->hide();
    spin_box->hide();
    val_slider->hide();
    default_button->hide();
}

void BrushSettingWidget::show()
{
    name_label->show();
    spin_box->show();
    val_slider->show();
    default_button->show();
}

void BrushSettingWidget::setVisible(bool b_t)
{
    if(b_t)
    {
        show();
    }
    else
    {
        hide();
    }
}

void BrushSettingWidget::setDefaultButtonEnabled(bool b_t)
{
    default_button->setEnabled(b_t);
}

