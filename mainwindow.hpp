#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

#include <memory>

namespace Ui {
class MainWindow;
}

class QGraphicsView;
class QElapsedTimer;

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

    void on_action_advance_setting_triggered();

    void on_action_basic_setting_triggered();

    void on_action_exit_triggered();

    void on_action_qt_triggered();

    void on_action_fatcow_triggered();    

    void on_action_visit_program_website_triggered();

    void on_action_check_for_update_triggered();

private:
    bool can_update() const;
    void delete_img(QString const &name);
    void duplicate_img_select(QModelIndex const &index);
    void enable_folder_edit_ui();
    void enable_image_edit_ui();
    void enable_main_ui();
    void enable_up_down_arrow(int item_size, int select_size);
    void find_similar_pics();
    void find_similar_pics_end();
    QString get_select_name(int col);
    void move_file(QString const &name);
    void open_folder(QString const &name);
    void remove_img_from_table(QString const &name);
    void resizeEvent(QResizeEvent*) override;
    void scan_folders();
    void set_label_info(QString info);
    bool view_duplicate_img(QString const &name,
                            bool img_read,
                            QGraphicsView *view);

    Ui::MainWindow *ui;

    duplicate_img_model *duplicate_img_model_;
    folder_model *folder_model_;    
    bool img_lf_changed_;
    bool img_rt_changed_;
    pics_find_img_hash *pf_img_hash_;
    QString pre_img_name_lf_;
    QString pre_img_name_rt_;
    scan_folder *scf_thread_;    
    std::unique_ptr<QElapsedTimer> timer_;
};

#endif // MAINWINDOW_HPP
