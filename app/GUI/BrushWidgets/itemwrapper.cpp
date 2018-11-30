#include "itemwrapper.h"

ItemTarget::ItemTarget(const QString &name,
                       const QString &collectionName,
                       const QImage &icon) {
    mCollectionName = collectionName;
    mName = name;
    mIcon = icon;
}

const QImage &ItemTarget::getIcon() {
    return mIcon;
}

void ItemTarget::setSelected(const bool &select) {
    if(select == mSelected) return;
    mSelected = select;
    for(ItemWidgetQObject* widget : mItemWidgets) {
        widget->setSelected(select);
    }
}

void ItemTarget::select() {
    setSelected(true);
}

void ItemTarget::deselect() {
    setSelected(false);
}

void ItemTarget::addItemWidget(ItemWidgetQObject *widget) {
    mItemWidgets << widget;
}

void ItemTarget::removeItemWidget(ItemWidgetQObject *widget) {
    mItemWidgets.removeOne(widget);
}

const QString &ItemTarget::getName() const {
    return mName;
}

const QString &ItemTarget::getCollectionName() const {
    return mCollectionName;
}

void ItemTarget::setCollectionName(const QString &collName) {
    mCollectionName = collName;
}

void ItemTarget::setName(const QString &name) {
    mName = name;
}

void ItemTarget::setIcon(const QImage &icon) {
    mIcon = icon;
    for(ItemWidgetQObject* widget : mItemWidgets) {
        widget->update();
    }
}

const bool &ItemTarget::selected() const {
    return mSelected;}
