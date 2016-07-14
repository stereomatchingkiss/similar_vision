#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class QGraphicsView;
class QTimer;

class duplicate_img_model;
class folder_model;
class pics_find_img_hash;
class scan_folder;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pb_add_folder_clicked();

    void on_pb_find_folder_clicked();

    void on_pb_refresh_clicked();

    void on_pb_delete_folder_clicked();

    void on_pb_up_clicked();

    void on_pb_down_clicked();

    void on_pb_lf_move_clicked();

    void on_pb_rt_move_clicked();

    void on_pb_lf_browse_clicked();

    void on_pb_rt_browse_clicked();

    void on_pb_lf_recycle_clicked();

    void on_pb_rt_recycle_clicked();

private:
    void delete_img(QString const &name);
    void duplicate_img_select(QModelIndex const &index);
    void enable_folder_edit_ui();
    void enable_image_edit_ui();
    void enable_main_ui();
    void enable_up_down_arrow(int item_size, int select_size);
    void find_similar_pics();
    QString get_select_name(int col);
    void move_file(QString const &name);
    void open_folder(QString const &name);
    void resizeEvent(QResizeEvent*) override;
    void scan_folders();        
    void view_duplicate_img(QString const &name,
                            bool img_read,
                            QGraphicsView *view);

    Ui::MainWindow *ui;

    duplicate_img_model *duplicate_img_model_;
    folder_model *folder_model_;
    pics_find_img_hash *pf_img_hash_;
    bool img_lf_changed_;
    bool img_rt_changed_;
    QString pre_img_name_lf_;
    QString pre_img_name_rt_;
    scan_folder *scf_thread_;
    QTimer *timer_;
};

#endif // MAINWINDOW_HPP
