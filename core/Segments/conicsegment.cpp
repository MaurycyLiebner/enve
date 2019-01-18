#include "conicsegment.h"
#include "quadsegment.h"
#include "qcubicsegment2d.h"
#include <QList>

//template<typename T, typename Tw>
//QList<QuadSegment<T> > ConicSegment<T, Tw>::toQuads() const {
//    SkAutoConicToQuads helper;
//    const SkPoint * const pts =
//            helper.computeQuads(SkConic(fP0, fP1, fP2, fW), 0.1f);
//    int nQuads = helper.countQuads();

//    QList<QuadSegment<T>> segs;
//    int ptId = 0;
//    for(int i = 0; i < nQuads; i++) {
//        segs.append({pts[ptId++], pts[ptId++], pts[ptId]});
//    }
//    return segs;
//}
