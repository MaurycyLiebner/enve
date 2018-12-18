#ifndef TASKEXECUTOR_H
#define TASKEXECUTOR_H
#include <QObject>
#include <QTimer>
#include "Boxes/boundingbox.h"

class TaskExecutor : public QObject {
    Q_OBJECT
public:
    explicit TaskExecutor(const int &id,
                            QObject *parent = nullptr);

signals:
    void finishedUpdating(int, _ScheduledTask*);
public slots:
    void updateUpdatable(_ScheduledTask * const updatable,
                         const int &targetId);
private:
    int mId;
};

#endif // TASKEXECUTOR_H
