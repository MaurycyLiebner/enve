#ifndef SIMPLETASK_H
#define SIMPLETASK_H
#include <QObject>
#include <functional>

class SimpleTask : public QObject {
    Q_OBJECT
    typedef std::function<void(void)> Func;
public:
    static SimpleTask *sSchedule(const Func& func);
    static void sProcessAll();
private:
    SimpleTask(const Func& func);
    void process() {
        mFunc();
        emit finished();
    }

    const Func mFunc;
    static QList<SimpleTask*> sTasks;
signals:
    void finished();
};

class SimpleTaskScheduler : public QObject {
    typedef std::function<void(void)> Func;
public:
    SimpleTaskScheduler(const Func& func);

    void schedule();
    void operator()() { schedule(); }
private:
    bool mScheduled = false;
    const Func mFunc;
};

#endif // SIMPLETASK_H
