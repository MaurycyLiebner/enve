#ifndef BRUSHSAVEDIALOG_H
#define BRUSHSAVEDIALOG_H
#include <QDialog>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include "../windowvariables.h"

class Brush;

class BrushSaveDialog : public QDialog
{
    Q_OBJECT
public:
    BrushSaveDialog(WindowVariables *window_vars_t);

    void saveBrushIcon(QString collection_name, QString brush_name);
private:
    WindowVariables *window_vars = NULL;

    QVBoxLayout *main_layout = NULL;

    //PaintCanvas *canvas = NULL;
    QPushButton *clear_button = NULL;

    QHBoxLayout *collection_layout = NULL;
    QComboBox *collection_combo_box = NULL;
    QPushButton *new_collection_button = NULL;

    QLineEdit *brush_name_line = NULL;

    QPushButton *save_button = NULL;
public slots:
    void clearCanvas();
    void saveBrush();

    void newCollection();
    void addNewCollectionToComboBox(QString collection_name);
};

#endif // BRUSHSAVEDIALOG_H
