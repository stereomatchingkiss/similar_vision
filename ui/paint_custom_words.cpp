#include "paint_custom_words.hpp"

#include <QKeyEvent>
#include <QPainter>

paint_custom_words::paint_custom_words(QWidget *parent) :
    QListView(parent),
    empty_str_("You can drag directories into here")
{

}

const QString &paint_custom_words::get_empty_str() const
{
    return empty_str_;
}

void paint_custom_words::set_emptry_str(const QString &value)
{
    empty_str_ = value;
}

void paint_custom_words::keyPressEvent(QKeyEvent *event)
{
    QListView::keyPressEvent(event);
    emit view_selected();
}

void paint_custom_words::mousePressEvent(QMouseEvent *event)
{
    QListView::mousePressEvent(event);
    if(!indexAt(event->pos()).isValid()){
        clearSelection();
    }
    emit view_selected();
}

void paint_custom_words::paintEvent(QPaintEvent *e)
{
    QListView::paintEvent(e);
    if(model() && model()->rowCount(rootIndex()) > 0) return;

    QPainter p(viewport());
    p.drawText(rect(), Qt::AlignCenter, empty_str_);
}
