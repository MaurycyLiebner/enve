#ifndef BOXESLISTKEYSVIEWWIDGET_H
#define BOXESLISTKEYSVIEWWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
class AnimationWidgetScrollBar;
#include "keysview.h"
#include "BoxesList/boxscrollwidget.h"
class BoxesListAnimationDockWidget;
class ChangeWidthWidget;
#include <QMenuBar>
#include "actionbutton.h"

class BoxesListKeysViewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BoxesListKeysViewWidget(QWidget *topWidget,
                                     BoxesListAnimationDockWidget *animationDock,
                                     QWidget *parent);
    ~BoxesListKeysViewWidget();

    void setTopWidget(QWidget *topWidget);
    bool processFilteredKeyEvent(QKeyEvent *event);
    void connectToFrameWidget(AnimationWidgetScrollBar *frameRange);
    void connectToChangeWidthWidget(
            ChangeWidthWidget *changeWidthWidget);
signals:

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
    QWidget *mTopWidget = NULL;
    QMenuBar *mBoxesListMenuBar;
    QLineEdit *mSearchLine;
    ScrollArea *mBoxesListScrollArea;
    BoxScrollWidget *mBoxesListWidget;
    KeysView *mKeysView;
    MainWindow *mMainWindow;
    ActionButton *mGraphEnabledButton;
    AnimationDockWidget *mAnimationDockWidget;

    BoxesListAnimationDockWidget *mBoxesListAnimationDockWidget;
};

#endif // BOXESLISTKEYSVIEWWIDGET_H
