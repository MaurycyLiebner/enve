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
#include <mypaint-brush.h>
#include "exceptions.h"
#include "Private/esettings.h"
#include "Paint/brushcontexedwrapper.h"
class FlowLayout;

struct BrushContexedCollection {
    BrushContexedCollection(const BrushCollectionData& raw) {
        fName = raw.fName;
        for(const auto& brushRaw : raw.fBrushes) {
            fBrushes << enve::make_shared<BrushContexedWrapper>(brushRaw);
        }
    }
    QString fName;
    QList<qsptr<BrushContexedWrapper>> fBrushes;
};

struct BrushesContext {
    BrushesContext(const QList<BrushCollectionData>& raw) {
        for(const auto& coll : raw) {
            fCollections << BrushContexedCollection(coll);
        }
    }

    bool setSelectedWrapper(SimpleBrushWrapper* const wrapper) {
        for(const auto& coll : fCollections) {
            for(auto& brush : coll.fBrushes) {
                if(brush->getSimpleBrush() == wrapper) {
                    brush->setSelected(true);
                    return true;
                }
            }
        }
        return false;
    }

    QList<BrushContexedCollection> fCollections;
};

class BrushSelectionWidget : public QTabWidget {
    Q_OBJECT
public:
    BrushSelectionWidget(const int contextId,
                         QWidget * const parent = nullptr);
    ~BrushSelectionWidget() {}

    void updateBrushes();

    int getContextId() const {
        return mContextId;
    }

    static int sCreateNewContext() {
        if(!sLoaded) {
            const QString brushesDir = eSettings::sSettingsDir() + "/brushes";
            sLoadCollectionsFromDir(brushesDir);
            sLoadCollectionsFromDir(":/brushes");
            sLoaded = true;
        }
        const int id = sBrushContexts.count();
        sBrushContexts << BrushesContext(BrushCollectionData::sData);
        return id;
    }

    static void sSetCurrentBrushForContext(
            const int contextId,
            SimpleBrushWrapper* const wrapper) {
        auto& context = sBrushContexts[contextId];
        context.setSelectedWrapper(wrapper);
    }

    SimpleBrushWrapper * getCurrentBrush() {
        if(mCurrentBrushCWrapper)
            return mCurrentBrushCWrapper->getSimpleBrush();
        return nullptr;
    }
signals:
    void currentBrushChanged(SimpleBrushWrapper*);
private:
    void brushCWrapperSelected(BrushContexedWrapper * wrapper) {
        if(mCurrentBrushCWrapper) mCurrentBrushCWrapper->setSelected(false);
        mCurrentBrushCWrapper = wrapper;
        emit currentBrushChanged(getCurrentBrush());
    }
    static void sLoadCollectionsFromDir(const QString& mainDirPath);

    static bool sLoaded;
    static QList<BrushesContext> sBrushContexts;

    static const BrushesContext& sGetContext(const int id) {
        if(id < 0 || id >= sBrushContexts.count())
            RuntimeThrow("Id does not correspond to any context");
        return sBrushContexts.at(id);
    }

    BrushContexedWrapper * mCurrentBrushCWrapper = nullptr;
    const int mContextId;
};

#endif // BRUSHSELECTIONWIDGET_H
