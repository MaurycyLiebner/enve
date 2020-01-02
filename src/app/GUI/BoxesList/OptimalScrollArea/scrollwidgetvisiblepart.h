// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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

#ifndef SCROLLWIDGETVISIBLEPART_H
#define SCROLLWIDGETVISIBLEPART_H

#include "minimalscrollwidgetvisiblepart.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QtMath>
#include <memory>
#include "smartPointers/ememory.h"
#include "singlewidgettarget.h"
class ScrollWidget;
class SingleWidgetTarget;

class ScrollWidgetVisiblePart : public MinimalScrollWidgetVisiblePart {
    Q_OBJECT
public:
    ScrollWidgetVisiblePart(ScrollWidget * const parent);

    virtual QWidget *createNewSingleWidget();
    void updateVisibleWidgetsContent();

    void setMainAbstraction(SWT_Abstraction *abs);
    void setCurrentRule(const SWT_BoxRule rule);
    void setCurrentTarget(SingleWidgetTarget *targetP,
                          const SWT_Target target);
    void setAlwaysShowChildren(const bool alwaysShowChildren);
    void setCurrentSearchText(const QString &text);

    void scheduleContentUpdateIfIsCurrentRule(const SWT_BoxRule rule);
    bool isCurrentRule(const SWT_BoxRule rule);

    SWT_RulesCollection getCurrentRulesCollection() {
        return mCurrentRulesCollection;
    }

    bool getAlwaysShowChildren() {
        return mAlwaysShowChildren;
    }

    void scheduleContentUpdateIfSearchNotEmpty();
    void scheduleContentUpdateIfIsCurrentTarget(SingleWidgetTarget *targetP,
                                                const SWT_Target target);
    void setCurrentType(const SWT_Type type);
    int getId() const { return mId; }

    const UpdateFuncs& getUpdateFuncs() const {
        return mUpdateFuncs;
    }
protected:
    void setupUpdateFuncs() {
        mUpdateFuncs.fContentUpdateIfIsCurrentRule =
                [this](const SWT_BoxRule rule) {
            scheduleContentUpdateIfIsCurrentRule(rule);
        };
        mUpdateFuncs.fContentUpdateIfIsCurrentTarget =
                [this](SingleWidgetTarget* targetP,
                       const SWT_Target target) {
            scheduleContentUpdateIfIsCurrentTarget(targetP, target);
        };
        mUpdateFuncs.fContentUpdateIfSearchNotEmpty = [this]() {
            scheduleContentUpdateIfSearchNotEmpty();
        };
        mUpdateFuncs.fUpdateParentHeight = [this]() {
            planScheduleUpdateParentHeight();
        };
        mUpdateFuncs.fUpdateVisibleWidgetsContent = [this]() {
            planScheduleUpdateVisibleWidgetsContent();
        };
    }

    UpdateFuncs mUpdateFuncs;
    static int sNextId;
    const int mId;
    int mNVisible = 0;
    bool mAlwaysShowChildren = false;
    SWT_RulesCollection mCurrentRulesCollection;
    stdptr<SWT_Abstraction> mMainAbstraction;
};


#endif // SCROLLWIDGETVISIBLEPART_H
