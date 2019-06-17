#ifndef STACKWIDGETWRAPPER_H
#define STACKWIDGETWRAPPER_H

#include <QWidget>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QMainWindow>
class StackWrapperMenu;

class StackWidgetWrapper : public QWidget {
public:
    typedef std::function<void(StackWidgetWrapper*)> SetupOp;
    explicit StackWidgetWrapper(const SetupOp &setup,
                                QWidget * const parent = nullptr);

    void setMenuBar(StackWrapperMenu * const menu);
    void setCentralWidget(QWidget * const widget);

    void splitH();
    void splitV();
private:
    template <class T>
    void split() {
        if(!parentWidget()) return;
        const auto layout = parentWidget()->layout();
        const auto window = qobject_cast<QMainWindow*>(parentWidget());
        if(!layout && !window) return;
        const auto stack = new T(parentWidget());
        if(window) {
            window->takeCentralWidget();
            window->setCentralWidget(stack);
        } else layout->replaceWidget(this, stack, Qt::FindDirectChildrenOnly);
        stack->appendWidget(this);
        const auto newWid = new StackWidgetWrapper(mSetupOp, stack);
        stack->appendWidget(newWid);
    }

    const SetupOp mSetupOp;
    QVBoxLayout* mLayout;
    StackWrapperMenu* mMenuBar = nullptr;
    QWidget* mCenterWidget = nullptr;
};

class StackWrapperMenu : public QWidget {
    friend class StackWidgetWrapper;
protected:
    explicit StackWrapperMenu();

    void addWidget(QWidget * const widget);
private:
    void setParent(StackWidgetWrapper * const parent);

    QHBoxLayout* mMenuBarLayout = nullptr;
    QHBoxLayout* mLayout = nullptr;

    StackWidgetWrapper * mParent = nullptr;

    QAction * mSplitV = nullptr;
    QAction * mSplitH = nullptr;
    QAction * mClose = nullptr;
};

#endif // STACKWIDGETWRAPPER_H
