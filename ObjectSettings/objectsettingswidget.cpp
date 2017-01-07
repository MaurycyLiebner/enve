#include "objectsettingswidget.h"
#include "BoxesList/boxitemwidgetcontainer.h"

ObjectSettingsWidget::ObjectSettingsWidget(QWidget *parent) :
    BoxesListWidget(parent) {

}

void ObjectSettingsWidget::setBoundingBox(BoundingBox *box) {
    if(mCurrentBox != NULL) {
        removeItemForBox(mCurrentBox);
    }

    mCurrentBox = box;
    if(box == NULL) return;
    addItemForBox(box);

    mBoxWidgetContainers.last()->setDetailsVisible(true);
}
