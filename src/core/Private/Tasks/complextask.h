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

#ifndef COMPLEXTASK_H
#define COMPLEXTASK_H

#include <QObject>

#include "Tasks/updatable.h"

class ComplexTask : public QObject {
    Q_OBJECT
    friend class TaskScheduler;
public:
    ComplexTask(const QString& name);

    void addPlannedTasks(const int tasks);
    void removePlannedTasks(const int tasks);
    void addPlannedTask(const stdsptr<eTask>& task);
    void addTask(const stdsptr<eTask>& task);

    void cancel();

    int count() const;
    const QString& name() const;
    int finishedCount() const;

    bool done() const { return mDone; }
signals:
    void finished(const int nFinished);
    void finishedAll();
    void canceled();
private:
    void start() { mStarted = true; }
    void finishedEmitters();
    void incFinished();

    bool mStarted = false;
    bool mDone = false;
    int mPlannedCount = 0;
    int mFinishedCount = 0;
    const QString mName;
    QList<stdsptr<eTask>> mTasks;
};

#endif // COMPLEXTASK_H
