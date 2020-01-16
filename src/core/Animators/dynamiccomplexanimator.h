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

#ifndef DYNAMICCOMPLEXANIMATOR_H
#define DYNAMICCOMPLEXANIMATOR_H
#include <QApplication>
#include "complexanimator.h"
#include "typemenu.h"
#include "Properties/emimedata.h"

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
        Q_UNUSED(index)
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

    void prp_setupTreeViewMenu(PropertyMenu * const menu) {
        ComplexAnimator::prp_setupTreeViewMenu(menu);
        menu->addSeparator();

        const PropertyMenu::PlainSelectedOp<DynamicComplexAnimatorBase<T>> dOp =
        [](DynamicComplexAnimatorBase<T> * animTarget) {
            animTarget->ca_removeAllChildren();
        };
        menu->addPlainAction("Clear", dOp)->setEnabled(this->ca_getNumberOfChildren());
    }

    void insertChild(const qsptr<T>& child, const int index) {
        clearOldParent(child);
        ca_insertChild(child, index);

        {
            prp_pushUndoRedoName("Insert " + child->prp_getName());
            UndoRedo ur;
            ur.fUndo = [this, child]() {
                removeChild(child);
            };
            ur.fRedo = [this, child, index]() {
                insertChild(child, index);
            };
            prp_addUndoRedo(ur);
        }
    }

    void addChild(const qsptr<T>& newChild) {
        insertChild(newChild, ca_getNumberOfChildren());
    }

    void removeChild(const qsptr<T>& child) {
        const int index = ca_getChildPropertyIndex(child.get());
        ca_removeChild(child);

        {
            prp_pushUndoRedoName("Remove " + child->prp_getName());
            UndoRedo ur;
            ur.fUndo = [this, child]() {
                removeChild(child);
            };
            ur.fRedo = [this, child, index]() {
                insertChild(child, index);
            };
            prp_addUndoRedo(ur);
        }
    }

    T* getChild(const int index) const {
        return ca_getChildAt<T>(index);
    }

    qsptr<T> takeChildAt(const int index) {
        const auto child = getChild(index)->template ref<T>();
        removeChild(child);
        return child;
    }

    void swapChildren(const int id1, const int id2) {
        ca_swapChildren(id1, id2);

        {
            prp_pushUndoRedoName("Swap");
            UndoRedo ur;
            ur.fUndo = [this, id1, id2]() {
                swapChildren(id2, id1);
            };
            ur.fRedo = [this, id1, id2]() {
                swapChildren(id1, id2);
            };
            prp_addUndoRedo(ur);
        }
    }

    void saveOrder() {
        mSavedOrder = ca_getChildren();
    }

    void restoreOrder() {
        restoreOrder(mSavedOrder);
    }

    void finishOrder() {
        prp_pushUndoRedoName("Reorder");
        UndoRedo ur;
        const auto oldValue = mSavedOrder;
        const auto newValue = ca_getChildren();
        ur.fUndo = [this, oldValue]() {
            restoreOrder(oldValue);
        };
        ur.fRedo = [this, newValue]() {
            restoreOrder(newValue);
        };
        prp_addUndoRedo(ur);
    }

    void swapChildrenTemporary(const int id1, const int id2) {
        ca_swapChildren(id1, id2);
    }

    using ComplexAnimator::ca_removeAllChildren;
private:
    using ComplexAnimator::ca_addChild;
    using ComplexAnimator::ca_insertChild;
    using ComplexAnimator::ca_prependChild;

    using ComplexAnimator::ca_removeChild;
    using ComplexAnimator::ca_replaceChild;
    using ComplexAnimator::ca_takeChildAt;

    using ComplexAnimator::ca_swapChildren;
    using ComplexAnimator::ca_moveChildInList;
    using ComplexAnimator::ca_moveChildBelow;
    using ComplexAnimator::ca_moveChildAbove;

    void restoreOrder(const QList<qsptr<Property>>& order) {
        ca_removeAllChildren();
        for(const auto& child : order)
            ca_addChild(child);
    }

    void clearOldParent(const qsptr<T>& futureChild) {
        const auto oldParent = futureChild->template getParent
                <DynamicComplexAnimatorBase<T>>();
        if(oldParent) oldParent->removeChild(futureChild);
    }

    QList<qsptr<Property>> mSavedOrder;
};

template <class T>
qsptr<T> TCreateOnly(eReadStream& src) {
    Q_UNUSED(src)
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
    qsptr<T> createDuplicate(T* const src) override {
        QBuffer buffer;
        buffer.open(QIODevice::ReadWrite);
        eWriteStream writeStream(&buffer);
        if(TWriteType) (src->*TWriteType)(writeStream);
        src->prp_writeProperty(writeStream);
        writeStream.writeFutureTable();
        buffer.seek(0);
        eReadStream readStream(&buffer);
        buffer.seek(buffer.size() - qint64(sizeof(int)));
        readStream.readFutureTable();
        buffer.seek(0);
        const auto duplicate = TReadTypeAndCreate(readStream);
        duplicate->prp_readProperty(readStream);
        buffer.close();
        return duplicate;
    }

    void prp_writeProperty(eWriteStream& dst) const override {
        const auto& children = this->ca_getChildren();
        const int nProps = children.count();
        dst << nProps;
        for(const auto& prop : children) {
            const auto futureId = dst.planFuturePos();
            const auto TProp = static_cast<T*>(prop.get());
            if(TWriteType) (TProp->*TWriteType)(dst);
            TProp->prp_writeProperty(dst);
            dst.assignFuturePos(futureId);
        }
    }

    void prp_readProperty(eReadStream& src) override {
        int nProps;
        src >> nProps;
        for(int i = 0; i < nProps; i++) {
            const auto futurePos = src.readFuturePos();
            try {
                const auto prop = TReadTypeAndCreate(src);
                prop->prp_readProperty(src);
                this->addChild(prop);
            } catch(const std::exception& e) {
                src.seek(futurePos);
                gPrintExceptionCritical(e);
            }
        }
    }
};

#endif // DYNAMICCOMPLEXANIMATOR_H
