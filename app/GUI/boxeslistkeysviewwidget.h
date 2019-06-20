#ifndef BOXESLISTKEYSVIEWWIDGET_H
#define BOXESLISTKEYSVIEWWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMenuBar>
#include "smartPointers/stdselfref.h"
#include "framerange.h"

class SWT_Abstraction;
class FrameScrollBar;
class KeysView;
class BoxesListAnimationDockWidget;
class ChangeWidthWidget;
class MainWindow;
class ScrollArea;
class AnimationDockWidget;
class BoxScrollWidget;
class Document;

enum SWT_Type : short;
enum SWT_BoxRule : short;

class BoxesListKeysViewWidget : public QWidget {
    Q_OBJECT
public:
    explicit BoxesListKeysViewWidget(Document& document,
                                     QWidget *topWidget,
                                     BoxesListAnimationDockWidget *animationDock,
                                     QWidget *parent);

    void setTopWidget(QWidget *topWidget);
    void connectToFrameWidget(FrameScrollBar *frameRange);
    void connectToChangeWidthWidget(
            ChangeWidthWidget *changeWidthWidget);
    void setDisplayedFrameRange(const FrameRange &range);
signals:
    void changedFrameRange(FrameRange);
private:
    void setRuleNone();
    void setRuleSelected();
    void setRuleAnimated();
    void setRuleNotAnimated();
    void setRuleVisible();
    void setRuleHidden();
    void setRuleUnloced();
    void setRuleLocked();

    void setTargetAll();
    void setTargetCurrentCanvas();
    void setTargetCurrentGroup();

    void setSearchText(const QString &text);
    void moveSlider(int val);

    void setTypeAll();
    void setTypeGraphics();
    void setTypeSound();

    void setGraphEnabled(const bool bT);
protected:
    void setBoxesListWidth(const int width);

    void addNewBelowThis();
    void removeThis();
private:
    void setCurrentType(const SWT_Type type);
    void setBoxRule(const SWT_BoxRule rule);

    Document& mDocument;

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
