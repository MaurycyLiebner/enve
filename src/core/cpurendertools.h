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

#ifndef CPURENDERTOOLS_H
#define CPURENDERTOOLS_H

#include "skia/skiahelpers.h"
class CpuRenderTools {
public:
    CpuRenderTools(const SkBitmap& srcBtmp);
    CpuRenderTools(const SkBitmap& srcBtmp,
                   const SkBitmap& backupBtmp);

    const SkBitmap fSrcDst;

    bool hasBackupBitmap() const { return !fBackupBtmp.isNull(); }

    SkBitmap requestBackupBitmap();

    void swap();
private:
    SkBitmap fBackupBtmp;
};

#endif // CPURENDERTOOLS_H
