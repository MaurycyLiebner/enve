#include "paintcontroler.h"

PaintControler::PaintControler(QObject *parent) : QObject(parent)
{

}

void PaintControler::updateUpdatable(Updatable *updatable) {
    updatable->processUpdate();
    emit finishedUpdating();
}
