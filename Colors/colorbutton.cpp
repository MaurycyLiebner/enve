#include "colorbutton.h"
#include <QPainter>
#include "mainwindow.h"
#include "ColorWidgets/colorsettingswidget.h"

ColorButton::ColorButton(Color color_p_t,
                         QWidget *parent_t) : QWidget(parent_t)
{
    setFixedSize(30, 30);
    color_p = color_p_t;
    color_settings_widget = new ColorSettingsWidget(color_p.gl_h, color_p.gl_s, color_p.gl_v);
    color_settings_widget->hide();
    color_settings_widget->resize(450, 250);
    connect(color_settings_widget, SIGNAL(colorChangedHSVSignal(GLfloat,GLfloat,GLfloat)),
            this, SLOT(setColorHSV(GLfloat,GLfloat,GLfloat)) );
    connect(color_settings_widget, SIGNAL(hideThis()), this, SLOT(switchSelected() ) );
}

void ColorButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QColor col_t = MainWindow::getColor(false, selected, hover).qcol;
    p.fillRect(rect(), col_t);
    p.translate(5, 5);
    p.fillRect(0, 0, 20, 20, Qt::black);
    p.fillRect(2, 2, 16, 16, Qt::white);
    p.fillRect(4, 4, 12, 12, color_p.qcol);
    p.end();
}

void ColorButton::mousePressEvent(QMouseEvent *)
{
    openCloseColorWidget();
}

void ColorButton::enterEvent(QEvent *)
{
    hover = true;
    repaint();
}

void ColorButton::leaveEvent(QEvent *)
{
    hover = false;
    repaint();
}

void ColorButton::setSelected(bool b_t)
{
    selected = b_t;
    repaint();
}

void ColorButton::openCloseColorWidget()
{
    switchSelected();
    if(color_settings_widget->isHidden() )
    {
        color_settings_widget->show();
        color_settings_widget->setLastColorHSV(color_p.gl_h, color_p.gl_s, color_p.gl_v);
        //color_settings_widget->move(mapTo(window_vars->getMainWindow(), QPoint(0, height()) ) );
    }
    else
    {
        color_settings_widget->hide();
    }
}

void ColorButton::updateColorSettingsWidgetCurrentColor()
{
    color_settings_widget->setCurrentColor(color_p.gl_h, color_p.gl_s, color_p.gl_v);
}

void ColorButton::setColorHSV(GLfloat h, GLfloat s, GLfloat v)
{
    color_p.setHSV(h, s, v);
    emit colorSet(h, s, v);
    update();
}

void ColorButton::switchSelected()
{
    setSelected(!selected );
}
