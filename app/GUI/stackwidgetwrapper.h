#ifndef STACKWIDGETWRAPPER_H
#define STACKWIDGETWRAPPER_H

#include "GUI/widgetstack.h"
#include "basicreadwrite.h"

#include <QMenuBar>
#include <QVBoxLayout>
#include <QMainWindow>
#include <memory>

class StackWidgetWrapper;
struct VSplitStackItem;
struct HSplitStackItem;

class StackWrapperCornerMenu : public QMenuBar {
public:
    StackWrapperCornerMenu(StackWidgetWrapper* const target);

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

    virtual void saveDataToLayout() const = 0;

    StackWrapperCornerMenu* getCornerMenu() {
        return mCornerMenu;
    }

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
    template <class T, class U>
    StackWidgetWrapper* split(
            U* const lItem,
            WidgetStackLayoutItem* const otherLayoutItem);

    StackWrapperCornerMenu* mCornerMenu;
    WidgetStackLayoutItem* const mLayoutItem;
    const LayoutItemCreator mLayoutItemCreator;
    const Creator mCreator;
    const SetupOp mSetupOp;
    QVBoxLayout* mLayout;
    StackWrapperMenu* mMenuBar = nullptr;
    QWidget* mCenterWidget = nullptr;
};

template <class T, class U>
StackWidgetWrapper* StackWidgetWrapper::split(
        U* const lItem,
        WidgetStackLayoutItem * const otherLayoutItem) {
    mCornerMenu->enableClose();
    const auto stack = new T(lItem, parentWidget());
    gReplaceWidget(this, stack);
    stack->appendWidget(this);
    const auto newWid = mCreator(otherLayoutItem, stack);
    stack->appendWidget(newWid);
    return newWid;
}

#endif // STACKWIDGETWRAPPER_H
