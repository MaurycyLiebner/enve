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

#include "basicreadwrite.h"

char FileFooter::sEVFormat[15] = "enve ev";
char FileFooter::sAppName[15] = "enve";
char FileFooter::sAppVersion[15] = "0.0.0";

eReadFutureTable::eReadFutureTable(QIODevice * const main) : mMain(main) {
    const qint64 savedPos = main->pos();
    const qint64 pos = main->size() - FileFooter::sSize() -
            qint64(sizeof(int));
    main->seek(pos);
    int nFutures;
    main->read(reinterpret_cast<char*>(&nFutures), sizeof(int));

    main->seek(pos - nFutures*qint64(sizeof(eFuturePos)));
    for(int i = 0; i < nFutures; i++) {
        eFuturePos pos;
        main->read(reinterpret_cast<char*>(&pos), sizeof(eFuturePos));
        mFutures << pos;
    }
    main->seek(savedPos);
}

void eWriteFutureTable::write(eWriteStream &dst) {
    for(const auto& future : mFutures) {
        dst.write(&future, sizeof(eFuturePos));
    }
    dst << mFutures.count();
}
