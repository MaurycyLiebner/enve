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

#ifndef HddCACHABLECACHEHANDLER_H
#define HddCACHABLECACHEHANDLER_H
#include "hddcachablerangecont.h"
#include "rangemap.h"

class HddCachableCacheHandler {
public:
    typedef HddCachableRangeCont Cont;

    void blockConts(const FrameRange &range, const bool blocked);

    void drawCacheOnTimeline(QPainter * const p,
                             const QRectF &drawRect,
                             const int startFrame,
                             const int endFrame,
                             const qreal unit = 1) const;

    int firstEmptyFrameAtOrAfter(const int frame) const {
        return mConts.firstEmptyRangeLowerBound(frame).fMin;
    }

    void add(const stdsptr<Cont>& cont) {
        /*const auto ret = */mConts.insert({cont->getRange(), cont});
        //if(!ret.second) RuntimeThrow("Range already occupied by a different element");
    }

    void clear() {
        mConts.clear();
    }

    void remove(const stdsptr<Cont>& cont) {
        mConts.erase(cont->getRange());
    }

    void remove(const iValueRange& range) {
        mConts.erase(range);
    }

    template <class T = Cont>
    T * atFrame(const int relFrame) const {
        const auto it = mConts.atFrame(relFrame);
        if(it == mConts.end()) return nullptr;
        return static_cast<T*>(it->second.get());
    }

    template <class T = Cont>
    T * atOrBeforeFrame(const int relFrame) const {
        const auto it = mConts.atOrBeforeFrame(relFrame);
        if(it == mConts.end()) return nullptr;
        return static_cast<T*>(it->second.get());
    }
private:
    RangeMap<stdsptr<Cont>> mConts;
};

#endif // HddCACHABLECACHEHANDLER_H
