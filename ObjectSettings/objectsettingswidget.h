#ifndef OBJECTSETTINGSWIDGET_H
#define OBJECTSETTINGSWIDGET_H
#include "BoxesList/boxeslistwidget.h"

class ObjectSettingsWidget : public BoxesListWidget
{
    Q_OBJECT
public:
    explicit ObjectSettingsWidget(QWidget *parent = 0);
    void setBoundingBox(BoundingBox *box);
private:
    BoundingBox *mCurrentBox = NULL;
signals:

public slots:
};

#endif // OBJECTSETTINGSWIDGET_H
