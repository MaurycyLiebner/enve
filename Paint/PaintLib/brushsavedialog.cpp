#include "brushsavedialog.h"
#include "brush.h"
#include "../paintcanvas.h"
#include "Colors/helpers.h"

BrushSaveDialog::BrushSaveDialog(WindowVariables *window_vars_t)
{
    window_vars = window_vars_t;
    main_layout = new QVBoxLayout(this);

    //canvas = new PaintCanvas(128, 128, window_vars, this);
    //canvas->loadImage(0, window_vars_t->getBrush()->getBrushFilePath().replace(".plb", ".png") );
    //main_layout->addWidget(canvas);
    clear_button = new QPushButton("Clear", this);
    connect(clear_button, SIGNAL(released()), this, SLOT(clearCanvas()) );
    main_layout->addWidget(clear_button);

    collection_layout = new QHBoxLayout();
    collection_combo_box = new QComboBox(this);
    collection_combo_box->addItem(window_vars_t->getBrush()->getCollectionName());
    collection_layout->addWidget(collection_combo_box);
    new_collection_button = new QPushButton("New Collection", this);
    connect(new_collection_button, SIGNAL(released()), this, SLOT(newCollection()) );
    collection_layout->addWidget(new_collection_button);

    main_layout->addLayout(collection_layout);

    brush_name_line= new QLineEdit(window_vars_t->getBrush()->getBrushName(), this);
    main_layout->addWidget(brush_name_line);

    save_button = new QPushButton("Save", this);
    connect(save_button, SIGNAL(released()), this, SLOT(saveBrush()) );
    main_layout->addWidget(save_button);

    setLayout(main_layout);

    setFixedSize(400, 400);
    show();
}

#include <QInputDialog>
void BrushSaveDialog::newCollection()
{
    bool ok;
    QString text = QInputDialog::getText(NULL, "New Brush Collection",
                                         "Collection name:", QLineEdit::Normal,
                                         "collection", &ok);
    if(ok && !text.isEmpty())
    {
        addNewCollectionToComboBox(text);
    }
}

void BrushSaveDialog::addNewCollectionToComboBox(QString collection_name)
{
    collection_combo_box->addItem(collection_name);
}

void BrushSaveDialog::saveBrushIcon(QString collection_name, QString brush_name) {
    QString file_path = "brushes/" + collection_name + "/" + brush_name + ".png";
    //canvas->saveAsPng(file_path);
}

void BrushSaveDialog::clearCanvas()
{
    canvas->clear();
}
#include <QDir>
void BrushSaveDialog::saveBrush()
{
    QString collection_name = collection_combo_box->currentText();
    QDir dir("brushes/");
    dir.mkdir(collection_name);
    QString brush_name = brush_name_line->text();
    saveBrushDataAsFile(window_vars->current_brush, collection_name, brush_name);
    saveBrushIcon(collection_name, brush_name);
}
