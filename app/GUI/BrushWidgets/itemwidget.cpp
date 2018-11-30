#include "itemwidget.h"

#include <QFile>
#include <QPainter>
#include <QMouseEvent>
#include <QToolTip>
#include "sharedpointerdefs.h"

template <class Item>
bool ItemWidget<Item>::event(QEvent *event) {
    if(event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        QToolTip::showText(helpEvent->globalPos(), mItem->getName());

        return true;
    }
    return QWidget::event(event);
}

template <class Item>
ItemWidget<Item>::ItemWidget(const sptr<Item>& itemSptr,
                             QWidget *parent) : ItemWidgetQObject(parent) {
    setFocusPolicy(Qt::NoFocus);
    mItem = itemSptr;

    setFixedSize(64, 64);

    if(mItem == nullptr) return;
    mItem->addItemWidget(this);
}

template <class Item>
void ItemWidget<Item>::paintEvent(QPaintEvent *) {
    QPainter p(this);

    if(mItem != nullptr) {
        const QImage& icon = mItem->getIcon();
        p.drawImage((64 - icon.width())/2, (64 - icon.height())/2, icon);
        if(mDrawName) {
            p.drawText(rect(),
                       Qt::AlignHCenter | Qt::AlignBottom |
                       Qt::TextWrapAnywhere,
                       mItem->getName());
        }
    }
    if(isSelected()) {
        p.setPen(QPen(Qt::red, 4., Qt::SolidLine,
                      Qt::SquareCap, Qt::MiterJoin));
        p.drawRect(2, 2, 60, 60);
    } else if(isHovered()) {
        p.setPen(QPen(Qt::red, 2., Qt::SolidLine,
                      Qt::SquareCap, Qt::MiterJoin));
        p.drawRect(1, 1, 62, 62);
    } else {
        p.setPen(QPen(Qt::darkGray, 1., Qt::SolidLine,
                      Qt::SquareCap, Qt::MiterJoin));
        p.drawRect(0, 0, 63, 63);
    }

    p.end();
}

template <class Item>
void ItemWidget<Item>::enterEvent(QEvent*) {
    setHovered(true);
}

template <class Item>
void ItemWidget<Item>::leaveEvent(QEvent*) {
    setHovered(false);
}

template <class Item>
void ItemWidget<Item>::mousePressEvent(QMouseEvent *event) {
    if(mItem == nullptr) return;
    if(event->button() == Qt::LeftButton) {
        if(isSelected()) return;
        emit selected(GetAsPtr(mItem, StdSelfRef));
    } else if(event->button() == Qt::RightButton) {
        emit rightPressed(GetAsPtr(mItem, StdSelfRef),
                          event->globalPos());
    }
}

//template <class Item>
//void CollectionWidget::tabletEvent(QTabletEvent *event) {
//    if(mItem == nullptr) return;
//    if(event->type() == QTabletEvent::TabletPress) {
//        if(event->button() == Qt::LeftButton) {
//            if(mSelected) return;
//            emit selected(mItem);
//        }
//    }
//}

template <class Item>
ItemWidget<Item> *ItemWidget<Item>::createWidget(const sptr<Item>& itemSptr,
                                                 QWidget *parent) {
    if(itemSptr == nullptr) return nullptr;
    ItemWidget* newWidget = new ItemWidget(itemSptr, parent);
    newWidget->setSelected(itemSptr->selected());
    return newWidget;
}
