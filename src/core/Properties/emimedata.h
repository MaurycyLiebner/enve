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

#ifndef EMIMEDATA_H
#define EMIMEDATA_H
#include <QMimeData>
#include "exceptions.h"
#include "smartPointers/ememory.h"

class eDraggedObjects {
public:
    template <typename T>
    eDraggedObjects(const QList<T*>& objs) :
        mMetaObj(T::staticMetaObject) {
        for(const auto& obj : objs) mObjects << obj;
    }

    template <typename T>
    bool hasType() const {
        return &mMetaObj == &T::staticMetaObject;
    }

    template <typename T>
    QList<T*> getObjects() const {
        if(!hasType<T>()) RuntimeThrow("Incompatible type");
        QList<T*> result;
        for(const auto& obj : mObjects) {
            const auto ObjT = enve::cast<T*>(obj);
            if(ObjT) result << ObjT;
        }
        return result;
    }

    template <typename T>
    T* getObject(const int index) const {
        return enve::cast<T*>(mObjects.at(index));
    }

    int count() const { return mObjects.count(); }

    bool hasObject(QObject* const obj) const {
        for(const auto& iObj : mObjects) {
            if(iObj == obj) return true;
        }
        return false;
    }
private:
    const QMetaObject& mMetaObj;
    QList<QObject*> mObjects;
};

class eMimeData : public QMimeData, public eDraggedObjects {
public:
    template <typename T>
    eMimeData(const QList<T*>& objs) : eDraggedObjects(objs) {}

    template <typename T>
    static bool sHasType(const QMimeData* const data) {
        if(!data->hasFormat("eMimeData")) return false;
        return static_cast<const eMimeData*>(data)->hasType<T>();
    }
protected:
    bool hasFormat(const QString &mimetype) const {
        if(mimetype == "eMimeData") return true;
        return false;
    }
};

#endif // EMIMEDATA_H
