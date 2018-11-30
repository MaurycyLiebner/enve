#ifndef COLLECTIONAREA_H
#define COLLECTIONAREA_H

#include <QMouseEvent>
#include <QScrollArea>
#include "tablethoverfix.h"
#include "selfref.h"
#include "itemwidget.h"
class QVBoxLayout;
class FlowLayout;
struct MyPaintItem;

class CollectionAreaQObject : public QScrollArea {
    Q_OBJECT
public:
    CollectionAreaQObject(QWidget* parent = nullptr) :
        QScrollArea(parent) {}
protected:
    void mousePressEvent(QMouseEvent* event) {
        if(event->button() == Qt::RightButton) {
            emit rightPressed(this, nullptr, event->globalPos());
        }
    }
signals:
    void currentItemChanged(StdSelfRef*);
    void rightPressed(CollectionAreaQObject*, StdSelfRef*, QPoint);
protected slots:
    void emitRightPressed(StdSelfRef* pressedItemWidget,
                          QPoint globalPos) {
        emit rightPressed(this, pressedItemWidget, globalPos);
    }
};

template <class Item>
class CollectionArea : public CollectionAreaQObject {
public:
    CollectionArea(const QString& name, QWidget* parent);

    const QString& getName() const { return mName; }
    Item *getItemWithName(const QString &name) const;
    void setName(const QString& name) { mName = name; }
    ItemWidget<Item>* getWidgetForItem(Item* item) const {
        for(ItemWidget<Item>* widget : mItemWidgets) {
            if(widget->getItem() == item) return widget;
        }
        return nullptr;
    }
    void addWidget(ItemWidget<Item>* widget);
protected:
    void removeWidget(ItemWidget<Item>* widget);
private:
    QString mName;
    QWidget *mLayoutWidget = nullptr;
    FlowLayout* mItemesLayout = nullptr;
    QList<ItemWidget<Item>*> mItemWidgets;
};

#include <QVBoxLayout>
#include <QScrollBar>
#include "flowlayout.h"
#include "itemwidget.h"

template <class Item>
CollectionArea<Item>::CollectionArea(const QString& name, QWidget *parent) :
        CollectionAreaQObject(parent) {
    setName(name);
    setFocusPolicy(Qt::NoFocus);
    verticalScrollBar()->setSingleStep(64);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setBackgroundRole(QPalette::Window);
    setFrameShadow(QFrame::Plain);
    setFrameShape(QFrame::NoFrame);
    setWidgetResizable(true);

    mLayoutWidget = new QWidget(this);
    mItemesLayout = new FlowLayout(this, 0, 0, 0);
    mLayoutWidget->setLayout(mItemesLayout);
    setWidget(mLayoutWidget);
}

template <class Item>
Item* CollectionArea<Item>::getItemWithName(const QString &name) const {
    for(ItemWidget<Item>* widget : mItemWidgets) {
        Item* itemSptr = widget->getItem();
        if(itemSptr == nullptr) continue;
        if(itemSptr->getName() != name) continue;
        return itemSptr;
    }
    return nullptr;
}

template <class Item>
void CollectionArea<Item>::addWidget(ItemWidget<Item> *widget) {
    mItemWidgets.append(widget);
    mItemesLayout->addWidget(widget);
    connect(widget, &ItemWidget<Item>::selected,
            this, &CollectionArea<Item>::currentItemChanged);
    connect(widget, &ItemWidget<Item>::rightPressed,
            this, &CollectionArea<Item>::emitRightPressed);
}

template <class Item>
void CollectionArea<Item>::removeWidget(ItemWidget<Item> *widget) {
    mItemWidgets.removeOne(widget);
    delete widget;
}
#endif // COLLECTIONAREA_H
