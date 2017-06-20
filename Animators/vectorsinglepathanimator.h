#ifndef VECTORSINGLEPATHANIMATOR_H
#define VECTORSINGLEPATHANIMATOR_H
#include "singlepathanimator.h"
class PathAnimator;

class VectorSinglePathAnimator : public SinglePathAnimator
{
public:
    VectorSinglePathAnimator(PathAnimator *parentAnimator);
};

#endif // VECTORSINGLEPATHANIMATOR_H
