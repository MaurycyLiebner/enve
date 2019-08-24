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

#ifndef STACKLAYOUTS_H
#define STACKLAYOUTS_H
#include <memory>
#include <QWidget>
#include "basicreadwrite.h"

#include "stackwidgetwrapper.h"

struct WidgetStackLayoutItem;
struct ParentStackLayoutItem;
struct SplitStackLayoutItem;

class Document;
class AudioHandler;

struct StackLayoutItem {
    enum Type {
        V_SPLIT, H_SPLIT,
        WIDGET, NONE
    };

    typedef std::unique_ptr<StackLayoutItem> UniPtr;
    typedef std::unique_ptr<SplitStackLayoutItem> SplitPtr;
    typedef std::unique_ptr<WidgetStackLayoutItem> WidgetPtr;
    virtual ~StackLayoutItem() {}
    virtual QWidget* create(Document &document,
                            QWidget* const parent,
                            QLayout* const layout = nullptr) = 0;
    virtual void write(eWriteStream& dst) const = 0;
    virtual void saveData() = 0;

    void writeType(eWriteStream& dst) const {
        dst.write(rcConstChar(&mType), sizeof(Type));
    }

    void setParent(ParentStackLayoutItem* const parent) {
        mParent = parent;
    }
protected:
    Type mType = NONE;
    ParentStackLayoutItem* mParent = nullptr;
};

struct ParentStackLayoutItem : public StackLayoutItem {
    virtual void childClosed_k(StackLayoutItem* const child) = 0;
    virtual UniPtr replaceChild(StackLayoutItem* const from,
                                UniPtr&& to) = 0;
protected:
    template <typename WidgetT>
    static UniPtr sReadChild(eReadStream& src);
    static void sWriteChild(StackLayoutItem* const child,
                            eWriteStream &dst);
};

struct SplitStackLayoutItem : public ParentStackLayoutItem {
    void write(eWriteStream& dst) const {
        dst << mSecondSizeFrac;
        sWriteChild(mChildItems.first.get(), dst);
        sWriteChild(mChildItems.second.get(), dst);
    }

    template <typename WidgetT>
    void read(eReadStream& src) {
        src >> mSecondSizeFrac;
        auto child1 = sReadChild<WidgetT>(src);
        auto child2 = sReadChild<WidgetT>(src);
        setChildren(std::move(child1), std::move(child2));
    }

    void setChildren(UniPtr&& child1, UniPtr&& child2) {
        if(child1) child1->setParent(this);
        if(child2) child2->setParent(this);
        mChildItems.first = std::move(child1);
        mChildItems.second = std::move(child2);
    }

    void childClosed_k(StackLayoutItem* const child) {
        if(mChildItems.first.get() == child) {
            mParent->replaceChild(this, std::move(mChildItems.second));
        } else if(mChildItems.second.get() == child) {
            mParent->replaceChild(this, std::move(mChildItems.first));
        }
    }

    UniPtr replaceChild(StackLayoutItem* const from,
                        UniPtr&& to) {
        if(mChildItems.first.get() == from) {
            UniPtr tmp = std::move(mChildItems.first);
            if(from) from->setParent(nullptr);
            if(to) to->setParent(this);
            mChildItems.first = std::move(to);
            return tmp;
        } else if(mChildItems.second.get() == from) {
            UniPtr tmp = std::move(mChildItems.second);
            if(from) from->setParent(nullptr);
            if(to) to->setParent(this);
            mChildItems.second = std::move(to);
            return tmp;
        }
        return nullptr;
    }

    void saveData() {
        mChildItems.first->saveData();
        mChildItems.second->saveData();
    }
protected:
    std::pair<UniPtr, UniPtr> mChildItems;
    qreal mSecondSizeFrac = 0.5;
};

struct SplittableStackItem : StackLayoutItem {
    VSplitStackItem *splitV(WidgetPtr&& other);
    HSplitStackItem *splitH(WidgetPtr&& other);
private:
    template<class SplitItemClass>
    SplitItemClass* split(WidgetPtr&& other);
};

struct WidgetStackLayoutItem : public SplittableStackItem {
    WidgetStackLayoutItem() { mType = Type::WIDGET; }

    virtual void clear() = 0;
    virtual void read(eReadStream& src) = 0;

    void setCurrent(StackWidgetWrapper* const current) {
        mCurrent = current;
    }

    void saveData() {
        Q_ASSERT(mCurrent);
        mCurrent->saveDataToLayout();
        mCurrent = nullptr;
    }

