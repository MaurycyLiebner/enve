#ifndef BRUSHSETTINGSWIDGET_H
#define BRUSHSETTINGSWIDGET_H

#include <QWidget>
#include "../PaintLib/brush.h"
#include "brushsettingwidget.h"

struct WindowVariables;

class BrushButton;

class BrushSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BrushSettingsWidget(WindowVariables *window_vars_t, QWidget *parent = 0);
    void setBrushSetting(BrushSetting setting_id, float val_t, bool edited_t);
    float getBrushSetting(BrushSetting settind_id);
    void incBrushRadius();
    void decBrushRadius();
    void setBrushButton(BrushButton *button_t);
    void setButtonSetting(BrushSetting setting_id, float val_t);
    void revertSettingToDefault(BrushSetting setting_t);
    bool hasSettingDefaultVal(BrushSetting setting_t);
    BrushButton *getCurrentButton();

signals:

public slots:
    void showHideAdvancedSettings();
    void openBrushSaveDialog();
    void overwriteBrushSettings();
private:
    QHBoxLayout *buttons_layout = NULL;
    QPushButton *save_brush_button = NULL;
    QPushButton *overwrite_brush_settings = NULL;
    QPushButton *advanced_button = NULL;

    bool advanced_settings_visible = false;
    BrushButton *current_button = NULL;
    WindowVariables *window_vars = NULL;
    QList<BrushSettingWidget*> setting_widgets;
    QVBoxLayout *main_layout = NULL;
    QHBoxLayout *h_layout = NULL;
    QVBoxLayout *labels_layout = NULL;
    QVBoxLayout *rest_layout = NULL;
};

#endif // BRUSHSETTINGSWIDGET_H
