#include "brushsettingswidget.h"
#include "Colors/helpers.h"
#include <QPushButton>

BrushSettingsWidget::BrushSettingsWidget(QWidget *parent)
    : QWidget(parent) {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    main_layout = new QVBoxLayout(this);

    h_layout = new QHBoxLayout();
    main_layout->addLayout(h_layout);
    labels_layout = new QVBoxLayout();
    rest_layout = new QVBoxLayout();
    h_layout->addLayout(labels_layout);
    h_layout->addLayout(rest_layout);
    for(int i = 0; i < BRUSH_SETTINGS_COUNT; i++)
    {
        BrushSettingWidget *setting_widget_t =
                new BrushSettingWidget(labels_layout, rest_layout,
                                       static_cast<BrushSetting>(i),
                                       this);
        setting_widgets.append(setting_widget_t);
        setting_widget_t->hide();
        connect(setting_widget_t, SIGNAL(setBrushSettings(BrushSetting,qreal)),
                this, SLOT(setBrushSetting(BrushSetting,qreal)));
    }
    setting_widgets.at(BRUSH_SETTING_RADIUS)->show();
    setting_widgets.at(BRUSH_SETTING_HARDNESS)->show();
    setting_widgets.at(BRUSH_SETTING_OPACITY)->show();
    //setting_widgets.at(BRUSH_SETTING_SLOW_TRACKING)->show();

    buttons_layout = new QHBoxLayout();
    save_brush_button = new QPushButton("Save Brush...", this);
    connect(save_brush_button, SIGNAL(released()),
            this, SLOT(openBrushSaveDialog()) );
    buttons_layout->addWidget(save_brush_button);

    overwrite_brush_settings = new QPushButton("Overwrite Brush Settings");
    connect(overwrite_brush_settings, SIGNAL(released()),
            this, SLOT(overwriteBrushSettings()) );
    buttons_layout->addWidget(overwrite_brush_settings);

    advanced_button = new QPushButton("Advanced Settings", this);
    connect(advanced_button, SIGNAL(released()),
            this, SLOT(showHideAdvancedSettings()) );
    buttons_layout->addWidget(advanced_button);
    main_layout->addLayout(buttons_layout);


    setLayout(main_layout);
}

void BrushSettingsWidget::setBrushWidgetSetting(const BrushSetting &setting_id,
                                                const qreal &val_t,
                                                const bool &edited_t) {
    setting_widgets.at(setting_id)->setVal(val_t, edited_t);
}

void BrushSettingsWidget::setCurrentBrush(Brush *brushT) {
    mCurrentBrush = brushT;
    for(int i = 0; i < BRUSH_SETTINGS_COUNT; i++) {
        BrushSetting id_t = static_cast<BrushSetting>(i);
        setBrushWidgetSetting(id_t,
                        brushT->getSettingVal(id_t),
                        false);
    }
}

void BrushSettingsWidget::setBrushSetting(const BrushSetting &setting_id,
                                          const qreal &val_t) {
    mCurrentBrush->setSetting(setting_id, val_t);
}

void BrushSettingsWidget::revertSettingToDefault(const BrushSetting &setting_t) {
    setBrushWidgetSetting(setting_t,
                    mCurrentBrush->getBrushSettingInfo(setting_t)->def,
                    false);
}

bool BrushSettingsWidget::hasSettingDefaultVal(const BrushSetting &setting_t) {
    return true;
}

Brush *BrushSettingsWidget::getCurrentBrush() {
    return mCurrentBrush;
}

void BrushSettingsWidget::showHideAdvancedSettings() {
    advanced_settings_visible = !advanced_settings_visible;
    for(int i = BRUSH_SETTING_ALPHA; i < BRUSH_SETTINGS_COUNT; i++)
    {
        setting_widgets.at(i)->setVisible(advanced_settings_visible);
    }
}

void BrushSettingsWidget::openBrushSaveDialog() {
    //new BrushSaveDialog(window_vars);
}

void BrushSettingsWidget::overwriteBrushSettings() {
    //mCurrentBrush->setDefaultSettingsFromCurrent();
    saveBrushDataAsFile(mCurrentBrush,
                        mCurrentBrush->getCollectionName(),
                        mCurrentBrush->getBrushName());
    for(int i = 0; i < BRUSH_SETTINGS_COUNT; i++) {
        BrushSetting id_t = static_cast<BrushSetting>(i);
        setting_widgets.at(i)->setDefaultButtonEnabled(false);
    }
}

float BrushSettingsWidget::getBrushSetting(const BrushSetting &settind_id) {
    return setting_widgets.at(settind_id)->getVal();
}

void BrushSettingsWidget::incBrushRadius() {
    BrushSettingWidget *setting_widget_t =
            setting_widgets.at(BRUSH_SETTING_RADIUS);
    setting_widget_t->incVal(mCurrentBrush->getRadius()*0.2f + 0.3f);
}

void BrushSettingsWidget::decBrushRadius() {
    BrushSettingWidget *setting_widget_t =
            setting_widgets.at(BRUSH_SETTING_RADIUS);
    setting_widget_t->incVal(-(mCurrentBrush->getRadius()*0.2f + 0.3f) );
}
