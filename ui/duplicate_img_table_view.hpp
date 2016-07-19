#ifndef DUPLICATE_IMG_TABLE_VIEW_HPP
#define DUPLICATE_IMG_TABLE_VIEW_HPP

#include <QTableView>

class duplicate_img_table_view : public QTableView
{
Q_OBJECT

public:
    explicit duplicate_img_table_view(QWidget *parent = nullptr);

    void keyPressEvent(QKeyEvent *event);

signals:
    void key_press(int key);
};

#endif // DUPLICATE_IMG_TABLE_VIEW_HPP
