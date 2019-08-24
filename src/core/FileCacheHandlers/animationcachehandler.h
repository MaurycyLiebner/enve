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

#ifndef ANIMATIONCACHEHANDLER_H
#define ANIMATIONCACHEHANDLER_H
#include "smartPointers/selfref.h"
#include "skia/skiahelpers.h"
class eTask;

class AnimationFrameHandler : public SelfRef {
protected:
    AnimationFrameHandler();
public:
    virtual sk_sp<SkImage> getFrameAtFrame(const int relFrame) = 0;
    virtual sk_sp<SkImage> getFrameAtOrBeforeFrame(const int relFrame) = 0;
    virtual eTask* scheduleFrameLoad(const int frame) = 0;
    virtual int getFrameCount() const = 0;
    virtual void reload() = 0;

    sk_sp<SkImage> getFrameCopyAtFrame(const int relFrame);
    sk_sp<SkImage> getFrameCopyAtOrBeforeFrame(const int relFrame);
protected:
};
#endif // ANIMATIONCACHEHANDLER_H
