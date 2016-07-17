#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include "core/pics_find_img_hash.hpp"
#include "core/scan_folder.hpp"
#include "model/duplicate_img_model.hpp"
#include "model/folder_model.hpp"
#include "ui/basic_setting_dialog.hpp"

#include <QDebug>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrentRun>
#include <QTimer>

#include <opencv2/highgui.hpp>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    basic_settings_(new basic_setting_dialog(this)),
    duplicate_img_model_(new duplicate_img_model(this)),
    folder_model_(new folder_model(this)),
    img_lf_changed_(false),
    img_rt_changed_(false),
    pf_img_hash_(nullptr),
    scf_thread_(nullptr),    
    timer_(new QTimer(this))
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

    connect(folder_model_, &folder_model::drop_data,
            this, &MainWindow::enable_folder_edit_ui);
    connect(ui->list_view_folder, &paint_custom_words::view_selected,
            this, &MainWindow::enable_folder_edit_ui);
    connect(ui->table_view_similar_pics, &QTableView::clicked,
            this, &MainWindow::duplicate_img_select);
}

MainWindow::~MainWindow()
{
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
    scf_thread_ = new scan_folder(folder_model_->stringList(),
                                  ui->cb_scan_subdir->isChecked(), this);
    connect(scf_thread_, &scan_folder::progress, ui->label_info, &QLabel::setText);
    connect(scf_thread_, &scan_folder::end, this, &MainWindow::find_similar_pics);
    connect(scf_thread_, &scan_folder::finished, scf_thread_, &QObject::deleteLater);
    setEnabled(false);
    scf_thread_->start();
}

void MainWindow::on_pb_find_folder_clicked()
{    
    scan_folders();
}

void MainWindow::find_similar_pics()
{
    using namespace cv::img_hash;

    pf_img_hash_ = new pics_find_img_hash(AverageHash::create(),
                                          scf_thread_->get_abs_file_path(),
                                          this);

    connect(pf_img_hash_, &pics_find_img_hash::progress, ui->label_info, &QLabel::setText);
    connect(pf_img_hash_, &pics_find_img_hash::end, this, &MainWindow::enable_main_ui);
    connect(pf_img_hash_, &pics_find_img_hash::finished, pf_img_hash_, &QObject::deleteLater);

    pf_img_hash_->start();
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
    view_duplicate_img(pre_img_name_lf_,
                       img_lf_changed_,
                       ui->gp_view_lf);
    view_duplicate_img(pre_img_name_rt_,
                       img_rt_changed_,
                       ui->gp_view_rt);
}

void MainWindow::view_duplicate_img(const QString &name,
                                    bool img_changed,
                                    QGraphicsView *view)
{
    if(img_changed){
        QImage img(name);
        if(!img.isNull()){
            view->scene()->clear();
            view->scene()->addPixmap(QPixmap::fromImage(img));
            view->fitInView(view->scene()->itemsBoundingRect(),
                            Qt::KeepAspectRatio);
        }
    }else{
        view->fitInView(view->scene()->itemsBoundingRect(),
                        Qt::KeepAspectRatio);
    }
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
    basic_settings_->exec();
}
