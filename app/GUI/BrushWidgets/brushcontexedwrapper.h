#ifndef BRUSHCONTEXEDWRAPPER_H
#define BRUSHCONTEXEDWRAPPER_H
#include "simplebrushwrapper.h"
#include <QImage>
struct BrushData {
    QString fName;
    stdsptr<SimpleBrushWrapper> fWrapper;
    QImage fIcon;
    QByteArray fWholeFile;
};

class BrushContexedWrapper : public SelfRef {
    Q_OBJECT
    friend class SelfRef;
public:
    void setSelected(const bool& selected) {
        if(selected == mSelected) return;
        mSelected = selected;
        emit selectionChanged(mSelected);
    }

    const bool& selected() const {
        return mSelected;
    }

    const BrushData& getBrushData() const {
        return mBrushData;
    }

    SimpleBrushWrapper * getSimpleBrush() {
        return mBrushData.fWrapper.get();
    }
protected:
    BrushContexedWrapper(const BrushData& brushData) :
        mBrushData(brushData) {}
signals:
    void selectionChanged(bool);
private:
    bool mSelected = false;
    const BrushData& mBrushData;
};

#endif // BRUSHCONTEXEDWRAPPER_H
