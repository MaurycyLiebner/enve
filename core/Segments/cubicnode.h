#ifndef CUBICNODE_H
#define CUBICNODE_H
#include <QPointF>

struct CubicNode {
public:
    CubicNode(const QPointF& c1, const QPointF& p, const QPointF& c2) {
        mC1 = c1; mP = p; mC2 = c2;
    }
private:
    QPointF mC1;
    QPointF mP;
    QPointF mC2;
};

#endif // CUBICNODE_H
