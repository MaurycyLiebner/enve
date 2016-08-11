#ifndef SAVEDCOLORSWIDGET_H
#define SAVEDCOLORSWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include "color.h"

class WindowVariables;

class SavedColorsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SavedColorsWidget(QWidget *parent = 0);

    void addColorButton(Color color_t);
    void mousePressEvent(QMouseEvent *e);
private:
    QHBoxLayout *main_layout = NULL;
signals:

public slots:
    void setColorFromButton(Color color_t);
};

#endif // SAVEDCOLORSWIDGET_H
