#ifndef BOXESLISTKEYSVIEWWIDGET_H
#define BOXESLISTKEYSVIEWWIDGET_H

#include <QWidget>
#include <QGridLayout>
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
class Canvas;
class SceneChooser;
class StackWrapperCornerMenu;

enum SWT_Type : short;
enum SWT_BoxRule : short;

class TimelineWidget : public QWidget {
public:
    explicit TimelineWidget(Document& document,
                            StackWrapperCornerMenu* const menu,
                            QWidget *parent);

    Canvas* getCurrrentScene() const {
        return mCurrentScene;
    }

    void setCurrentScene(Canvas* const scene);
    void setBoxesListWidth(const int width);
    void setGraphEnabled(const bool enabled);
private:
    void setViewedFrameRange(const FrameRange &range);
    void setCanvasFrameRange(const FrameRange &range);

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
private:
    void setCurrentType(const SWT_Type type);
    void setBoxRule(const SWT_BoxRule rule);

    Canvas* mCurrentScene = nullptr;

    Document& mDocument;

    SceneChooser* mSceneChooser;

    FrameScrollBar* mFrameScrollBar;
    FrameScrollBar* mFrameRangeScrollBar;

    QGridLayout *mMainLayout;
    QVBoxLayout *mBoxesListLayout;
    QVBoxLayout *mKeysViewLayout;
    QHBoxLayout *mMenuLayout;
    QHBoxLayout *mMenuWidgetsLayout;
    QAction *mGraphAct = nullptr;
    QWidget* mMenuWidget;
    QWidget *mMenuWidgetsCont;
    QMenuBar *mBoxesListMenuBar;
    QMenuBar *mCornerMenuBar;
    QLineEdit *mSearchLine;
    ScrollArea *mBoxesListScrollArea;
    BoxScrollWidget *mBoxesListWidget;
    KeysView *mKeysView;
    MainWindow *mMainWindow;
    AnimationDockWidget *mAnimationDockWidget;
};

#endif // BOXESLISTKEYSVIEWWIDGET_H
