#include "paintcontroler.h"

PaintControler::PaintControler(QObject *parent) : QObject(parent)
{

}

void PaintControler::updateBoxPrettyPixmap(BoundingBox *box) {
    box->updatePrettyPixmap();
    emit finishedPrettyUpdatingLastBox();
}

void PaintControler::updateBoxPixmaps(BoundingBox *box) {
    box->updatePixmaps();
    emit finishedUpdatingLastBox();
}
