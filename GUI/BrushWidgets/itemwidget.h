#ifndef ITEMWIDGET_H
#define ITEMWIDGET_H
#include <QWidget>
#include "selfref.h"
#include "tablethoverfix.h"

class ItemWidgetQObject : public QWidget {
    Q_OBJECT
public:
    ItemWidgetQObject(QWidget* parent) : QWidget(parent) {}

    void setSelected(const bool& bT) {
        mSelected = bT;
        update();
    }

    void setHovered(const bool& bT) {
        mHovered = bT;
        update();
    }

    const bool& isSelected() const { return mSelected; }
    const bool& isHovered() const { return mHovered; }
protected:
    bool mSelected = false;
    bool mHovered = false;
signals:
    void selected(StdSelfRef*);
    void rightPressed(StdSelfRef*, QPoint);
};

template <class Item>
class ItemWidget : public ItemWidgetQObject {
public:
    static ItemWidget *createWidget(const sptr<Item>& itemSptr,
                                    QWidget *parent);
    ~ItemWidget() {
        if(mSelected) emit selected(nullptr);
        if(mItem == nullptr) return;
        mItem->removeItemWidget(this);
    }

    Item* getItem() {
        return mItem.get();
    }
protected:
    ItemWidget(const sptr<Item>& item, QWidget *parent = nullptr);
    bool event(QEvent *event);
    void paintEvent(QPaintEvent*);
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);
    void mousePressEvent(QMouseEvent *event);
#if TABLET_MOUSE_SYNTH == TABLET_DEFAULT_MOUSE_SYNTHESIZE
    void tabletEvent(QTabletEvent* event);
#endif
private:
    bool mDrawName = false;
    sptr<Item> mItem;
};
#include "itemwidget.cpp"

#endif // ITEMWIDGET_H
