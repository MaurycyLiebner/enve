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

#include "tile.h"
#include "ReadWrite/evformat.h"

Tile::Tile(const size_t &size) : fSize(size) {}

Tile::Tile(const Tile &other) : Tile(other.fSize) {
    copyFrom(other);
}

Tile::~Tile() { if(mData) delete[] mData; }

void Tile::swap(Tile &other) {
    std::swap(mData, other.mData);
}

void Tile::allocateData() {
    removeData();
    mData = new uint16_t[fSize];
    if(!mData) RuntimeThrow("Could not allocate memory for a tile.");
}

void Tile::zeroData() {
    memset(requestData(), 0, fSize*sizeof(uint16_t));
}

void Tile::removeData() {
    if(mData) {
        delete[] mData;
        mData = nullptr;
    }
}

bool Tile::dataTransparent() {
    if(!mData) return false;
    for(size_t a = 3; a < fSize; a += 4) {
        if(mData[a] != 0) return false;
    }
    return true;
}

uint16_t *Tile::requestData() {
    if(!mData) allocateData();
    return mData;
}

uint16_t *Tile::requestZeroedData() {
    if(!mData) {
        allocateData();
        zeroData();
    }
    return mData;
}

uint16_t *Tile::data() const { return mData; }

void Tile::write(eWriteStream &dst) const {
    dst << static_cast<uint64_t>(fSize);
    const bool data = mData; dst << data;
    if(data) dst.writeCompressed(mData, fSize*sizeof(uint16_t));
}

stdsptr<Tile> Tile::sRead(eReadStream &src, const TileCreator &tileCreator) {
    uint64_t size; src >> size;
    bool data; src >> data;
    const auto result = tileCreator(size);
    if(data) {
        const auto data = result->requestData();
        if(src.evFileVersion() >= EvFormat::dataCompression) {
            const auto readData = src.readCompressed();
            Q_ASSERT(size*sizeof(uint16_t) == size_t(readData.size()));
            memcpy(data, readData.data(), readData.size());
        } else src.read(data, size*sizeof(uint16_t));
    }
    return result;
}

void Tile::copyFrom(const Tile &other) {
    const auto otherData = other.data();
    if(otherData) {
        const auto thisData = requestData();
        memcpy(thisData, otherData, fSize*sizeof(uint16_t));
    } else {
        removeData();
    }
}
