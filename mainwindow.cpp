#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include "core/pics_find_img_hash.hpp"
#include "core/scan_folder.hpp"
#include "model/duplicate_img_model.hpp"
#include "model/folder_model.hpp"
#include "ui/advance_setting_dialog.hpp"
#include "ui/basic_setting_dialog.hpp"

#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrentRun>
#include <QElapsedTimer>
#include <QProcess>
#include <QSettings>

#include <opencv2/highgui.hpp>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    duplicate_img_model_(new duplicate_img_model(this)),
    folder_model_(new folder_model(this)),
    img_lf_changed_(false),
    img_rt_changed_(false),
    pf_img_hash_(nullptr),
    scf_thread_(nullptr),
    timer_(new QElapsedTimer)
{
    ui->setupUi(this);
    ui->list_view_folder->setModel(folder_model_);
    ui->list_view_folder->setAcceptDrops(true);
    ui->list_view_folder->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->list_view_folder->setEditTriggers(QAbstractItemView::NoEditTriggers);

    enable_folder_edit_ui();
    enable_image_edit_ui();
    ui->table_view_similar_pics->setModel(duplicate_img_model_);
    ui->table_view_similar_pics->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->table_view_similar_pics->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->gp_view_lf->setScene(new QGraphicsScene(this));
    ui->gp_view_rt->setScene(new QGraphicsScene(this));

    QSettings settings;
    if(settings.contains("main_geometry")){
        restoreGeometry(settings.value("main_geometry").toByteArray());
    }

    connect(folder_model_, &folder_model::drop_data,
            this, &MainWindow::enable_folder_edit_ui);
    connect(ui->list_view_folder, &paint_custom_words::view_selected,
            this, &MainWindow::enable_folder_edit_ui);
    connect(ui->table_view_similar_pics, &QTableView::clicked,
            this, &MainWindow::duplicate_img_select);
    connect(ui->table_view_similar_pics, &duplicate_img_table_view::key_press,
            [this](int)
    {
        auto const index = ui->table_view_similar_pics->currentIndex();
        if(index.isValid()){
            duplicate_img_select(index);
        }
    });
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("main_geometry", saveGeometry());

    delete ui;
}

