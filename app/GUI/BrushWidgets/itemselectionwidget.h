#ifndef ITEMSELECTIONWIDGET_H
#define ITEMSELECTIONWIDGET_H

template <typename Item>
class CollectionArea;
#include <QTabWidget>
#include "itemwidget.h"
#include "smartPointers/sharedpointerdefs.h"
class FlowLayout;
class CollectionAreaQObject;

class ItemSelectionWidgetQObject : public QTabWidget {
    Q_OBJECT
public:
    template <typename Item> friend class ItemSelectionWidget;
    ItemSelectionWidgetQObject(QWidget* parent = nullptr) :
        QTabWidget(parent) {
        setSizePolicy(QSizePolicy::Preferred, sizePolicy().verticalPolicy());
        connect(this, &QTabWidget::tabBarClicked,
                this, &ItemSelectionWidgetQObject::pressedIndex);
    }

    QSize sizeHint() const {
        if(_mSavedWidth == -1) return QTabWidget::sizeHint();
        return QSize(_mSavedWidth, QTabWidget::sizeHint().height());
    }

    void setRightPressedFunction(std::function<void(ItemSelectionWidgetQObject*,
                                                    CollectionAreaQObject*,
                                                    StdSelfRef*,
                                                    QPoint)> rightPressedFunction) {
        mRightPressedFunction = rightPressedFunction;
    }
private slots:
    void itemSelectedSlot(StdSelfRef* item) {
        itemSelected(item);
    }

    void pressedIndex(int index) {
        if(index == -1) return;
        _mSavedWidth = width();
    }
protected slots:
    void rightPressedSlot(CollectionAreaQObject* srcColl,
                          StdSelfRef* pressedItemWidget,
                          QPoint globalPos) {
        mRightPressedFunction(this, srcColl, pressedItemWidget, globalPos);
    }
protected:
    void mousePressEvent(QMouseEvent* event) {
        if(event->button() == Qt::RightButton) {
            mRightPressedFunction(this, getCurrentCollectionAreaQObject(),
                                  nullptr, event->globalPos());
        }
    }
    virtual CollectionAreaQObject* getCurrentCollectionAreaQObject() = 0;
private:
    int _mSavedWidth = -1;
    std::function<void(ItemSelectionWidgetQObject*,
                       CollectionAreaQObject*,
                       StdSelfRef*,
                       QPoint)> mRightPressedFunction;
    virtual void itemSelected(StdSelfRef* item) = 0;
};

template <typename Item>
class ItemSelectionWidget : public ItemSelectionWidgetQObject {
public:
    ItemSelectionWidget(QWidget* parent = nullptr) :
        ItemSelectionWidgetQObject(parent) {}
    ~ItemSelectionWidget() {
        if(mCurrentItem != nullptr) mCurrentItem->deselect();
    }

    Item* getCurrentItem() const {
        return mCurrentItem;
    }

    Item* getItem(const QString &collectionName,
                  const QString &itemName) const;

    void setDefaultItem(stdsptr<Item> item) {
        if(mCurrentItem == mDefaultItem) mCurrentItem = nullptr;
        mDefaultItem = item;
        if(mCurrentItem == nullptr) mCurrentItem = mDefaultItem;
    }

    CollectionArea<Item>* getCurrentCollectionArea() {
        if(currentIndex() == -1) return nullptr;
        return mChildCollections.at(currentIndex());
    }
protected:
    CollectionAreaQObject* getCurrentCollectionAreaQObject() {
        if(currentIndex() == -1) return nullptr;
        return mChildCollections.at(currentIndex());
    }

    CollectionArea<Item>* getItemCollectionWithName(const QString& name) const;

    void setCurrentItem(Item* item) {
        if(mCurrentItem) mCurrentItem->deselect();
        if(!item) item = mDefaultItem.get();
        if(!item) return;
        mCurrentItem = item;
        emitCurrentItemChanged(item);
        mCurrentItem->select();
    }

    virtual void emitCurrentItemChanged(Item* item) = 0;

    void addChildCollection(CollectionArea<Item>* collection,
                            const QString& name);
    void removeChildCollection(CollectionArea<Item> *collection);

    QList<CollectionArea<Item>*> mChildCollections;
private:
    void itemSelected(StdSelfRef* item) {
        setCurrentItem(StdSelfRef::getAsPtr<Item>(item));
    }

    StdPointer<Item> mCurrentItem;
    stdsptr<Item> mDefaultItem;
};

#include <QDir>
#include "flowlayout.h"
#include "collectionarea.h"

template <typename Item>
Item* ItemSelectionWidget<Item>::getItem(const QString &collectionName,
                                         const QString &itemName) const {
    CollectionArea<Item>* itemColl = getItemCollectionWithName(collectionName);
    if(itemColl == nullptr) return nullptr;
    return itemColl->getItemWithName(itemName);
}
template <typename Item>
CollectionArea<Item> *ItemSelectionWidget<Item>::getItemCollectionWithName(
        const QString &name) const {
    for(CollectionArea<Item>* collection : mChildCollections) {
        if(collection->getName() == name) return collection;
    }
    return nullptr;
}

template <typename Item>
void ItemSelectionWidget<Item>::addChildCollection(CollectionArea<Item> *collection,
                                                   const QString &name) {
    mChildCollections.append(collection);
    addTab(collection, name);
    connect(collection, &CollectionArea<Item>::currentItemChanged,
            this, &ItemSelectionWidgetQObject::itemSelectedSlot);
    connect(collection, &CollectionArea<Item>::rightPressed,
            this, &ItemSelectionWidgetQObject::rightPressedSlot);
}


template <typename Item>
void ItemSelectionWidget<Item>::removeChildCollection(CollectionArea<Item> *collection) {
    removeTab(mChildCollections.indexOf(collection));
    mChildCollections.removeOne(collection);
}

#endif // ITEMSELECTIONWIDGET_H
