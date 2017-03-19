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

    BoxesListAnimationDockWidget *mAnimationDockWidget;
};

#endif // BOXESLISTKEYSVIEWWIDGET_H
