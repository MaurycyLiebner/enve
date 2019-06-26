#ifndef STACKWIDGETWRAPPER_H
#define STACKWIDGETWRAPPER_H

#include "GUI/widgetstack.h"
#include "basicreadwrite.h"

#include <QMenuBar>
#include <QVBoxLayout>
#include <QMainWindow>
#include <memory>
class StackWidgetWrapper;

class StackWrapperCornerMenu : public QMenuBar {
public:
    StackWrapperCornerMenu();

    void setTarget(StackWidgetWrapper * const target);

    void disableClose() {
        mClose->setVisible(false);
    }

    void enableClose() {
        mClose->setVisible(true);
    }
private:
    QAction * mSplitV = nullptr;
    QAction * mSplitH = nullptr;
    QAction * mClose = nullptr;

    StackWidgetWrapper * mTarget = nullptr;
};

class StackWrapperMenu : public QMenuBar {
    friend class StackWidgetWrapper;
protected:
    explicit StackWrapperMenu();
private:
    void setTarget(StackWidgetWrapper * const target);
    void disableClose() {
        mCornerMenu->disableClose();
    }

    void enableClose() {
        mCornerMenu->enableClose();
    }

    StackWrapperCornerMenu* mCornerMenu;
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

    StackWrapperMenu* getMenuBar() const { return mMenuBar; }
    QWidget* getCentralWidget() const { return mCenterWidget; }

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
    enum Type {
        V_SPLIT, H_SPLIT,
        WIDGET, NONE
    };

    typedef std::unique_ptr<StackLayoutItem> UniPtr;
    typedef std::unique_ptr<SplitStackLayoutItem> SplitPtr;
    typedef std::unique_ptr<WidgetStackLayoutItem> WidgetPtr;
    virtual ~StackLayoutItem() {}
    virtual void apply(StackWidgetWrapper* const stack) const = 0;
    virtual void write(QIODevice* const dst) const = 0;
    void writeType(QIODevice* const dst) const {
        dst->write(rcConstChar(&mType), sizeof(Type));
    }

    void setParent(ParentStackLayoutItem* const parent) {
        mParent = parent;
    }
protected:
    Type mType = NONE;
    ParentStackLayoutItem* mParent = nullptr;
};

struct ParentStackLayoutItem : public StackLayoutItem {
    virtual void childClosed_k(StackLayoutItem* const child) = 0;
    virtual UniPtr replaceChild(StackLayoutItem* const from,
                                UniPtr&& to) = 0;
protected:
    template <typename WidgetT>
    static UniPtr sReadChild(QIODevice* const src);
    static void sWriteChild(StackLayoutItem* const child,
                            QIODevice* const dst);
};

struct SplitStackLayoutItem : public ParentStackLayoutItem {
    void write(QIODevice* const dst) const {
        sWriteChild(mChildItems.first.get(), dst);
        sWriteChild(mChildItems.second.get(), dst);
    }

    template <typename WidgetT>
    void read(QIODevice* const src) {
        auto child1 = sReadChild<WidgetT>(src);
        auto child2 = sReadChild<WidgetT>(src);
        setChildren(std::move(child1), std::move(child2));
    }

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
    WidgetStackLayoutItem() { mType = Type::WIDGET; }

    virtual void clear() = 0;
    virtual void read(QIODevice* const src) = 0;
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

    StackLayoutItem* getChild() const {
        return mChild.get();
    }

    void setName(const QString& name) const {
        mName = name;
    }

    const QString& getName() const {
        return mName;
    }

    void write(QIODevice* const dst) const {
        gWrite(dst, mName);
        ParentStackLayoutItem::sWriteChild(mChild.get(), dst);
    }

    template<typename WidgetT>
    static UPtr sRead(QIODevice* const src) {
        const QString name = gReadString(src);
        const auto result = new BaseStackItem;
        result->setName(name);
        auto child = ParentStackLayoutItem::sReadChild<WidgetT>(src);
        result->setChild(std::move(child));
        return UPtr(result);
    }
private:
    mutable QString mName;
    UniPtr mChild;
};

struct HSplitStackItem : public SplitStackLayoutItem {
public:
    HSplitStackItem() { mType = Type::H_SPLIT; }
protected:
    StackWidgetWrapper* split(StackWidgetWrapper* const stack) const {
        return stack->splitH();
    }
};

struct VSplitStackItem : public SplitStackLayoutItem {
public:
    VSplitStackItem() { mType = Type::V_SPLIT; }
protected:
    StackWidgetWrapper* split(StackWidgetWrapper* const stack) const {
        return stack->splitV();
    }
};

template <typename WidgetT>
StackLayoutItem::UniPtr ParentStackLayoutItem::sReadChild(
        QIODevice * const src) {
    Type type;
    src->read(rcChar(&type), sizeof(Type));
    if(type == Type::H_SPLIT) {
        const auto hSplit = new HSplitStackItem;
        hSplit->read<WidgetT>(src);
        return UniPtr(hSplit);
    } else if(type == Type::V_SPLIT) {
        const auto vSplit = new VSplitStackItem;
        vSplit->read<WidgetT>(src);
        return UniPtr(vSplit);
    } else if(type == Type::WIDGET) {
        const auto wid = new WidgetT;
        wid->read(src);
        return UniPtr(wid);
    } else Q_ASSERT(false);
}
#endif // STACKWIDGETWRAPPER_H
