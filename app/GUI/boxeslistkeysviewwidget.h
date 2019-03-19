#ifndef BOXESLISTKEYSVIEWWIDGET_H
#define BOXESLISTKEYSVIEWWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMenuBar>
#include "smartPointers/stdselfref.h"

class SingleWidgetAbstraction;
class AnimationWidgetScrollBar;
class KeysView;
class BoxesListAnimationDockWidget;
class ChangeWidthWidget;
class MainWindow;
class ScrollArea;
class AnimationDockWidget;
class BoxScrollWidget;

class BoxesListKeysViewWidget : public QWidget {
    Q_OBJECT
public:
    explicit BoxesListKeysViewWidget(QWidget *topWidget,
                                     BoxesListAnimationDockWidget *animationDock,
                                     QWidget *parent);
    ~BoxesListKeysViewWidget();

    void setTopWidget(QWidget *topWidget);
    void connectToFrameWidget(AnimationWidgetScrollBar *frameRange);
    void connectToChangeWidthWidget(
            ChangeWidthWidget *changeWidthWidget);
    void setDisplayedFrameRange(const int& minFrame,
                             const int& maxFrame);
signals:
    void changedFrameRange(const int&, const int&);
public slots:
    void setRuleNone();
    void setRuleSelected();
    void setRuleAnimated();
    void setRuleNotAnimated();
    void setRuleVisible();
    void setRuleInvisible();
    void setRuleUnloced();
    void setRuleLocked();

    void setTargetAll();
    void setTargetCurrentCanvas();
    void setTargetCurrentGroup();

    void setSearchText(const QString &text);
    void moveSlider(int val);

    void setTypeSound();
    void setTypeAll();
    void setGraphEnabled(const bool &bT);
protected slots:
    void setBoxesListWidth(const int &width);

    void addNewBelowThis();
    void removeThis();
private:
    QHBoxLayout *mMainLayout;
    QVBoxLayout *mBoxesListLayout;
    QVBoxLayout *mKeysViewLayout;
    QHBoxLayout *mMenuLayout;
    QHBoxLayout *mMenuWidgetsLayout;
    QAction *mGraphAct = nullptr;
    QWidget *mMenuWidgetsCont;
    QWidget *mTopWidget = nullptr;
    QMenuBar *mBoxesListMenuBar;
    QMenuBar *mCornerMenuBar;
    QLineEdit *mSearchLine;
    ScrollArea *mBoxesListScrollArea;
    BoxScrollWidget *mBoxesListWidget;
    KeysView *mKeysView;
    MainWindow *mMainWindow;
    AnimationDockWidget *mAnimationDockWidget;

    BoxesListAnimationDockWidget *mBoxesListAnimationDockWidget;
};

#endif // BOXESLISTKEYSVIEWWIDGET_H
