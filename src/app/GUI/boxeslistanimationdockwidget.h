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

#ifndef BOXESLISTANIMATIONDOCKWIDGET_H
#define BOXESLISTANIMATIONDOCKWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <qscrollarea.h>
#include <QScrollArea>
#include <QApplication>
#include <QScrollBar>
#include <QComboBox>
#include <QMenuBar>
#include <QLineEdit>
#include <QWidgetAction>
#include <QToolBar>
#include "smartPointers/ememory.h"
#include "framerange.h"
#include "scenelayout.h"
class FrameScrollBar;
class TimelineWidget;
class RenderWidget;
class MainWindow;
class AnimationDockWidget;
class RenderWidget;
class ActionButton;
class Canvas;
class Document;
class LayoutHandler;

class BoxesListAnimationDockWidget : public QWidget {
public:
    explicit BoxesListAnimationDockWidget(Document &document,
                                          LayoutHandler* const layoutH,
                                          MainWindow *parent);
    bool processKeyPress(QKeyEvent *event);
    void previewFinished();
    void previewBeingPlayed();
    void previewBeingRendered();
    void previewPaused();

    void updateSettingsForCurrentCanvas(Canvas * const canvas);
    void clearAll();

    RenderWidget *getRenderWidget();
    SceneLayout* getTimelineLayout() {
        return mTimelineLayout;
    }
private:
    void setResolutionFractionText(QString text);

    void interruptPreview();

    void setLocalPivot(const bool bT);

    void setTimelineMode();
    void setRenderMode();
    void playPreview();
    void renderPreview();
    void pausePreview();
    void resumePreview();
private:
    Document& mDocument;

    SceneLayout* mTimelineLayout;

    QWidget* mCentralWidget = nullptr;

    QToolBar *mToolBar;

    QVBoxLayout *mMainLayout;

    QLabel *mControlButtonsWidget;

    QComboBox *mResolutionComboBox;

    ActionButton *mPlayButton;
    ActionButton *mStopButton;
    ActionButton *mLocalPivot;

    QAction *mTimelineAction;
    QAction *mRenderAction;

    QList<TimelineWidget*> mBoxesListKeysViewWidgets;
    RenderWidget *mRenderWidget;
    AnimationDockWidget *mAnimationDockWidget;

    MainWindow *mMainWindow;
};

#endif // BOXESLISTANIMATIONDOCKWIDGET_H