    void close() {
        if(!mParent) return clear();
        mParent->childClosed_k(this);
    }
protected:
    StackWidgetWrapper* mCurrent = nullptr;
};

struct BaseStackItem : public ParentStackLayoutItem {
    typedef std::unique_ptr<BaseStackItem> UPtr;

    void saveData() {
        mChild->saveData();
    }

    QWidget* create(Document &document,
                    QWidget* const parent,
                    QLayout* const layout = nullptr) {
        return mChild->create(document, parent, layout);
    }

    void childClosed_k(StackLayoutItem* const child) {
        Q_UNUSED(child);
        return;
    }

    UniPtr replaceChild(StackLayoutItem* const from,
                        UniPtr&& to) {
        if(from != mChild.get()) return nullptr;
        return setChild(std::move(to));
    }

    UniPtr setChild(UniPtr&& to) {
        UniPtr tmp = std::move(mChild);
        if(tmp) tmp->setParent(nullptr);
        mChild = std::move(to);
        if(mChild) mChild->setParent(this);
        return tmp;
    }

    StackLayoutItem* getChild() const {
        return mChild.get();
    }

    void setName(const QString& name) const {
        mName = name;
    }

    const QString& getName() const {
        return mName;
    }

    void write(eWriteStream& dst) const {
        dst << mName;
        ParentStackLayoutItem::sWriteChild(mChild.get(), dst);
    }
protected:
    template <typename WidgetT>
    void readBaseStackItem(eReadStream& src) {
        QString name; src >> name;
        setName(name);
        auto child = ParentStackLayoutItem::sReadChild<WidgetT>(src);
        setChild(std::move(child));
    }

    mutable QString mName;
    UniPtr mChild;
};

struct HSplitStackItem : public SplitStackLayoutItem {
public:
    HSplitStackItem() { mType = Type::H_SPLIT; }

    QWidget* create(Document &document,
                    QWidget* const parent,
                    QLayout* const layout = nullptr) {
        const auto split = new HWidgetStack(this, parent);
        if(layout) layout->addWidget(split);
        split->appendWidget(mChildItems.first->create(document, split),
                            1 - mSecondSizeFrac);
        split->appendWidget(mChildItems.second->create(document, split),
                            mSecondSizeFrac);
        return split;
    }

    void saveData() {
        Q_ASSERT(mCurrent);
        SplitStackLayoutItem::saveData();
        mSecondSizeFrac = mCurrent->percentAt(1);
        mCurrent = nullptr;
    }

    void setCurrent(HWidgetStack* const current) {
        mCurrent = current;
    }
private:
    HWidgetStack* mCurrent = nullptr;
};

struct VSplitStackItem : public SplitStackLayoutItem {
public:
    VSplitStackItem() { mType = Type::V_SPLIT; }

    QWidget* create(Document &document,
                    QWidget* const parent,
                    QLayout* const layout = nullptr) {
        const auto split = new VWidgetStack(this, parent);
        if(layout) layout->addWidget(split);
        split->appendWidget(mChildItems.first->create(document, split),
                            1 - mSecondSizeFrac);
        split->appendWidget(mChildItems.second->create(document, split),
                            mSecondSizeFrac);
        return split;
    }

    void saveData() {
        Q_ASSERT(mCurrent);
        SplitStackLayoutItem::saveData();
        mSecondSizeFrac = mCurrent->percentAt(1);
        mCurrent = nullptr;
    }

    void setCurrent(VWidgetStack* const current) {
        mCurrent = current;
    }
private:
    VWidgetStack* mCurrent = nullptr;
};

template <typename WidgetT>
StackLayoutItem::UniPtr ParentStackLayoutItem::sReadChild(eReadStream& src) {
    Type type;
    src.read(rcChar(&type), sizeof(Type));
    if(type == Type::H_SPLIT) {
        const auto hSplit = new HSplitStackItem;
        hSplit->read<WidgetT>(src);
        return UniPtr(hSplit);
    } else if(type == Type::V_SPLIT) {
        const auto vSplit = new VSplitStackItem;
        vSplit->read<WidgetT>(src);
        return UniPtr(vSplit);
    } else if(type == Type::WIDGET) {
        const auto wid = new WidgetT;
        wid->read(src);
        return UniPtr(wid);
    } else Q_ASSERT(false);
    return nullptr;
}

#endif // STACKLAYOUTS_H
