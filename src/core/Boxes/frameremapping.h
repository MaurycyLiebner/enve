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

#ifndef FRAMEREMAPPING_H
#define FRAMEREMAPPING_H
#include "Animators/qrealanimator.h"

class CORE_EXPORT FrameRemappingBase : public QrealAnimator {
    Q_OBJECT
protected:
    FrameRemappingBase();
public:    
    bool enabled() const { return mEnabled; }

    void enableAction(const int minFrame, const int maxFrame,
                      const int animStartRelFrame);
    void disableAction();

    void setFrameCount(const int count);

    void prp_readProperty(eReadStream &src) override;
    void prp_writeProperty(eWriteStream &dst) const override;

    QDomElement prp_writePropertyXEV(const XevExporter& exp) const override;
    void prp_readPropertyXEV(const QDomElement& ele,
                             const XevImporter& imp) override;
signals:
    void enabledChanged(const bool enabled);
private:
    void setEnabled(const bool enabled);

    bool mEnabled;
};

class CORE_EXPORT IntFrameRemapping : public FrameRemappingBase {
    e_OBJECT
protected:
    IntFrameRemapping();
public:
    int frame(const qreal relFrame) const;
};

class CORE_EXPORT QrealFrameRemapping : public FrameRemappingBase {
    e_OBJECT
protected:
    QrealFrameRemapping();
public:
    qreal frame(const qreal relFrame) const;
};

#endif // FRAMEREMAPPING_H
