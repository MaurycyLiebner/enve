#ifndef TYPEMENU_H
#define TYPEMENU_H

#include <QMenu>
#include <typeindex>
#include "smartPointers/sharedpointerdefs.h"
#include "canvas.h"

template <typename Type>
class TypeMenu {
    friend class Canvas;
    using TTypeMenu = TypeMenu<Type>;
public:
    template <class T> using PlainOp = std::function<void(T*)>;
    template <class T> using CheckOp = std::function<void(T*, bool)>;
    template <class T> using AllOp = std::function<void(QList<T*>)>;

    TypeMenu(QMenu * const targetMenu, Canvas * const targetCanvas,
             QWidget * const parent) :
        mTargetMenu(targetMenu),
        mTargetCanvas(targetCanvas),
        mParentWidget(parent) {}

    template <class T>
    QAction* addCheckableAction(const QString& text, const bool& checked,
                                const CheckOp<T>& op) {
        QAction * const qAction = mTargetMenu->addAction(text);
        qAction->setCheckable(true);
        qAction->setChecked(checked);
        const PlainOp<T> plainOp = [op, checked](T* pt) {
            op(pt, checked);
        };
        connectAction(qAction, plainOp);
        return qAction;
    }

    template <class T>
    QAction* addPlainAction(const QString& text, const PlainOp<T>& op) {
        QAction * const qAction = mTargetMenu->addAction(text);
        connectAction(qAction, op);
        return qAction;
    }

    template <class T>
    QAction* addAllAction(const QString& text, const AllOp<T>& op) {
        QAction * const qAction = mTargetMenu->addAction(text);
        connectAllAction(qAction, op);
        return qAction;
    }

    TTypeMenu * addMenu(const QString& title) {
        QMenu * const qMenu = mTargetMenu->addMenu(title);
        const auto child = std::make_shared<TTypeMenu>(qMenu, mTargetCanvas,
                                                       mParentWidget);
        mChildMenus.append(child);
        return child.get();
    }

    void addSeparator() {
        mTargetMenu->addSeparator();
    }

    bool isEmpty() {
        return mTargetMenu->isEmpty();
    }

    void clear() {
        mTargetMenu->clear();
        mChildMenus.clear();
        mTypeIndex.clear();
    }

    QWidget* getParentWidget() const {
        return mParentWidget;
    }
protected:
    void addedActionsForType(Type * const obj) {
        mTypeIndex.append(std::type_index(typeid(obj)));
    }

    bool hasActionsForType(Type * const obj) const {
        return mTypeIndex.contains(std::type_index(typeid(obj)));
    }
private:
    template <typename U>
    void connectAction(BoundingBox * const,
                       QAction * const qAction,
                       const U& op) {
        const auto targetCanvas = mTargetCanvas;
        const auto canvasOp = [op, targetCanvas]() {
            targetCanvas->execOpOnSelectedBoxes(op);
        };
        QObject::connect(qAction, &QAction::triggered, canvasOp);
    }

    template <typename U>
    void connectAction(MovablePoint * const,
                       QAction * const qAction,
                       const U& op) {
        const auto targetCanvas = mTargetCanvas;
        const auto canvasOp = [op, targetCanvas]() {
            targetCanvas->execOpOnSelectedPoints(op);
        };
        QObject::connect(qAction, &QAction::triggered, canvasOp);
    }

    template <class T>
    void connectAction(QAction * const qAction,
                       const PlainOp<T>& op) {
        connectAction(static_cast<T*>(nullptr), qAction, op);
    }

    template <class T>
    void connectAction(QAction * const qAction,
                       const AllOp<T>& op) {
        connectAction(static_cast<T*>(nullptr), qAction, op);
    }

    QMenu * const mTargetMenu;
    Canvas * const mTargetCanvas;
    QWidget * const mParentWidget;

    QList<stdsptr<TTypeMenu>> mChildMenus;
    QList<std::type_index> mTypeIndex;
};

typedef TypeMenu<MovablePoint> PointTypeMenu;
typedef TypeMenu<BoundingBox> BoxTypeMenu;
typedef  TypeMenu<Property> PropertyTypeMenu;

#endif // TYPEMENU_H
