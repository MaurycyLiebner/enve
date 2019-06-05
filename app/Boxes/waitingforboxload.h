#ifndef WAITINGFORBOXLOAD_H
#define WAITINGFORBOXLOAD_H
#include <functional>
class BoundingBox;

struct WaitingForBoxLoad {
    typedef std::function<void(BoundingBox*)> BoxReadFunc;
    typedef std::function<void()> BoxNeverReadFunc;
    WaitingForBoxLoad(const int boxReadId,
                      const BoxReadFunc& boxRead,
                      const BoxNeverReadFunc& boxNeverRead);

    bool boxRead(BoundingBox * const box) const;
    void boxNeverRead() const;
private:
    int mBoxReadId;
    std::function<void(BoundingBox*)> mBoxReadFunction;
    std::function<void()> mBoxNeverReadFunction;
};


#endif // WAITINGFORBOXLOAD_H
