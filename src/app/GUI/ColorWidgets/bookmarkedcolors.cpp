#include "bookmarkedcolors.h"
#include "savedcolorbutton.h"
#include "Private/document.h"

BookmarkedColors::BookmarkedColors(const bool vertical,
                                   const int dimension,
                                   QWidget * const parent) :
    BookmarkedWidget(vertical, dimension, parent) {
    connect(Document::sInstance, &Document::bookmarkColorAdded,
            this, &BookmarkedColors::addColor);
    connect(Document::sInstance, &Document::bookmarkColorRemoved,
            this, &BookmarkedColors::removeColor);
    connect(Document::sInstance, &Document::brushColorChanged,
            this, &BookmarkedColors::setCurrentColor);
}

void BookmarkedColors::setCurrentColor(const QColor &color) {
    const int iCount = count();
    for(int i = 0 ; i < iCount; i++) {
        const auto widget = static_cast<SavedColorButton*>(getWidget(i));
        widget->setSelected(widget->getColor().rgba() == color.rgba());
    }
}

void BookmarkedColors::addColor(const QColor &color) {
    const auto button = new SavedColorButton(color);
    connect(button, &SavedColorButton::selected, this, [color]() {
        Document::sInstance->setBrushColor(color);
    });
    addWidget(button);
}

void BookmarkedColors::removeColor(const QColor &color) {
    const auto rgba = color.rgba();
    const int iCount = count();
    for(int i = 0 ; i < iCount; i++) {
        const auto button = static_cast<SavedColorButton*>(getWidget(i));
        if(button->getColor().rgba() == rgba) {
            removeWidget(button);
            break;
        }
    }
}
