#include "duplicate_img_table_view.hpp"

#include <QKeyEvent>

duplicate_img_table_view::duplicate_img_table_view(QWidget *parent) :
    QTableView(parent)
{

}

void duplicate_img_table_view::keyPressEvent(QKeyEvent *event)
{
    QTableView::keyPressEvent(event);

    if(event->key() == Qt::Key_Up || event->key() == Qt::Key_Down){
        emit key_press(event->key());
    }
}
