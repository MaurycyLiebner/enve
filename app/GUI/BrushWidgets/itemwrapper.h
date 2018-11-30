#ifndef ITEMWRAPPER_H
#define ITEMWRAPPER_H
#include "selfref.h"
#include "itemwidget.h"

class ItemTarget : public StdSelfRef {
public:
    ItemTarget(const QString& name,
               const QString& collectionName,
               const QImage& icon = QImage());

    virtual const QImage& getIcon();

    void setSelected(const bool& select);

    void select();

    void deselect();

    void addItemWidget(ItemWidgetQObject* widget);

    void removeItemWidget(ItemWidgetQObject* widget);

    const QString& getName() const;
    const QString& getCollectionName() const;
    void setCollectionName(const QString& collName);

    void setName(const QString& name);

    void setIcon(const QImage& icon);

    const bool& selected() const;
private:
    bool mSelected = false;
    QString mName;
    QString mCollectionName;

    QImage mIcon;
    QList<ItemWidgetQObject*> mItemWidgets;
};

template <typename Item>
class ItemWrapper : public ItemTarget {
public:
    ItemWrapper(const QString& name,
                const QString& collectionName,
                Item item,
                const QImage& icon = QImage()) :
        ItemTarget(name, collectionName, icon) {
        mItem = item;
    }

    Item getItem() {
        return mItem;
    }
private:
    Item mItem;
};
#endif // ITEMWRAPPER_H
