#include "smartpointertarget.h"
#include <QDebug>
SmartPointerTarget::SmartPointerTarget() {
    mPointerToThis = this;
}

void SmartPointerTarget::incNumberPointers()
{
    mNumberOfPointers++;
}

void SmartPointerTarget::decNumberPointers()
{
    if(mBlocked) return;
    mNumberOfPointers--;
    if(mNumberOfPointers > 0) return;
    delete this;
}

void SmartPointerTarget::blockPointer()
{
    mBlocked = true;
}
