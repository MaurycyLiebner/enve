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

#ifndef DYNAMICCOMPLEXANIMATOR_H
#define DYNAMICCOMPLEXANIMATOR_H
#include <QApplication>
#include "complexanimator.h"
#include "typemenu.h"

template <class T>
class DynamicComplexAnimatorBase : public ComplexAnimator {
protected:
    DynamicComplexAnimatorBase(const QString &name) :
        ComplexAnimator(name) {}
public:
    virtual qsptr<T> createDuplicate(T* const src) = 0;

    bool SWT_dropSupport(const QMimeData* const data) {
        return eMimeData::sHasType<T>(data);
    }

    bool SWT_dropIntoSupport(const int index, const QMimeData* const data) {
        Q_UNUSED(index);
        return eMimeData::sHasType<T>(data);
    }

    bool SWT_drop(const QMimeData* const data) {
        return SWT_dropInto(0, data);
    }

    bool SWT_dropInto(const int index, const QMimeData* const data) {
        const auto eData = static_cast<const eMimeData*>(data);
        const auto bData = static_cast<const eDraggedObjects*>(eData);
        const bool duplicate = QApplication::queryKeyboardModifiers() & Qt::CTRL;
        for(int i = 0; i < bData->count(); i++) {
            const auto iObj = bData->getObject<T>(i);
            if(duplicate) insertChild(createDuplicate(iObj), index + i);
            else insertChild(iObj->template ref<T>(), index + i);
        }
        return true;
    }

    void setupTreeViewMenu(PropertyMenu * const menu) {
        ComplexAnimator::setupTreeViewMenu(menu);
        menu->addSeparator();

        const PropertyMenu::PlainSelectedOp<DynamicComplexAnimatorBase<T>> dOp =
        [](DynamicComplexAnimatorBase<T> * animTarget) {
            animTarget->ca_removeAllChildAnimators();
        };
        menu->addPlainAction("Clear", dOp)->setEnabled(anim_getKeyOnCurrentFrame());
    }


    void insertChild(const qsptr<T>& newChild, const int index) {
        clearOldParent(newChild);
        ca_insertChild(newChild, index);
    }

    void addChild(const qsptr<T>& newChild) {
        clearOldParent(newChild);
        ca_addChild(newChild);
    }

    void removeChild(const qsptr<T>& child) {
        ca_removeChild(child);
    }

    T* getChild(const int index) const {
        return ca_getChildAt<T>(index);
    }

    qsptr<T> takeChildAt(const int index) {
        return ca_takeChildAt(index)->template ref<T>();
    }

    void prependChild(T * const oldChild, const qsptr<T>& newChild) {
        clearOldParent(newChild);
        ca_prependChildAnimator(oldChild, newChild);
    }

    void replaceChild(const qsptr<T>& oldChild, const qsptr<T> &newChild) {
        clearOldParent(newChild);
        ca_replaceChildAnimator(oldChild, newChild);
    }

    using ComplexAnimator::ca_removeAllChildAnimators;
private:
    using ComplexAnimator::ca_addChild;
    using ComplexAnimator::ca_insertChild;
    using ComplexAnimator::ca_prependChildAnimator;

    using ComplexAnimator::ca_removeChild;
    using ComplexAnimator::ca_replaceChildAnimator;
    using ComplexAnimator::ca_takeChildAt;

    void clearOldParent(const qsptr<T>& futureChild) {
        const auto oldParent = futureChild->template getParent
                <DynamicComplexAnimatorBase<T>>();
        if(oldParent) oldParent->removeChild(futureChild);
    }
};

template <class T>
qsptr<T> TCreateOnly(eReadStream& src) {
    Q_UNUSED(src);
    return enve::make_shared<T>();
}

template <class T,
          void (T::*TWriteType)(eWriteStream& dst) const = nullptr,
          qsptr<T> (*TReadTypeAndCreate)(eReadStream& src) = &TCreateOnly<T>>
class DynamicComplexAnimator : public DynamicComplexAnimatorBase<T> {
    e_OBJECT
protected:
    DynamicComplexAnimator(const QString &name) :
        DynamicComplexAnimatorBase<T>(name) {}
public:
    qsptr<T> createDuplicate(T* const src) {
        QBuffer buffer;
        buffer.open(QIODevice::ReadWrite);
        eWriteStream writeStream(&buffer);
        if(TWriteType) (src->*TWriteType)(writeStream);
        src->writeProperty(writeStream);
        writeStream.writeFutureTable();
        buffer.seek(0);
        eReadStream readStream(&buffer);
        buffer.seek(buffer.size() - qint64(sizeof(int)));
        readStream.readFutureTable();
        buffer.seek(0);
        const auto duplicate = TReadTypeAndCreate(readStream);
        duplicate->readProperty(readStream);
        buffer.close();
        return duplicate;
    }

    void writeProperty(eWriteStream& dst) const {
        const int nProps = this->ca_mChildAnimators.count();
        dst << nProps;
        for(const auto& prop : this->ca_mChildAnimators) {
            const auto futureId = dst.planFuturePos();
            const auto TProp = static_cast<T*>(prop.get());
            if(TWriteType) (TProp->*TWriteType)(dst);
            TProp->writeProperty(dst);
            dst.assignFuturePos(futureId);
        }
    }

    void readProperty(eReadStream& src) {
        int nProps;
        src >> nProps;
        for(int i = 0; i < nProps; i++) {
            const auto futurePos = src.readFuturePos();
            try {
                const auto prop = TReadTypeAndCreate(src);
                prop->readProperty(src);
                this->addChild(prop);
            } catch(const std::exception& e) {
                src.seek(futurePos);
                gPrintExceptionCritical(e);
            }
        }
    }
};

#endif // DYNAMICCOMPLEXANIMATOR_H
