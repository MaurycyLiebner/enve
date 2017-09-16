#include "paintcontroler.h"

PaintControler::PaintControler(const int &id,
                               QObject *parent) : QObject(parent) {
    mId = id;
}

void PaintControler::updateUpdatable(Executable *updatable,
                                     const int &targetId) {
    if(targetId == mId) {
        updatable->processUpdate();
        emit finishedUpdating(mId, updatable);
    }
}
