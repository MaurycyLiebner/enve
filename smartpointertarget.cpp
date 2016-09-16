#include "smartpointertarget.h"
#include <QDebug>
SmartPointerTarget::SmartPointerTarget()
{

}

void SmartPointerTarget::incNumberPointers()
{
    mNumberOfPointers++;
}

void SmartPointerTarget::decNumberPointers()
{
    mNumberOfPointers--;
    if(mNumberOfPointers > 0) return;
    delete this;
}
