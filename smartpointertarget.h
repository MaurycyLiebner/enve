#ifndef SMARTPOINTERTARGET_H
#define SMARTPOINTERTARGET_H


class SmartPointerTarget
{
public:
    SmartPointerTarget();
    virtual ~SmartPointerTarget() {}

    void incNumberPointers();
    void decNumberPointers();

    void blockPointer();
private:
    int mNumberOfPointers = 0;
    bool mBlocked = false;
};

#endif // SMARTPOINTERTARGET_H
