#ifndef BRUSHSETTINGSWIDGET_H
#define BRUSHSETTINGSWIDGET_H

#include <QWidget>
#include "../PaintLib/brush.h"
#include "brushsettingwidget.h"
class ColorSettingsWidget;
class ColorSetting;

class BrushSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BrushSettingsWidget(QWidget *parent = 0);
    void setBrushWidgetSetting(const BrushSetting &setting_id,
                               const qreal &val_t,
                               const bool &edited_t);
    float getBrushSetting(const BrushSetting &settind_id);
    void incBrushRadius();
    void decBrushRadius();
    void setCurrentBrush(Brush *brushT);
    void revertSettingToDefault(const BrushSetting &setting_t);
    bool hasSettingDefaultVal(const BrushSetting &setting_t);
    Brush *getCurrentBrush();
signals:

public slots:
    void setBrushSetting(const BrushSetting &setting_id,
                         const qreal &val_t);
    void showHideAdvancedSettings();
    void openBrushSaveDialog();
    void overwriteBrushSettings();
    void setColorSetting(const ColorSetting &colorSetting);
private:
    ColorSettingsWidget *mColorSettingsWidget;
    QHBoxLayout *buttons_layout = NULL;
    QPushButton *save_brush_button = NULL;
    QPushButton *overwrite_brush_settings = NULL;
    QPushButton *advanced_button = NULL;

    bool advanced_settings_visible = false;
    Brush *mCurrentBrush = NULL;
    QList<BrushSettingWidget*> setting_widgets;
    QVBoxLayout *main_layout = NULL;
    QHBoxLayout *h_layout = NULL;
    QVBoxLayout *labels_layout = NULL;
    QVBoxLayout *rest_layout = NULL;
};

#endif // BRUSHSETTINGSWIDGET_H