void MainWindow::on_pb_add_folder_clicked()
{
    QFileDialog dialog(nullptr, tr("Open Directory"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    dialog.setOption(QFileDialog::DontResolveSymlinks);
    if(dialog.exec()){
        auto const dir = dialog.selectedFiles()[0];
        auto str_list = folder_model_->stringList();
        str_list.push_back(dir);
        str_list.removeDuplicates();
        folder_model_->setStringList(str_list);
        enable_folder_edit_ui();
    }

    raise();
}

void MainWindow::scan_folders()
{        
    duplicate_img_model_->clear();
    ui->gp_view_lf->scene()->clear();
    ui->gp_view_rt->scene()->clear();
    timer_->restart();
    scf_thread_ = new scan_folder(folder_model_->stringList(),
                                  basic_setting_dialog().scan_img_type(),
                                  ui->cb_scan_subdir->isChecked(), this);
    connect(scf_thread_, &scan_folder::progress, this, &MainWindow::set_label_info);
    connect(scf_thread_, &scan_folder::finished, this, &MainWindow::find_similar_pics);
    setEnabled(false);
    scf_thread_->start();
}

void MainWindow::set_label_info(QString info)
{
    int const total_sec = timer_->elapsed() / 1000;
    int const min = (total_sec / 60)%60;
    int const hour = total_sec / 3600;
    int const sec = total_sec % 60;
    auto const elapsed = QString("%1:%2:%3").arg(hour, 2, 10, QChar('0')).
            arg(min, 2, 10, QChar('0')).
            arg(sec, 2, 10, QChar('0'));
    ui->label_info->setText(QString("%1, Elapsed time : %2").arg(info).
                            arg(elapsed));
}

void MainWindow::on_pb_find_folder_clicked()
{    
    scan_folders();
}

void MainWindow::find_similar_pics()
{
    using namespace cv::img_hash;

    pf_img_hash_ = new pics_find_img_hash(advance_setting_dialog().get_hash_algo(),
                                          scf_thread_->get_abs_file_path(),
                                          advance_setting_dialog().get_threshold(),
                                          this);
    scf_thread_->deleteLater();

    connect(pf_img_hash_, &pics_find_img_hash::progress, this, &MainWindow::set_label_info);
    connect(pf_img_hash_, &pics_find_img_hash::finished, this, &MainWindow::find_similar_pics_end);

    pf_img_hash_->start();
}

void MainWindow::find_similar_pics_end()
{
    pf_img_hash_->deleteLater();
    enable_main_ui();
    QMessageBox::information(this, tr("Information"),
                             tr("Search finished.\n\n"
                                "Similar images pair found : %1\n").
                             arg(duplicate_img_model_->rowCount()));
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    duplicate_img_select(ui->table_view_similar_pics->currentIndex());
}

void MainWindow::on_pb_refresh_clicked()
{
    folder_model_->setStringList({});
    enable_folder_edit_ui();
}

void MainWindow::enable_up_down_arrow(int item_size, int select_size)
{
    if(item_size > 1){
        if(select_size == 1){
            auto const index = ui->list_view_folder->currentIndex();
            if(index.isValid()){
                bool const not_at_bottom =
                        index.row() != folder_model_->rowCount() - 1;
                bool const not_at_top = index.row() != 0;
                ui->pb_up->setEnabled(not_at_top);
                ui->pb_down->setEnabled(not_at_bottom);
            }else{
                ui->pb_up->setEnabled(false);
                ui->pb_down->setEnabled(false);
            }
        }else{
            ui->pb_up->setEnabled(false);
            ui->pb_down->setEnabled(false);
        }
    }else{
        ui->pb_up->setEnabled(false);
        ui->pb_down->setEnabled(false);
    }
}

void MainWindow::enable_folder_edit_ui()
{
    auto const &str_list = folder_model_->stringList();
    bool const has_item = !str_list.isEmpty();
    auto const &selected_rows = ui->list_view_folder->selectionModel()->selectedRows();
    bool const item_selected = !selected_rows.isEmpty();

    ui->pb_delete_folder->setEnabled(has_item && item_selected);
    ui->pb_find_folder->setEnabled(has_item);
    ui->pb_refresh->setEnabled(has_item);
    ui->action_start_search->setEnabled(has_item);
    ui->cb_scan_subdir->setEnabled(has_item);

    enable_up_down_arrow(str_list.size(), selected_rows.size());
}

void MainWindow::enable_image_edit_ui()
{
    bool const enable = duplicate_img_model_->rowCount() != 0;
    ui->table_view_similar_pics->setEnabled(enable);
    ui->gb_left_pic->setEnabled(enable);
    ui->gb_right_pic->setEnabled(enable);
}

void MainWindow::enable_main_ui()
{
    duplicate_img_model_->set_img_set(pf_img_hash_->get_original_img(),
                                      pf_img_hash_->get_duplicate_img());
    ui->table_view_similar_pics->reset();
    ui->table_view_similar_pics->resizeColumnsToContents();
    setEnabled(true);
    enable_image_edit_ui();
}

void MainWindow::on_pb_delete_folder_clicked()
{
    struct guard_update
    {
        explicit guard_update(paint_custom_words *view, MainWindow *win) :
            view_(view),
            win_(win)
        {
            view_->setUpdatesEnabled(false);
            win_->disconnect(view_, &paint_custom_words::view_selected,
                             win_, &MainWindow::enable_folder_edit_ui);
        }

        ~guard_update()
        {
            view_->setUpdatesEnabled(true);
            win_->connect(view_, &paint_custom_words::view_selected,
                          win_,  &MainWindow::enable_folder_edit_ui);
        }

        paint_custom_words *view_;
        MainWindow *win_;
    };

    guard_update guard(ui->list_view_folder, this);
    auto indexes = ui->list_view_folder->selectionModel()->selectedRows();
    int offset = 0;
    for(auto const &var : indexes){
        int const Index = var.row() - offset;
        folder_model_->removeRow(Index, {});
        ++offset;
    }
    enable_folder_edit_ui();
}

void MainWindow::duplicate_img_select(QModelIndex const &index)
{
    if(index.isValid()){
        auto const img_name_lf =
                duplicate_img_model_->data(duplicate_img_model_->index(index.row(), 0),
                                           Qt::DisplayRole).toString();
        auto const img_name_rt =
                duplicate_img_model_->data(duplicate_img_model_->index(index.row(), 1),
                                           Qt::DisplayRole).toString();
        img_lf_changed_ = pre_img_name_lf_ != img_name_lf ? true : false;
        img_rt_changed_ = pre_img_name_rt_ != img_name_rt ? true : false;
        pre_img_name_lf_ = img_name_lf;
        pre_img_name_rt_ = img_name_rt;
        bool can_view = view_duplicate_img(pre_img_name_lf_,
                                           img_lf_changed_,
                                           ui->gp_view_lf);
        if(!can_view){
            QMessageBox::warning(this, tr("Error"), tr("Cannot open image %1").arg(img_name_lf));
            return;
        }
        can_view = view_duplicate_img(pre_img_name_rt_,
                                      img_rt_changed_,
                                      ui->gp_view_rt);
        if(!can_view){
            QMessageBox::warning(this, tr("Error"), tr("Cannot open image %1").arg(img_name_rt));
        }
    }
}

bool MainWindow::view_duplicate_img(const QString &name,
                                    bool img_changed,
                                    QGraphicsView *view)
{
    if(img_changed){
        QImage img(name);
        if(!img.isNull()){
            auto const suffix = QFileInfo(name).suffix();
            auto const img_info =
                    QString("%1 %2x%3 %4KB").arg(suffix).
                    arg(img.width()).arg(img.height()).
                    arg(qRound(QFile(name).size()/1024.0));
            if(view == ui->gp_view_lf){
                ui->lb_left_pic->setText(img_info);
            }else{
                ui->lb_right_pic->setText(img_info);
            }
            view->scene()->clear();
            view->scene()->addPixmap(QPixmap::fromImage(img));
            view->fitInView(view->scene()->itemsBoundingRect(),
                            Qt::KeepAspectRatio);
        }else{
            return false;
        }
    }else{
        view->fitInView(view->scene()->itemsBoundingRect(),
                        Qt::KeepAspectRatio);
    }

    return true;
}

void MainWindow::on_pb_up_clicked()
{
    auto const index = ui->list_view_folder->currentIndex();
    auto const new_index = folder_model_->index(index.row()-1);
    ui->list_view_folder->setCurrentIndex(new_index);
    enable_folder_edit_ui();
}

void MainWindow::on_pb_down_clicked()
{
    auto const index = ui->list_view_folder->currentIndex();
    auto const new_index = folder_model_->index(index.row()+1);
    ui->list_view_folder->setCurrentIndex(new_index);
    enable_folder_edit_ui();
}

QString MainWindow::get_select_name(int col)
{
    auto const index = ui->table_view_similar_pics->currentIndex();
    if(index.isValid()){
        auto const new_index = duplicate_img_model_->index(index.row(),col);
        return duplicate_img_model_->data(new_index, Qt::DisplayRole).toString();
    }else{
        return "";
    }
}

void MainWindow::move_file(QString const &name)
{
    QString const dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                          "",
                                                          QFileDialog::ShowDirsOnly
                                                          | QFileDialog::DontResolveSymlinks);
    if(QFile::rename(name, dir + "/" +
                     QFileInfo(name).fileName())){
        remove_img_from_table(name);
    }else{
        QMessageBox::warning(this, tr("Error"),
                             tr("Cannot move the file, it may not exist anymore "
                                "or accupied by other program"));
    }
}

void MainWindow::on_pb_lf_move_clicked()
{            
    move_file(get_select_name(0));
}

void MainWindow::on_pb_rt_move_clicked()
{
    move_file(get_select_name(1));
}

void MainWindow::open_folder(QString const &name)
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFiles);
    auto const dir_path = QFileInfo(name).absoluteDir();
    if(QFile::exists(dir_path.currentPath())){
        dialog.setDirectory(dir_path);
        dialog.exec();
    }else{
        QMessageBox::warning(this, tr("Error"),
                             tr("Cannot open dialog do not exist"));
    }
}

