#ifndef SAVEDCOLORSWIDGET_H
#define SAVEDCOLORSWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include "helpers.h"

class SavedColorsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SavedColorsWidget(QWidget *parent = nullptr);

    void addColorButton(const QColor& colorT);
    void mousePressEvent(QMouseEvent *e);
private:
    QHBoxLayout *mMainLayout = nullptr;
signals:

public slots:
    void setColorFromButton(const QColor &colorT);
};

#endif // SAVEDCOLORSWIDGET_H
