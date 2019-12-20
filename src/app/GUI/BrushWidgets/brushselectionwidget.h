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

#ifndef BRUSHSELECTIONWIDGET_H
#define BRUSHSELECTIONWIDGET_H

class BrushCollection;
#include <QTabWidget>
#include "Paint/brushescontext.h"
#include "exceptions.h"
#include "Private/esettings.h"
#include "Paint/brushcontexedwrapper.h"
class FlowLayout;

class BrushSelectionWidget : public QTabWidget {
    Q_OBJECT
public:
    BrushSelectionWidget(BrushesContext &context,
                         QWidget * const parent = nullptr);
    void updateBrushes();

    static qsptr<BrushesContext> sCreateNewContext() {
        if(!sLoaded) {
            const QString brushesDir = eSettings::sSettingsDir() + "/brushes";
            sLoadCollectionsFromDir(brushesDir);
            sLoadCollectionsFromDir(":/brushes");
            sLoaded = true;
        }
        return enve::make_shared<BrushesContext>(BrushCollectionData::sData);
    }

    void setCurrentBrush(SimpleBrushWrapper* const wrapper) {
        mContext.setSelectedWrapper(wrapper);
    }

    SimpleBrushWrapper * getCurrentBrush() {
        if(mCurrentBrushCWrapper)
            return mCurrentBrushCWrapper->getSimpleBrush();
        return nullptr;
    }

    static qsptr<BrushesContext> sPaintContext;
    static qsptr<BrushesContext> sOutlineContext;
signals:
    void currentBrushChanged(BrushContexedWrapper*);
private:
    void brushCWrapperSelected(BrushContexedWrapper * wrapper) {
        if(mCurrentBrushCWrapper) mCurrentBrushCWrapper->setSelected(false);
        mCurrentBrushCWrapper = wrapper;
        emit currentBrushChanged(wrapper);
    }
    static void sLoadCollectionsFromDir(const QString& mainDirPath);

    static bool sLoaded;

    BrushContexedWrapper * mCurrentBrushCWrapper = nullptr;
    BrushesContext& mContext;
};

#endif // BRUSHSELECTIONWIDGET_H
