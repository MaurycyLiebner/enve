#include "paintcontroler.h"

PaintControler::PaintControler(const int &id,
                               QObject *parent) : QObject(parent) {
    mId = id;
}

void PaintControler::updateUpdatable(_Executor *updatable,
                                     const int &targetId) {
    if(targetId == mId) {
        updatable->_processUpdate();
        emit finishedUpdating(mId, updatable);
    }
}
