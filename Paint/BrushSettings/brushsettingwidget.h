#ifndef BRUSHSETTINGWIDGET_H
#define BRUSHSETTINGWIDGET_H

#include <QObject>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include "../PaintLib/brush.h"
#include <QPushButton>

struct WindowVariables;

class BrushSettingWidget : public QObject
{
    Q_OBJECT
public:
    explicit BrushSettingWidget(QVBoxLayout *label_layout_t, QVBoxLayout *rest_layout_t,
                                BrushSetting setting_id_t,
                                WindowVariables *window_vars_t,
                                QWidget *parent = 0);
    float getVal();
    void setVal(float val_t, bool edited_t);

    void valChanged();

    void hide();
    void show();
    void setVisible(bool b_t);

    void setDefaultButtonEnabled(bool b_t);
signals:

public slots:
    void setSpinVal(int val_t);
    void setSliderVal(double val_t);
    void incVal(float inc_t);
    void revertToDefaultVal();
private:
    QHBoxLayout *h_layout = NULL;
    QPushButton *default_button = NULL;
    const BrushSettingInfo *setting_info = NULL;
    WindowVariables *window_vars = NULL;
    QLabel *name_label = NULL;
    QDoubleSpinBox *spin_box = NULL;
    QSlider *val_slider = NULL;
    BrushSetting setting_id;
};

#endif // BRUSHSETTINGWIDGET_H
