#ifndef CUSTOMIDENTIFIER_H
#define CUSTOMIDENTIFIER_H
#include "basicreadwrite.h"

struct CustomIdentifier {
    struct Version {
        uint fMajor;
        uint fMinor;
        uint fPatch;

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

    void write(QIODevice * const dst) const {
        gWrite(dst, fEffectId);
        gWrite(dst, fEffectName);
        dst->write(rcConstChar(&fVersion), sizeof(Version));
    }

    static CustomIdentifier sRead(QIODevice * const src) {
        CustomIdentifier id;
        id.fEffectId = gReadString(src);
        id.fEffectName = gReadString(src);
        src->read(rcChar(&id.fVersion), sizeof(Version));
        return id;
    }
};

#endif // CUSTOMIDENTIFIER_H
