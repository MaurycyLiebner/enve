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

#ifndef UNDOABLETILE_H
#define UNDOABLETILE_H
#include "tile.h"

struct CORE_EXPORT UndoableTile : public Tile {
    UndoableTile(const size_t& size) : Tile(size) {}

    bool fUndo = false;
};

class CORE_EXPORT UndoTile {
public:
    UndoTile(const int tx, const int ty,
             const stdsptr<UndoableTile>& tile);

    int tileX() const { return mX; }
    int tileY() const { return mY; }

    void saveForRedoAndReset();

    const stdsptr<Tile>& oldValue() const { return mOldValue; }
    const stdsptr<Tile>& newValue() const { return mNewValue; }
private:
    int mX;
    int mY;
    stdsptr<UndoableTile> mTile;
    stdsptr<Tile> mOldValue;
    stdsptr<Tile> mNewValue;
};

#endif // UNDOABLETILE_H
