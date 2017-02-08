#include "boxscrollwidgetvisiblepart.h"
#include "boxsinglewidget.h"

BoxScrollWidgetVisiblePart::BoxScrollWidgetVisiblePart(
        ScrollWidget *parent) :
    ScrollWidgetVisiblePart(parent) {

}

SingleWidget *BoxScrollWidgetVisiblePart::createNewSingleWidget() {
    return new BoxSingleWidget(this);
}
