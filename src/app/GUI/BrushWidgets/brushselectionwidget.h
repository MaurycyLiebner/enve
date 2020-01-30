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

#ifndef BRUSHSELECTIONWIDGET_H
#define BRUSHSELECTIONWIDGET_H

class BrushCollection;
#include <QTabWidget>
#include "Paint/brushescontext.h"
#include "exceptions.h"
#include "Private/esettings.h"
#include "Paint/brushcontexedwrapper.h"
class FlowLayout;
class ScrollArea;

class BrushSelectionWidget : public QTabWidget {
    Q_OBJECT
public:
    BrushSelectionWidget(BrushesContext &context,
                         QWidget * const parent = nullptr);

    static qsptr<BrushesContext> sCreateNewContext();

    void setCurrentBrush(SimpleBrushWrapper* const wrapper);

    SimpleBrushWrapper * getCurrentBrush();

    static qsptr<BrushesContext> sPaintContext;
    static qsptr<BrushesContext> sOutlineContext;
signals:
    void currentBrushChanged(BrushContexedWrapper*);
    void brushTriggered(BrushContexedWrapper*);
private:
    void setupBookmarksTab();
    void setNumberBookmarked(const int bookmarked);
    void updateBrushes();
    void brushCWrapperSelected(BrushContexedWrapper * wrapper);
    static void sLoadCollectionsFromDir(const QString& mainDirPath);

    static bool sLoaded;

    int mNumberBookmarked = 0;
    ScrollArea* mBookmarksScroll;
    BrushContexedWrapper * mSelected = nullptr;
    BrushesContext& mContext;
};

#endif // BRUSHSELECTIONWIDGET_H
