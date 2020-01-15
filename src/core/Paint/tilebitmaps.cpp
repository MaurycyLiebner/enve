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

#include "tilebitmaps.h"

TileBitmaps::TileBitmaps() {}

TileBitmaps::TileBitmaps(const TileBitmaps &src) {
    fRowCount = src.fRowCount;
    fColumnCount = src.fColumnCount;
    fZeroTileRow = src.fZeroTileRow;
    fZeroTileCol = src.fZeroTileCol;
    fBitmaps.clear();
    for(const auto& srcList : src.fBitmaps) {
        fBitmaps << QList<SkBitmap>();
        auto& list = fBitmaps.last();
        for(const auto& srcBitmap : srcList) {
            list << SkiaHelpers::makeCopy(srcBitmap);
        }
    }
}

TileBitmaps::TileBitmaps(TileBitmaps &&other) {
    swap(other);
}

TileBitmaps &TileBitmaps::operator=(TileBitmaps &&other) {
    TileBitmaps tmp(std::move(other));
    swap(tmp);
    return *this;
}

void TileBitmaps::write(eWriteStream &dst) const {
    dst << fRowCount;
    dst << fColumnCount;
    dst << fZeroTileRow;
    dst << fZeroTileCol;
    for(const auto& col : fBitmaps) {
        for(const auto& tile : col) {
            SkiaHelpers::writeBitmap(tile, dst);
        }
    }
}

void TileBitmaps::read(eReadStream &src) {
    src >> fRowCount;
    src >> fColumnCount;
    src >> fZeroTileRow;
    src >> fZeroTileCol;
    for(int i = 0; i < fColumnCount; i++) {
        fBitmaps.append(QList<SkBitmap>());
        auto& col = fBitmaps.last();
        for(int j = 0; j < fZeroTileRow; j++)
            col.append(SkiaHelpers::readBitmap(src));
    }
}

void TileBitmaps::swap(TileBitmaps &other) {
    fBitmaps.swap(other.fBitmaps);
    std::swap(fRowCount, other.fRowCount);
    std::swap(fColumnCount, other.fColumnCount);
    std::swap(fZeroTileRow, other.fZeroTileRow);
    std::swap(fZeroTileCol, other.fZeroTileCol);
}

void TileBitmaps::clear() {
    fBitmaps.clear();
    fZeroTileCol = 0;
    fZeroTileRow = 0;
    fColumnCount = 0;
    fRowCount = 0;
}

TileBitmaps &TileBitmaps::operator=(const TileBitmaps &other) {
    TileBitmaps tmp(other);
    swap(tmp);
    return *this;
}
