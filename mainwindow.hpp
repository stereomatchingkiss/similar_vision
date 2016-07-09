#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

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

private:
    void enable_folder_edit_ui();
    void enable_image_edit_ui();
    void enable_main_ui();
    void find_similar_pics();
    void scan_folders();

    Ui::MainWindow *ui;

    duplicate_img_model *duplicate_img_model_;
    folder_model *folder_model_;
    pics_find_img_hash *pf_img_hash_;
    scan_folder *scf_thread_;
    QTimer *timer_;
};

#endif // MAINWINDOW_HPP
