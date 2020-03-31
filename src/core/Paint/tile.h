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

#ifndef TILE_H
#define TILE_H
#include "exceptions.h"
#include "ReadWrite/ereadstream.h"
#include "ReadWrite/ewritestream.h"
#include "smartPointers/stdselfref.h"

class CORE_EXPORT Tile {
public:
    Tile(const size_t& size);
    Tile(const Tile& other);

    ~Tile();

    void swap(Tile& other);

    void allocateData();
    void zeroData();
    void removeData();

    bool dataTransparent();

    uint16_t* requestData();
    uint16_t* requestZeroedData();
    uint16_t* data() const;

    void write(eWriteStream& dst) const;

    using TileCreator = std::function<stdsptr<Tile>(const size_t&)>;
    static stdsptr<Tile> sRead(eReadStream& src, const TileCreator& tileCreator);

    void copyFrom(const Tile& other);

    const size_t fSize;
private:
    uint16_t* mData = nullptr;
};

#endif // TILE_H
