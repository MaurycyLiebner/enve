#ifndef WINDOWVARIABLES_H
#define WINDOWVARIABLES_H
#include "Colors/color.h"

class ColorButton;
class BrushesWidget;
class Canvas;
class TreeWidget;
class MainWindow;
class TreeWidgetBackground;
class TreePropertiesPanel;
class BrushSettingsWidget;
class BrushButton;

#include "PaintLib/brush.h"

struct WindowVariables {
    Brush *current_brush = new Brush();
    Color current_color;
    BrushSettingsWidget *brush_settings_widget = NULL;
    bool brush_saved = true;
    bool color_saved = true;

    bool color_picking = false;

    bool erasing = false;

    void updateBrushColor();
    void setBrushColor(GLfloat h_t, GLfloat s_t, GLfloat v_t);
    void currentColorChanged();

    Brush *getBrush();
    void setBrushButton(BrushButton *button_t);
    void setBrushSetting(BrushSetting setting_t, float val_t);
    void revertToDefault(BrushSetting setting_t);
    float getBrushSetting(BrushSetting setting_t);
    float getBrushRadius();
    void setBrushRadius(float radius_t);
    void setBrushHardness(float hardness_t);
    void setBrushOpacity(float opacity_t);
    void incBrushRadius();
    void decBrushRadius();
    bool hasSettingDefaultVal(BrushSetting setting_t);
    void saveColorIfNeeded();
    void setBrushSaved(bool b_t);
    void saveBrushIfNeeded();
    void saveBrushAndColorIfNeeded();
    void setErasing(bool b_t);
};


#endif // WINDOWVARIABLES_H
