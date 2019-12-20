#include "triggerlabel.h"
#include <QMouseEvent>

void TriggerLabel::mousePressEvent(QMouseEvent *ev) {
    if(ev->button() == Qt::LeftButton)
        emit triggered();
}
