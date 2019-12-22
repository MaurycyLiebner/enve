// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef BOXESLISTKEYSVIEWWIDGET_H
#define BOXESLISTKEYSVIEWWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMenuBar>
#include "smartPointers/stdselfref.h"
#include "framerange.h"
#include "ReadWrite/basicreadwrite.h"

class SWT_Abstraction;
class FrameScrollBar;
class KeysView;
class ChangeWidthWidget;
class MainWindow;
class ScrollArea;
class AnimationDockWidget;
class BoxScrollWidget;
class Document;
class Canvas;
class SceneChooser;
class StackWrapperCornerMenu;
class BoxScroller;

enum SWT_Type : short;
enum SWT_BoxRule : short;

class TimelineWidget : public QWidget {
public:
    explicit TimelineWidget(Document& document,
                            QMenuBar * const menu,
                            QWidget *parent);

    Canvas* getCurrrentScene() const {
        return mCurrentScene;
    }

    void setCurrentScene(Canvas* const scene);
    void setBoxesListWidth(const int width);
    void setGraphEnabled(const bool enabled);

    void writeState(eWriteStream& dst) const;
    void readState(eReadStream& src);
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
    BoxScroller *mBoxesListVisible;
    KeysView *mKeysView;
    AnimationDockWidget *mAnimationDockWidget;
};

#endif // BOXESLISTKEYSVIEWWIDGET_H
