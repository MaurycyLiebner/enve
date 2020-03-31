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

#ifndef TILEBITMAPS_H
#define TILEBITMAPS_H
#include "skia/skiahelpers.h"

struct CORE_EXPORT TileBitmaps {
    int fRowCount = 0;
    int fColumnCount = 0;
    int fZeroTileRow = 0;
    int fZeroTileCol = 0;
    QList<QList<SkBitmap>> fBitmaps;

    TileBitmaps();
    TileBitmaps(const TileBitmaps& src);
    TileBitmaps(TileBitmaps &&other);

    TileBitmaps &operator=(const TileBitmaps& other);
    TileBitmaps &operator=(TileBitmaps &&other);

    void write(eWriteStream& dst) const;
    void read(eReadStream& src);

    void swap(TileBitmaps& other);

    bool isEmpty() const { return fBitmaps.isEmpty(); }

    void clear();
};

#endif // TILEBITMAPS_H
