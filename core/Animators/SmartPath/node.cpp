#include "node.h"
#include "exceptions.h"

Node Node::sInterpolate(const Node &node1, const Node &node2,
                        const qreal &weight2) {
    Node result;
    if(node1.isNormal() && node2.isNormal())
        result = sInterpolateNormal(node1, node2, weight2);
    else if(node1.isDissolved() && node2.isDissolved())
        result = Node(node1.fT*(1 - weight2) + node2.fT*weight2);
    else if(node1.isDummy() && node2.isDissolved())
        result = Node(0.5*(1 - weight2) + node2.fT*weight2);
    else if(node1.isDissolved() && node2.isDummy())
        result = Node(node1.fT*(1 - weight2) + 0.5*weight2);
    else if(node1.isDummy() && node2.isDummy())
        result = Node(Node::DUMMY);
    else if(node1.isMove() && node2.isMove())
        result = Node(Node::MOVE);
    else RuntimeThrow("Unsupported");
    if(node1.getNextNodeId() != node2.getNextNodeId() ||
       node1.getPrevNodeId() != node2.getPrevNodeId())
        RuntimeThrow("Cannot interpolate neighbours");
    result.setNextNodeId(node1.getNextNodeId());
    result.setPrevNodeId(node1.getPrevNodeId());
    return result;
}

Node Node::sInterpolateNormal(const Node &node1, const Node &node2,
                              const qreal& weight2) {
    if(!node1.isNormal() || !node2.isNormal())
        RuntimeThrow("Unsupported node type");
//    if(isZero6Dec(weight2)) return node1;
//    if(isOne6Dec(weight2)) return node2;
    const qreal w1 = 1 - weight2;
    Node result(w1*node1.fC0 + weight2*node2.fC0,
                w1*node1.fP1 + weight2*node2.fP1,
                w1*node1.fC2 + weight2*node2.fC2);
    result.setC0Enabled(node1.getC0Enabled() || node2.getC0Enabled());
    result.setC2Enabled(node1.getC2Enabled() || node2.getC2Enabled());
    const CtrlsMode node1Ctrls = node1.getCtrlsMode();
    const CtrlsMode node2Ctrls = node2.getCtrlsMode();
    if(node1Ctrls == node2Ctrls) {
        result.setCtrlsMode(node1.getCtrlsMode());
    } else if((node1Ctrls == CtrlsMode::CTRLS_SYMMETRIC &&
               node2Ctrls == CtrlsMode::CTRLS_SMOOTH) ||
              (node1Ctrls == CtrlsMode::CTRLS_SMOOTH &&
               node2Ctrls == CtrlsMode::CTRLS_SYMMETRIC)) {
        result.setCtrlsMode(CtrlsMode::CTRLS_SMOOTH);
    } else {
        result.setCtrlsMode(CtrlsMode::CTRLS_CORNER);
    }
    return result;
}