void MainWindow::on_pb_lf_browse_clicked()
{
    open_folder(get_select_name(0));
}

void MainWindow::on_pb_rt_browse_clicked()
{
    open_folder(get_select_name(1));
}

void MainWindow::remove_img_from_table(QString const &name)
{
    int const cur_index = ui->table_view_similar_pics->currentIndex().row();
    duplicate_img_model_->remove_img(name);
    int const row_count = duplicate_img_model_->rowCount();
    if(row_count > 1){
        auto index = duplicate_img_model_->index(cur_index + 1, 0);
        if(index.isValid()){
            ui->table_view_similar_pics->setCurrentIndex(index);
            duplicate_img_select(index);
        }else{
            index = duplicate_img_model_->index(row_count-1, 0);
            ui->table_view_similar_pics->setCurrentIndex(index);
            duplicate_img_select(index);
        }
    }else if(row_count == 1){
        auto const index = duplicate_img_model_->index(0, 0);
        ui->table_view_similar_pics->setCurrentIndex(index);
        duplicate_img_select(index);
    }else{
        ui->gp_view_lf->scene()->clear();
        ui->gp_view_rt->scene()->clear();
        ui->lb_left_pic->clear();
        ui->lb_right_pic->clear();
    }
}

void MainWindow::delete_img(QString const &name)
{
    if(QFile::remove(name)){
        remove_img_from_table(name);
    }else{
        QMessageBox::warning(this, tr("Error"),
                             tr("Cannot remove img, it may "
                                "not exist or occupied by other "
                                "program"));
    }
}

