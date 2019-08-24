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

#ifndef BASICREADWRITE_H
#define BASICREADWRITE_H
#include <QString>
#include <QIODevice>
#include <QBuffer>
#include <cstring>
#include "exceptions.h"
#include "castmacros.h"
struct qCubicSegment1D;
class SmartPath;
class BrushPolyline;
struct AutoTiledSurface;

class FileFooter {
public:
    static bool sWrite(QIODevice * const target) {
        return target->write(rcConstChar(sEVFormat), sizeof(char[15])) &&
               target->write(rcConstChar(sAppName), sizeof(char[15])) &&
               target->write(rcConstChar(sAppVersion), sizeof(char[15]));
    }

    static qint64 sSize() {
        return static_cast<qint64>(3*sizeof(char[15]));
    }

    static bool sCompatible(QIODevice * const dst) {
        const qint64 savedPos = dst->pos();
        const qint64 pos = dst->size() - sSize();
        if(!dst->seek(pos))
            RuntimeThrow("Failed to seek to FileFooter");

        char format[15];
        dst->read(rcChar(format), sizeof(char[15]));
        if(std::strcmp(format, sEVFormat)) return false;

//        char appVersion[15];
//        target->read(rcChar(appVersion), sizeof(char[15]));

//        char appName[15];
//        target->read(rcChar(appName), sizeof(char[15]));

        if(!dst->seek(savedPos))
            RuntimeThrow("Could not restore current position for QIODevice.");
        return true;
    }
private:
    static char sEVFormat[15];
    static char sAppName[15];
    static char sAppVersion[15];
};

struct eFuturePos {
    qint64 fMain;
    qint64 fFutureTable;
};

class eReadFutureTable {
    friend class eReadStream;
    eReadFutureTable(QIODevice* const main) : mMain(main) {
        const qint64 savedPos = main->pos();
        const qint64 pos = main->size() - FileFooter::sSize() -
                           qint64(sizeof(qint64));
        main->seek(pos);
        qint64 tableSize;
        main->read(rcChar(&tableSize), sizeof(qint64));
        main->seek(pos - tableSize);
        mData = main->read(tableSize);
        mBuffer.setBuffer(&mData);
        main->seek(savedPos);
    }

    bool seek(const eFuturePos& pos) {
        return mMain->seek(pos.fMain) && mBuffer.seek(pos.fFutureTable);
    }

    eFuturePos readFuturePos() {
        eFuturePos pos;
        mBuffer.read(rcChar(&pos), sizeof(eFuturePos));
        return pos;
    }
private:
    QIODevice* const mMain;
    QBuffer mBuffer;
    QByteArray mData;
};

class eWriteFutureTable {
    friend class eWriteStream;
    eWriteFutureTable(QIODevice * const main) :
        mMain(main), mBuffer(&mData) {}
public:
    const QByteArray& data() const { return mData; }

    //! @brief Returns id to be used with assignFuturePos
    int planFuturePos() {
        const eFuturePos pos{0, 0};
        const int id = mData.size();
        mBuffer.write(rcConstChar(&pos), sizeof(eFuturePos));
        return id;
    }

    void assignFuturePos(const int id) {
        eFuturePos pos{mMain->pos(), mBuffer.pos()};
        mData.replace(id, sizeof(eFuturePos), rcConstChar(&pos));
    }
private:
    QIODevice* const mMain;
    QBuffer mBuffer;
    QByteArray mData;
};

#include <QPointF>
class eReadStream {
public:
    eReadStream(QIODevice* const src) : mSrc(src), mFutureTable(src) {}

    eFuturePos readFuturePos() { return mFutureTable.readFuturePos(); }

    bool seek(const eFuturePos& pos) {
        return mFutureTable.seek(pos);
    }

    void readCheckpoint(const QString& errMsg) {
        const qint64 sPos = mSrc->pos();
        qint64 pos; read(&pos, sizeof(qint64));
        if(pos != sPos)
            RuntimeThrow("The read QIODevice::pos does not match"
                         " the written QIODevice::pos.\n" + errMsg);
    }

    inline qint64 read(void* const data, const qint64 len) {
        return mSrc->read(reinterpret_cast<char*>(data), len);
    }

    eReadStream& operator>>(bool &val) {
        read(&val, sizeof(bool));
        return *this;
    }

    eReadStream& operator>>(int &val) {
        read(&val, sizeof(int));
        return *this;
    }

    eReadStream& operator>>(qreal &val) {
        read(&val, sizeof(qreal));
        return *this;
    }

    eReadStream& operator>>(QPointF &val) {
        read(&val, sizeof(QPointF));
        return *this;
    }

    eReadStream& operator>>(QString &val) {
        uint nChars; read(&nChars, sizeof(uint));
        if(nChars == 0) val = "";
        else {
            ushort * const chars = new ushort[nChars];
            read(chars, nChars*sizeof(ushort));
            val = QString::fromUtf16(chars, static_cast<int>(nChars));
            delete[] chars;
        }
        return *this;
    }
private:
    QIODevice* const mSrc;
    eReadFutureTable mFutureTable;
};

class eWriteStream {
public:
    eWriteStream(QIODevice* const dst) : mDst(dst), mFutureTable(dst) {}

    const QByteArray& futureData() const { return mFutureTable.data(); }

    //! @brief Returns id to be used with assignFuturePos
    int planFuturePos() {
        return mFutureTable.planFuturePos();
    }

    void assignFuturePos(const int id) {
        mFutureTable.assignFuturePos(id);
    }

    void writeCheckpoint() {
        const qint64 pos = mDst->pos();
        write(&pos, sizeof(qint64));
    }

    qint64 write(const void* const data, const qint64 len) {
        return mDst->write(reinterpret_cast<const char*>(data), len);
    }

    eWriteStream& operator<<(const bool val) {
        write(&val, sizeof(bool));
        return *this;
    }

    eWriteStream& operator<<(const int val) {
        write(&val, sizeof(int));
        return *this;
    }

    eWriteStream& operator<<(const qreal val) {
        write(&val, sizeof(qreal));
        return *this;
    }

    eWriteStream& operator<<(const QPointF& val) {
        write(&val, sizeof(QPointF));
        return *this;
    }

    eWriteStream& operator<<(const QString& val) {
        const uint nChars = static_cast<uint>(val.length());
        write(&nChars, sizeof(uint));
        if(nChars != 0) write(val.utf16(), nChars*sizeof(ushort));
        return *this;
    }
private:
    QIODevice* const mDst;
    eWriteFutureTable mFutureTable;
};

#endif // BASICREADWRITE_H
