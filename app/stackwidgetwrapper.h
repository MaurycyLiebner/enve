#ifndef STACKWIDGETWRAPPER_H
#define STACKWIDGETWRAPPER_H

#include "GUI/widgetstack.h"

#include <QMenuBar>
#include <QVBoxLayout>
#include <QMainWindow>
#include <memory>
class StackWidgetWrapper;

class StackWrapperMenu : public QMenuBar {
    friend class StackWidgetWrapper;
protected:
    explicit StackWrapperMenu();
private:
    void setParent(StackWidgetWrapper * const parent);

    void disableClose() {
        mClose->setVisible(false);
    }

    void enableClose() {
        mClose->setVisible(true);
    }

    StackWidgetWrapper * mParent = nullptr;

    QAction * mSplitV = nullptr;
    QAction * mSplitH = nullptr;
    QAction * mClose = nullptr;
};

struct WidgetStackLayoutItem;
class StackWidgetWrapper : public QWidget {
public:
    typedef std::function<void(StackWidgetWrapper*)> SetupOp;
    typedef std::function<StackWidgetWrapper*(WidgetStackLayoutItem* const layoutItem,
                                              QWidget * const parent)> Creator;
    typedef std::function<std::unique_ptr<WidgetStackLayoutItem>()> LayoutItemCreator;
    explicit StackWidgetWrapper(WidgetStackLayoutItem* const layoutItem,
                                const LayoutItemCreator& layoutItemCreator,
                                const Creator& creator, const SetupOp& setup,
                                QWidget* const parent = nullptr);

    StackWrapperMenu* getMenuBar() { return mMenuBar; }
    QWidget* getCentralWidget() { return mCenterWidget; }

    void setMenuBar(StackWrapperMenu * const menu);
    void setCentralWidget(QWidget * const widget);

    StackWidgetWrapper* splitH();
    StackWidgetWrapper* splitV();

    void closeWrapper();
    void disableClose();

    WidgetStackLayoutItem* getLayoutItem() const {
        return mLayoutItem;
    }
private:
    template <class T>
    StackWidgetWrapper* split(
            WidgetStackLayoutItem* const otherLayoutItem);

    WidgetStackLayoutItem* const mLayoutItem;
    const LayoutItemCreator mLayoutItemCreator;
    const Creator mCreator;
    const SetupOp mSetupOp;
    QVBoxLayout* mLayout;
    StackWrapperMenu* mMenuBar = nullptr;
    QWidget* mCenterWidget = nullptr;
};

template <class T>
StackWidgetWrapper* StackWidgetWrapper::split(
        WidgetStackLayoutItem * const otherLayoutItem) {
    if(mMenuBar) mMenuBar->enableClose();
    const auto stack = new T(parentWidget());
    gReplaceWidget(this, stack);
    stack->appendWidget(this);
    const auto newWid = mCreator(otherLayoutItem, stack);
    stack->appendWidget(newWid);
    return newWid;
}

struct ParentStackLayoutItem;
struct SplitStackLayoutItem;
struct StackLayoutItem {
    typedef std::unique_ptr<StackLayoutItem> UniPtr;
    typedef std::unique_ptr<SplitStackLayoutItem> SplitPtr;
    typedef std::unique_ptr<WidgetStackLayoutItem> WidgetPtr;
    virtual ~StackLayoutItem() {}
    virtual void apply(StackWidgetWrapper* const stack) const = 0;
    void setParent(ParentStackLayoutItem* const parent) {
        mParent = parent;
    }
protected:
    ParentStackLayoutItem* mParent = nullptr;
};

struct ParentStackLayoutItem : StackLayoutItem {
    virtual void childClosed_k(StackLayoutItem* const child) = 0;
    virtual UniPtr replaceChild(StackLayoutItem* const from,
                                UniPtr&& to) = 0;
};

struct SplitStackLayoutItem : public ParentStackLayoutItem {
    void setChildren(UniPtr&& child1, UniPtr&& child2) {
        if(child1) child1->setParent(this);
        if(child2) child2->setParent(this);
        mChildItems.first = std::move(child1);
        mChildItems.second = std::move(child2);
    }

    void apply(StackWidgetWrapper* const stack) const {
        const auto other = split(stack);
        if(mChildItems.first) mChildItems.first->apply(stack);
        if(mChildItems.second) mChildItems.second->apply(other);
    }

    void childClosed_k(StackLayoutItem* const child) {
        if(mChildItems.first.get() == child) {
            mParent->replaceChild(this, std::move(mChildItems.second));
        } else if(mChildItems.second.get() == child) {
            mParent->replaceChild(this, std::move(mChildItems.first));
        }
    }

    UniPtr replaceChild(StackLayoutItem* const from,
                        UniPtr&& to) {
        if(mChildItems.first.get() == from) {
            UniPtr tmp = std::move(mChildItems.first);
            if(from) from->setParent(nullptr);
            if(to) to->setParent(this);
            mChildItems.first = std::move(to);
            return tmp;
        } else if(mChildItems.second.get() == from) {
            UniPtr tmp = std::move(mChildItems.second);
            if(from) from->setParent(nullptr);
            if(to) to->setParent(this);
            mChildItems.second = std::move(to);
            return tmp;
        }
        return nullptr;
    }
protected:
    virtual StackWidgetWrapper* split(StackWidgetWrapper* const stack) const = 0;
private:
    std::pair<UniPtr, UniPtr> mChildItems;
};

struct SplittableStackItem : StackLayoutItem {
    void splitV(WidgetPtr&& other);
    SplitPtr splitV(WidgetPtr &&thisUni, WidgetPtr &&other);

    void splitH(WidgetPtr&& other);
    SplitPtr splitH(WidgetPtr &&thisUni, WidgetPtr &&other);
private:
    template<class SplitItemClass>
    void split(WidgetPtr&& other);
    template<class SplitItemClass>
    SplitPtr split(WidgetPtr &&thisUni, WidgetPtr &&other);
};

struct WidgetStackLayoutItem : public SplittableStackItem {
    virtual void clear() = 0;
    void close() {
        if(!mParent) return clear();
        mParent->childClosed_k(this);
    }
};

struct BaseStackItem : public ParentStackLayoutItem {
    typedef std::unique_ptr<BaseStackItem> UPtr;
    void apply(StackWidgetWrapper* const stack) const {
        if(mChild) mChild->apply(stack);
    }

    void childClosed_k(StackLayoutItem* const child) {
        Q_UNUSED(child);
        return;
    }

    UniPtr replaceChild(StackLayoutItem* const from,
                        UniPtr&& to) {
        if(from != mChild.get()) return nullptr;
        return setChild(std::move(to));
    }

    UniPtr setChild(UniPtr&& to) {
        UniPtr tmp = std::move(mChild);
        if(tmp) tmp->setParent(nullptr);
        mChild = std::move(to);
        if(mChild) mChild->setParent(this);
        return tmp;
    }

    void setName(const QString& name) const {
        mName = name;
    }

    const QString& getName() const {
        return mName;
    }
private:
    mutable QString mName;
    UniPtr mChild;
};

struct HSplitStackItem : public SplitStackLayoutItem {
protected:
    StackWidgetWrapper* split(StackWidgetWrapper* const stack) const {
        return stack->splitH();
    }
};

struct VSplitStackItem : public SplitStackLayoutItem {
protected:
    StackWidgetWrapper* split(StackWidgetWrapper* const stack) const {
        return stack->splitV();
    }
};
#endif // STACKWIDGETWRAPPER_H
