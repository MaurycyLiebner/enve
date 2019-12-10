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

#ifndef WIDGETWRAPPERNODE_H
#define WIDGETWRAPPERNODE_H

#include <QMenuBar>
#include "Private/esettings.h"
#include "wrappernode.h"

class StackWrapperMenu : public QMenuBar {
    friend class StackWidgetWrapper;
protected:
    explicit StackWrapperMenu();
};

class WidgetWrapperCornerMenu;
class WidgetWrapperNode : public QWidget, public WrapperNode {
public:
    WidgetWrapperNode(const WrapperNodeCreator& creator);

    void close();
    void splitV();
    void splitH();

    void setMenuBar(StackWrapperMenu * const menu);
    void setCentralWidget(QWidget * const widget);

    WidgetWrapperCornerMenu* cornerMenu() const {
        return mCornerMenu;
    }

    QWidget* widget() { return this; }
private:
    WidgetWrapperCornerMenu* mCornerMenu;
    StackWrapperMenu* mMenuBar = nullptr;
    QWidget* mCenterWidget = nullptr;
    QVBoxLayout* mLayout;
};

class WidgetWrapperCornerMenu : public QMenuBar {
public:
    WidgetWrapperCornerMenu(WidgetWrapperNode* const target) {
        const auto iconsDir = eSettings::sIconsDir();
        mSplitV = addAction("split v");
        mSplitV->setIcon(QIcon(iconsDir + "/split_v.png"));
        mSplitH = addAction("split h");
        mSplitH->setIcon(QIcon(iconsDir + "/split_h.png"));
        mClose = addAction("x");
        mClose->setIcon(QIcon(iconsDir + "/close.png"));

        connect(mSplitV, &QAction::triggered,
                target, &WidgetWrapperNode::splitV);
        connect(mSplitH, &QAction::triggered,
                target, &WidgetWrapperNode::splitH);
        connect(mClose, &QAction::triggered,
                target, &WidgetWrapperNode::close);

        setObjectName("cornerMenuBar");

        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    }

    void disableClose() {
        mClose->setVisible(false);
    }

    void enableClose() {
        mClose->setVisible(true);
    }
private:
    QAction * mSplitV = nullptr;
    QAction * mSplitH = nullptr;
    QAction * mClose = nullptr;
};

#endif // WIDGETWRAPPERNODE_H
