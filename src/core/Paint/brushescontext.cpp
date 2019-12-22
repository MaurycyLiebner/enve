#include "brushescontext.h"
QList<BrushCollectionData> BrushCollectionData::sData;

BrushContexedCollection::BrushContexedCollection(BrushesContext * const context,
                                                 const BrushCollectionData &raw) {
    fName = raw.fName;
    for(const auto& brushRaw : raw.fBrushes) {
        fBrushes << enve::make_shared<BrushContexedWrapper>(context, brushRaw);
    }
}

BrushesContext::BrushesContext(const QList<BrushCollectionData> &raw) {
    for(const auto& coll : raw) {
        fCollections << BrushContexedCollection(this, coll);
    }
}

BrushContexedWrapper *BrushesContext::brushWrapper(
        const SimpleBrushWrapper * const brush) {
    for(const auto& coll : fCollections) {
        for(auto& wrapper : coll.fBrushes) {
            if(wrapper->getSimpleBrush() == brush) {
                return wrapper.get();
            }
        }
    }
    return nullptr;
}

void BrushesContext::addBookmark(BrushContexedWrapper * const brush) {
    if(fBookmarked.contains(brush)) return;
    fBookmarked << brush;
    emit bookmarkAdded(brush);
}

void BrushesContext::removeBookmark(BrushContexedWrapper * const brush) {
    if(fBookmarked.removeOne(brush)) {
        emit bookmarkRemoved(brush);
    }
}

bool BrushesContext::setSelectedWrapper(SimpleBrushWrapper * const wrapper) {
    for(const auto& coll : fCollections) {
        for(auto& brush : coll.fBrushes) {
            if(brush->getSimpleBrush() == wrapper) {
                brush->setSelected(true);
                return true;
            }
        }
    }
    return false;
}
