// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "node.h"
#include "exceptions.h"

Node Node::sInterpolateDissolved(const Node &node1, const Node &node2,
                        const qreal weight2) {
    if(!node1.isDissolved() || !node2.isDissolved())
        RuntimeThrow("Unsupported node type");
    const qreal w1 = 1 - weight2;
    return Node(w1*node1.t() + weight2*node2.t());
}

Node Node::sInterpolateNormal(const Node &node1, const Node &node2,
                              const qreal weight2) {
    if(!node1.isNormal() || !node2.isNormal())
        RuntimeThrow("Unsupported node type");
    //if(isZero6Dec(weight2)) return node1;
    //if(isOne6Dec(weight2)) return node2;
    const qreal w1 = 1 - weight2;
    Node result(w1*node1.c0() + weight2*node2.c0(),
                w1*node1.mP1 + weight2*node2.mP1,
                w1*node1.c2() + weight2*node2.c2());
    result.setC0Enabled(node1.getC0Enabled() || node2.getC0Enabled());
    result.setC2Enabled(node1.getC2Enabled() || node2.getC2Enabled());
    const CtrlsMode node1Ctrls = node1.getCtrlsMode();
    const CtrlsMode node2Ctrls = node2.getCtrlsMode();
    if(node1Ctrls == node2Ctrls) {
        result.mCtrlsMode = node1Ctrls;
    } else if(node1Ctrls == CtrlsMode::CTRLS_CORNER ||
              node2Ctrls == CtrlsMode::CTRLS_CORNER) {
        result.mCtrlsMode = CtrlsMode::CTRLS_CORNER;
    } else if(node1Ctrls == CtrlsMode::CTRLS_SMOOTH ||
              node2Ctrls == CtrlsMode::CTRLS_SMOOTH) {
        result.mCtrlsMode = CtrlsMode::CTRLS_SMOOTH;
    } else {
        result.mCtrlsMode = CtrlsMode::CTRLS_SYMMETRIC;
    }
    result.setCtrlsMode(result.mCtrlsMode);
    return result;
}
