#ifndef COLORBUTTON_H
#define COLORBUTTON_H
#include <QWidget>
#include "GL/gl.h"
#include "Colors/color.h"

class ColorSettingsWidget;

class ColorButton : public QWidget
{
    Q_OBJECT
public:
    ColorButton(Color color_p_t, QWidget *parent_t = NULL);
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void setSelected(bool b_t);
    void openCloseColorWidget();
    void updateColorSettingsWidgetCurrentColor();
public slots:
    void setColorHSV(GLfloat h, GLfloat s, GLfloat v);
    void switchSelected();
signals:
    void colorSet(GLfloat, GLfloat, GLfloat);
private:
    ColorSettingsWidget *color_settings_widget = NULL;
    Color color_p;
    bool hover = false;
    bool selected = false;
};

#endif // COLORBUTTON_H
