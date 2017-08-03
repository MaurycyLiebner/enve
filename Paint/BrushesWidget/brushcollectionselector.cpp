#include "brushcollectionselector.h"
#include <QFontMetrics>

BrushCollectionSelector::BrushCollectionSelector(BrushSelector *brush_selector_t) : QGraphicsItem()
{
    brush_selector = brush_selector_t;

    QFontMetrics fm(button_font);
    setButtonHeight(fm.height() + 10);
}

void BrushCollectionSelector::selectCollection(CollectionButton *current_collection_t)
{
    if(current_collection != NULL)
    {
        current_collection->deselect();
    }
    current_collection = current_collection_t;
    brush_selector->setBrushCollection(current_collection->getCollection());
}

void BrushCollectionSelector::loadCollections(QString from_folder_t)
{
    QDirIterator directories(from_folder_t, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

    bool darker = false;
    while(directories.hasNext())
    {
        CollectionButton *button_t = new CollectionButton(directories.next(), brush_selector, this);
        button_t->setDarker(darker);
        darker = !darker;
        collections.append(button_t);
    }
    updateButtons();
}

int BrushCollectionSelector::getButtonHeight()
{
    return button_height;
}

void BrushCollectionSelector::setButtonHeight(int height_t)
{
    button_height = height_t;
}

QFont BrushCollectionSelector::getButtonFont()
{
    return button_font;
}

void BrushCollectionSelector::setWidth(int width_t)
{
    width = width_t;
    updateButtons();
}

void BrushCollectionSelector::setHeight(int height_t)
{
    height = height_t;
    brush_selector->setY(height_t);
    update();
}

void BrushCollectionSelector::generateButtonRowMargins(QList<int> *row_margins,
                                                       QList<int> *last_button_margins)
{
    int row_width_t = 0;
    int row_n_buttons_t = 0;
    foreach(CollectionButton *button_t, collections)
    {
        int button_width_t = button_t->getTextWidth() + 2*button_margin;
        if(row_width_t + button_width_t > width)
        {
            if(row_n_buttons_t != 0)
            {
                int margin_t = (width - row_width_t)/row_n_buttons_t;
                row_margins->append( margin_t );
                last_button_margins->append( width - (row_n_buttons_t - 1)*margin_t - row_width_t );
            }
            row_n_buttons_t = 0;
            row_width_t = 0;
        }
        row_width_t += button_width_t;
        row_n_buttons_t++;
    }
    if(row_n_buttons_t == 0)
    {
        return;
    }
    int margin_t = (width - row_width_t)/row_n_buttons_t;
    row_margins->append( margin_t );
    last_button_margins->append( width - (row_n_buttons_t - 1)*margin_t - row_width_t );
}

void BrushCollectionSelector::updateButtons()
{
    QList<int> row_margins;
    QList<int> last_button_margins;
    generateButtonRowMargins(&row_margins, &last_button_margins);

    int x_t = 0;
    int y_t = 0;
    int row_id_t = 0;
    CollectionButton *last_button_t = NULL;
    if(!row_margins.isEmpty())
    {
        foreach(CollectionButton *button_t, collections)
        {
            int button_width_t = button_t->getTextWidth() + 2*button_margin;
            if(x_t + button_width_t > width)
            {
                if(last_button_t != NULL)
                {
                    last_button_t->setAdditionalMargin(last_button_margins.at(row_id_t) );
                }
                row_id_t++;
                if(row_id_t >= row_margins.count() )
                {
                    break;
                }
                y_t += button_height;
                x_t = 0;
            }
            button_t->setAdditionalMargin(row_margins.at(row_id_t) );
            button_t->setX(x_t);
            button_t->setY(y_t);
            x_t += button_t->getWidth();

            last_button_t = button_t;
        }
        if(last_button_t != NULL)
        {
            last_button_t->setAdditionalMargin(last_button_margins.last() );
        }
    }
    setHeight(y_t + button_height);
}

void BrushCollectionSelector::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
{

}

QRectF BrushCollectionSelector::boundingRect() const
{
    return QRectF(0, 0, width, height);
}

int BrushCollectionSelector::getButtonMargin()
{
    return button_margin;
}
