#include "ewidgets.h"
#include <QtCore>

eWidgets* eWidgets::sInstance = nullptr;

eWidgets::eWidgets() {
    Q_ASSERT(!sInstance);
    sInstance = this;
}
