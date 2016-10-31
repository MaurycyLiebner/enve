#include "paintcontroler.h"

PaintControler::PaintControler(QObject *parent) : QObject(parent)
{

}

void PaintControler::updateBoxPrettyPixmap(BoundingBox *box)
{
    box->updatePrettyPixmap();
    emit finishedUpdatingLastBox();
}