void MainWindow::on_pb_lf_recycle_clicked()
{
    delete_img(get_select_name(0));
}

void MainWindow::on_pb_rt_recycle_clicked()
{
    delete_img(get_select_name(1));
}

void MainWindow::on_action_basic_setting_triggered()
{    
    basic_setting_dialog().exec();
}

void MainWindow::on_action_exit_triggered()
{
    close();
}

void MainWindow::on_action_qt_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_action_fatcow_triggered()
{
    QMessageBox::information(this, tr("Source"),
                             tr("The icons are come from http://www.fatcow."
                                "com/free-icons"));
}

void MainWindow::on_action_advance_setting_triggered()
{
    advance_setting_dialog().exec();
}

void MainWindow::on_action_visit_program_website_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/"
                                   "stereomatchingkiss/similar_vision"));
}

void MainWindow::on_action_check_for_update_triggered()
{
    qDebug()<<"start update";

    setEnabled(false);
    if(can_update()){
        QString const app_dir_path = QCoreApplication::applicationDirPath();
        qDebug()<<app_dir_path + "/auto_updater/auto_updater";

        bool const can_open = QProcess::startDetached(app_dir_path + "/auto_updater/auto_updater",
                                                      QStringList()<<"-a"<<(app_dir_path + "/similar_vision"),
                                                      app_dir_path + "/auto_updater");
        if(can_open){
            close();
        }else{
            QMessageBox::warning(this, tr("Update fail"),
                                 tr("Cannot open auto_updater, please try again or "
                                    "execute the app(%1) by yourself after you "
                                    "close this app").arg(app_dir_path));
        }
    }else{
        QMessageBox::information(this, tr("Update info"),
                                 tr("Nothing to update"));
    }
    setEnabled(true);
}

bool MainWindow::can_update() const
{
    QString const app_dir_path = QCoreApplication::applicationDirPath();
    QProcess process;
    struct kill_process
    {
        kill_process(QProcess *p) : process_(p) {}
        ~kill_process() { process_->kill(); }

    private:
        QProcess *process_;
    };
    kill_process kp(&process);

    process.start(app_dir_path + "/auto_updater/auto_updater",
                  QStringList()<<"-n",
                  QIODevice::ReadOnly);
    if(process.waitForFinished(-1)){
        QString const process_output(process.readAll());

        qDebug()<<"process output : "<<process_output;
        if(process_output.contains("need to update")){
            return true;
        }
    }

    return false;
}
