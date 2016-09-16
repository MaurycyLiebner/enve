#ifndef SMARTPOINTERTARGET_H
#define SMARTPOINTERTARGET_H


class SmartPointerTarget
{
public:
    SmartPointerTarget();
    virtual ~SmartPointerTarget() {}

    void incNumberPointers();
    void decNumberPointers();
private:
    int mNumberOfPointers = 0;
};

#endif // SMARTPOINTERTARGET_H
