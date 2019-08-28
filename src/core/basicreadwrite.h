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
#include "framerange.h"
struct qCubicSegment1D;
class SmartPath;
class BrushPolyline;
struct AutoTiledSurface;

class FileFooter {
public:
    static bool sWrite(QIODevice * const target) {
        return target->write(reinterpret_cast<const char*>(sEVFormat), sizeof(char[15])) &&
               target->write(reinterpret_cast<const char*>(sAppName), sizeof(char[15])) &&
               target->write(reinterpret_cast<const char*>(sAppVersion), sizeof(char[15]));
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
        dst->read(format, sizeof(char[15]));
        if(std::strcmp(format, sEVFormat)) return false;

//        char appVersion[15];
//        target->read(appVersion, sizeof(char[15]));

//        char appName[15];
//        target->read(appName, sizeof(char[15]));

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
    eReadFutureTable(QIODevice* const main);

    bool seek(const eFuturePos& pos) {
        mFutureId = int(pos.fFutureTable) + 1;
        return mMain->seek(pos.fMain);
    }

    eFuturePos readFuturePos() {
        return mFutures.at(mFutureId++);
    }

    void read();
private:
    int mFutureId = 0;
    QList<eFuturePos> mFutures;
    QIODevice* const mMain;
};

class eWriteStream;
class eWriteFutureTable {
    friend class eWriteStream;
    eWriteFutureTable(QIODevice * const main) :
        mMain(main) {}

    void write(eWriteStream& dst);

    //! @brief Returns id to be used with assignFuturePos
    int planFuturePos() {
        const int id = mFutures.count();
        mFutures.append({-1, -1});
        return id;
    }

    void assignFuturePos(const int id) {
        mFutures.replace(id, {mMain->pos(), id});
    }
private:
    QList<eFuturePos> mFutures;
    QIODevice* const mMain;
};

#include <QPointF>
class eReadStream {
public:
    eReadStream(QIODevice* const src) : mSrc(src), mFutureTable(src) {}

    void readFutureTable() {
        mFutureTable.read();
    }

    eFuturePos readFuturePos() { return mFutureTable.readFuturePos(); }

    bool seek(const eFuturePos& pos) {
        return mFutureTable.seek(pos);
    }

    void readCheckpoint(const QString& errMsg) {
        const qint64 sPos = mSrc->pos();
        qint64 pos; read(&pos, sizeof(qint64));
        if(pos != sPos)
            RuntimeThrow("The QIODevice::pos '" + QString::number(sPos) + "' does not match"
                         " the written QIODevice::pos '" + QString::number(pos) + "'.\n" + errMsg);
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


    eReadStream& operator>>(uint& val) {
        read(&val, sizeof(uint));
        return *this;
    }

    eReadStream& operator>>(uint64_t& val) {
        read(&val, sizeof(uint64_t));
        return *this;
    }

    eReadStream& operator>>(iValueRange& val) {
        read(&val, sizeof(iValueRange));
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
    friend class MainWindow;
public:
    class FuturePosId {
        friend class eWriteStream;
        FuturePosId(const int id) : fId(id) {}
        const int fId;
    };

    eWriteStream(QIODevice* const dst) : mDst(dst), mFutureTable(dst) {}

    void writeFutureTable() {
        mFutureTable.write(*this);
    }

    //! @brief Returns id to be used with assignFuturePos
    FuturePosId planFuturePos() {
        return mFutureTable.planFuturePos();
    }

    void assignFuturePos(const FuturePosId id) {
        mFutureTable.assignFuturePos(id.fId);
    }

    void writeCheckpoint() {
        const qint64 pos = mDst->pos();
        write(&pos, sizeof(qint64));
    }

    qint64 writeFile(QFile* const file) {
        if(!file) RuntimeThrow("No file to write");
        const bool openRes = file->open(QIODevice::ReadOnly);
        if(!openRes) RuntimeThrow("Could not open file");
        const qint64 size = file->size();
        const qint64 lineSize = 1024;
        char line[lineSize];
        const int nLines = static_cast<int>(size/lineSize);
        const qint64 rem = size - nLines*lineSize;
        for(int i = 0; i < nLines; i++) {
            file->read(&line[0], lineSize);
            write(&line[0], lineSize);
        }
        if(rem > 0) {
            file->read(&line[0], rem);
            write(&line[0], rem);
        }
        file->close();
        return size;
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

    eWriteStream& operator<<(const uint val) {
        write(&val, sizeof(uint));
        return *this;
    }

    eWriteStream& operator<<(const uint64_t val) {
        write(&val, sizeof(uint64_t));
        return *this;
    }

    eWriteStream& operator<<(const iValueRange val) {
        write(&val, sizeof(iValueRange));
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
