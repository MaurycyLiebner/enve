#ifndef NODEPOINTVALUES_H
#define NODEPOINTVALUES_H
#include <QPointF>

struct NodePointValues {
    NodePointValues(const QPointF &c0,
                    const QPointF &p1,
                    const QPointF &c2) {
        fC0 = c0;
        fP1 = p1;
        fC2 = c2;
    }

    NodePointValues() {}

    QPointF fC0;
    QPointF fP1;
    QPointF fC2;

    NodePointValues &operator/=(const qreal &val) {
        qreal inv = 1./val;
        fC0 *= inv;
        fP1 *= inv;
        fC2 *= inv;
        return *this;
    }

    NodePointValues &operator*=(const qreal &val) {
        fC0 *= val;
        fP1 *= val;
        fC2 *= val;
        return *this;
    }

    NodePointValues &operator+=(const NodePointValues &ppv) {
        fC0 += ppv.fC0;
        fP1 += ppv.fP1;
        fC2 += ppv.fC2;
        return *this;
    }

    NodePointValues &operator-=(const NodePointValues &ppv) {
        fC0 -= ppv.fC0;
        fP1 -= ppv.fP1;
        fC2 -= ppv.fC2;
        return *this;
    }
};

NodePointValues operator+(const NodePointValues &ppv1, const NodePointValues &ppv2);
NodePointValues operator-(const NodePointValues &ppv1, const NodePointValues &ppv2);
NodePointValues operator/(const NodePointValues &ppv, const qreal &val);
NodePointValues operator*(const NodePointValues &ppv, const qreal &val);
NodePointValues operator*(const qreal &val, const NodePointValues &ppv);

#endif // NODEPOINTVALUES_H
