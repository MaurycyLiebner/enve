#ifndef POINTTYPEMENU_H
#define POINTTYPEMENU_H

#include <QMenu>
#include <typeindex>
#include "smartPointers/sharedpointerdefs.h"
#include "canvas.h"

class MovablePoint;
class PointTypeMenu {
    friend class Canvas;
public:
    PointTypeMenu(QMenu * const targetMenu, Canvas * const targetCanvas) :
        mTargetMenu(targetMenu), mTargetCanvas(targetCanvas) {}

    template <class T>
    QAction* addCheckableAction(const QString& text, const bool& checked,
                            const std::function<void(T*, bool)>& op) {
        QAction * const qAction = mTargetMenu->addAction(text);
        qAction->setCheckable(true);
        qAction->setChecked(checked);
        const auto plainOp = [op, checked](T* pt) {
            op(pt, checked);
        };
        connectPlainAction(qAction, plainOp);
        return qAction;
    }

    template <class T>
    QAction* addPlainAction(const QString& text, const std::function<void(T*)>& op) {
        QAction * const qAction = mTargetMenu->addAction(text);
        connectPlainAction(qAction, op);
        return qAction;
    }

    PointTypeMenu * addMenu(const QString& title) {
        QMenu * const qMenu = mTargetMenu->addMenu(title);
        const auto child = std::make_shared<PointTypeMenu>(qMenu, mTargetCanvas);
        mChildMenus.append(child);
        return child.get();
    }

    bool isEmpty() {
        return mTargetMenu->isEmpty();
    }
protected:
    void addedActionsForPointType(MovablePoint * const pt) {
        mTypeIndex.append(std::type_index(typeid(pt)));
    }

    bool hasActionsForPointType(MovablePoint * const pt) const {
        return mTypeIndex.contains(std::type_index(typeid(pt)));
    }
private:
    template <class T>
    void connectPlainAction(QAction * const qAction,
                            const std::function<void(T*)>& op) {
        const auto targetCanvas = mTargetCanvas;
        const auto canvasOp = [op, targetCanvas]() {
            targetCanvas->execOpOnSelectedPoints(op);
        };
        QObject::connect(qAction, &QAction::triggered, canvasOp);
    }

    QMenu * const mTargetMenu;
    Canvas * const mTargetCanvas;
    QList<stdsptr<PointTypeMenu>> mChildMenus;
    QList<std::type_index> mTypeIndex;
};
#endif // POINTTYPEMENU_H
