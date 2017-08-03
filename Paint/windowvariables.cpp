#include "windowvariables.h"
#include "BrushesWidget/brusheswidget.h"
#include "BrushesWidget/brushbutton.h"
#include "mainwindow.h"
#include "BrushSettings/brushsettingswidget.h"
#include <QtMath>

void WindowVariables::updateBrushColor()
{
    current_brush->setHSV(current_color.gl_h, current_color.gl_s, current_color.gl_v);
}

void WindowVariables::setBrushColor(GLfloat h_t, GLfloat s_t, GLfloat v_t)
{
    current_color.setHSV(h_t, s_t, v_t);
    updateBrushColor();
}

void WindowVariables::currentColorChanged()
{
    updateBrushColor();
}

void WindowVariables::setBrushesWidget(BrushesWidget *brushes_widget_t)
{
    brushes_widget = brushes_widget_t;
}

BrushesWidget *WindowVariables::getBrushesWidget()
{
    return brushes_widget;
}

Brush *WindowVariables::getBrush()
{
    return current_brush;
}

void WindowVariables::setBrushButton(BrushButton *button_t)
{
    button_t->setModifiedSettingsForBrush(current_brush);
    brush_settings_widget->setBrushButton(button_t);
    updateBrushColor();
    setBrushSaved(false);
}

void WindowVariables::setBrushSetting(BrushSetting setting_t, float val_t) {
    current_brush->setSetting(setting_t, val_t);
    brush_settings_widget->setButtonSetting(setting_t, val_t);
    setBrushSaved(false);
}

void WindowVariables::revertToDefault(BrushSetting setting_id_t)
{
    brush_settings_widget->revertSettingToDefault(setting_id_t);
}

float WindowVariables::getBrushSetting(BrushSetting setting_t)
{
    return current_brush->getSettingVal(setting_t);
}

float WindowVariables::getBrushRadius()
{
    return getBrushSetting(BRUSH_SETTING_RADIUS);
}

void WindowVariables::setBrushRadius(float radius_t)
{
    setBrushSetting(BRUSH_SETTING_RADIUS, qLn(radius_t));
}

void WindowVariables::setBrushHardness(float hardness_t)
{
    setBrushSetting(BRUSH_SETTING_HARDNESS, hardness_t);
}

void WindowVariables::setBrushOpacity(float opacity_t)
{
    setBrushSetting(BRUSH_SETTING_OPACITY, opacity_t);
}

void WindowVariables::incBrushRadius()
{
    brush_settings_widget->incBrushRadius();
}

void WindowVariables::decBrushRadius()
{
    brush_settings_widget->decBrushRadius();
}

bool WindowVariables::hasSettingDefaultVal(BrushSetting setting_t)
{
    return brush_settings_widget->hasSettingDefaultVal(setting_t);
}

void WindowVariables::saveColorIfNeeded()
{
    if(color_saved)
    {
        return;
    }

}

void WindowVariables::setBrushSaved(bool b_t)
{
    brush_saved = b_t;
}

void WindowVariables::saveBrushIfNeeded()
{
    if(brush_saved)
    {
        return;
    }

    current_brush->setChangedSinceLastStroke(true);
    setBrushSaved(true);
}

void WindowVariables::saveBrushAndColorIfNeeded()
{
    saveColorIfNeeded();
    saveBrushIfNeeded();
}

void WindowVariables::setErasing(bool b_t)
{
    erasing = b_t;
}
