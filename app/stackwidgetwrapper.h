#ifndef STACKWIDGETWRAPPER_H
#define STACKWIDGETWRAPPER_H

#include "GUI/widgetstack.h"

#include <QMenuBar>
#include <QVBoxLayout>
#include <QMainWindow>
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

class StackWidgetWrapper : public QWidget {
public:
    typedef std::function<void(StackWidgetWrapper*)> SetupOp;
    explicit StackWidgetWrapper(const SetupOp &setup,
                                QWidget * const parent = nullptr);

    void setMenuBar(StackWrapperMenu * const menu);
    void setCentralWidget(QWidget * const widget);

    void splitH();
    void splitV();

    void closeWrapper();
    void disableClose() {
        if(mMenuBar) mMenuBar->disableClose();
    }
private:
    template <class T>
    void split();

    const SetupOp mSetupOp;
    QVBoxLayout* mLayout;
    StackWrapperMenu* mMenuBar = nullptr;
    QWidget* mCenterWidget = nullptr;
};

template <class T>
void StackWidgetWrapper::split() {
    if(mMenuBar) mMenuBar->enableClose();
    const auto stack = new T(parentWidget());

    if(!gReplaceWidget(this, stack)) {
        delete stack;
        return;
    }

    stack->appendWidget(this);
    const auto newWid = new StackWidgetWrapper(mSetupOp, stack);
    stack->appendWidget(newWid);
}

#endif // STACKWIDGETWRAPPER_H
