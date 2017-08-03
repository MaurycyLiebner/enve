#include "brushsettingswidget.h"
#include "Colors/helpers.h"
#include "../BrushesWidget/brushbutton.h"
#include "../PaintLib/brushsavedialog.h"
#include <QPushButton>

BrushSettingsWidget::BrushSettingsWidget(WindowVariables *window_vars_t, QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    main_layout = new QVBoxLayout(this);

    h_layout = new QHBoxLayout();
    main_layout->addLayout(h_layout);
    labels_layout = new QVBoxLayout();
    rest_layout = new QVBoxLayout();
    h_layout->addLayout(labels_layout);
    h_layout->addLayout(rest_layout);
    window_vars = window_vars_t;
    for(int i = 0; i < BRUSH_SETTINGS_COUNT; i++)
    {
        BrushSettingWidget *setting_widget_t =
                new BrushSettingWidget(labels_layout, rest_layout,
                                       static_cast<BrushSetting>(i), window_vars, this);
        setting_widgets.append(setting_widget_t);
        setting_widget_t->hide();
    }
    setting_widgets.at(BRUSH_SETTING_RADIUS)->show();
    setting_widgets.at(BRUSH_SETTING_HARDNESS)->show();
    setting_widgets.at(BRUSH_SETTING_OPACITY)->show();
    //setting_widgets.at(BRUSH_SETTING_SLOW_TRACKING)->show();

    buttons_layout = new QHBoxLayout();
    save_brush_button = new QPushButton("Save Brush...", this);
    connect(save_brush_button, SIGNAL(released()), this, SLOT(openBrushSaveDialog()) );
    buttons_layout->addWidget(save_brush_button);

    overwrite_brush_settings = new QPushButton("Overwrite Brush Settings");
    connect(overwrite_brush_settings, SIGNAL(released()), this, SLOT(overwriteBrushSettings()) );
    buttons_layout->addWidget(overwrite_brush_settings);

    advanced_button = new QPushButton("Advanced Settings", this);
    connect(advanced_button, SIGNAL(released()), this, SLOT(showHideAdvancedSettings()) );
    buttons_layout->addWidget(advanced_button);
    main_layout->addLayout(buttons_layout);


    setLayout(main_layout);
}

void BrushSettingsWidget::setBrushSetting(BrushSetting setting_id, float val_t, bool edited_t)
{
    setting_widgets.at(setting_id)->setVal(val_t, edited_t);
}

void BrushSettingsWidget::setBrushButton(BrushButton *button_t)
{
    current_button = button_t;
    for(int i = 0; i < BRUSH_SETTINGS_COUNT; i++)
    {
        BrushSetting id_t = static_cast<BrushSetting>(i);
        setBrushSetting(id_t, button_t->getBrushSetting(id_t), button_t->getEdited(id_t) );
    }
}

void BrushSettingsWidget::setButtonSetting(BrushSetting setting_id, float val_t)
{
    current_button->setBrushSetting(setting_id, val_t);
}

void BrushSettingsWidget::revertSettingToDefault(BrushSetting setting_t)
{
    setBrushSetting(setting_t, current_button->revertSettingToDefault(setting_t), false);
}

bool BrushSettingsWidget::hasSettingDefaultVal(BrushSetting setting_t)
{
    return current_button->getEdited(setting_t);
}

BrushButton *BrushSettingsWidget::getCurrentButton()
{
    return current_button;
}

void BrushSettingsWidget::showHideAdvancedSettings()
{
    advanced_settings_visible = !advanced_settings_visible;
    for(int i = BRUSH_SETTING_ALPHA; i < BRUSH_SETTINGS_COUNT; i++)
    {
        setting_widgets.at(i)->setVisible(advanced_settings_visible);
    }
}

void BrushSettingsWidget::openBrushSaveDialog()
{
    new BrushSaveDialog(window_vars);
}

void BrushSettingsWidget::overwriteBrushSettings()
{
    Brush *current_brush_t = window_vars->getBrush();
    current_button->setDefaultSettingsFromCurrent();
    saveBrushDataAsFile(current_brush_t,
                        current_brush_t->getCollectionName(),
                        current_brush_t->getBrushName());
    for(int i = 0; i < BRUSH_SETTINGS_COUNT; i++)
    {
        BrushSetting id_t = static_cast<BrushSetting>(i);
        setting_widgets.at(i)->setDefaultButtonEnabled( current_button->getEdited(id_t) );
    }
}

float BrushSettingsWidget::getBrushSetting(BrushSetting settind_id)
{
    return setting_widgets.at(settind_id)->getVal();
}

void BrushSettingsWidget::incBrushRadius()
{
    BrushSettingWidget *setting_widget_t =
            setting_widgets.at(BRUSH_SETTING_RADIUS);
    setting_widget_t->incVal(window_vars->getBrushRadius()*0.2f + 0.3f);
}

void BrushSettingsWidget::decBrushRadius()
{
    BrushSettingWidget *setting_widget_t =
            setting_widgets.at(BRUSH_SETTING_RADIUS);
    setting_widget_t->incVal(-(window_vars->getBrushRadius()*0.2f + 0.3f) );
}
