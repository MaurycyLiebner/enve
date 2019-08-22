#ifndef CUSTOMIDENTIFIER_H
#define CUSTOMIDENTIFIER_H
#include "basicreadwrite.h"

struct CustomIdentifier {
    struct Version {
        uint fMajor;
        uint fMinor;
        uint fPatch;

        QString toString() const {
            return QString::number(fMajor) + "." +
                   QString::number(fMinor) + "." +
                   QString::number(fPatch);
        }

        bool operator==(const Version& other) const {
            return fMajor == other.fMajor &&
                   fMinor == other.fMinor &&
                   fPatch == other.fPatch;
        }

        bool operator>(const Version& other) const {
            if(fMajor < other.fMajor) return false;
            if(fMajor > other.fMajor) return true;
            if(fMinor < other.fMinor) return false;
            if(fMinor > other.fMinor) return true;
            return fPatch > other.fPatch;
        }

        bool operator<(const Version& other) const {
            return other > *this;
        }
    };

    QString fEffectId;
    QString fEffectName;
    Version fVersion;

    QString toString() const {
        return fEffectId + " " + fEffectName + " " + fVersion.toString();
    }

    void write(eWriteStream& dst) const {
        dst << fEffectId;
        dst << fEffectName;
        dst.write(&fVersion, sizeof(Version));
    }

    static CustomIdentifier sRead(eReadStream& src) {
        CustomIdentifier id;
        src >> id.fEffectId;
        src >> id.fEffectName;
        src.read(&id.fVersion, sizeof(Version));
        return id;
    }
};

#endif // CUSTOMIDENTIFIER_H
