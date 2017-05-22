#include "paintcontroler.h"

PaintControler::PaintControler(QObject *parent) : QObject(parent)
{

}

void PaintControler::updateBoxPixmaps(BoundingBox *box) {
    box->processUpdate();
    emit finishedUpdatingLastBox();
}
