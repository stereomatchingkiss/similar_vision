#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include "core/pics_find_img_hash.hpp"
#include "core/scan_folder.hpp"
#include "model/duplicate_img_model.hpp"
#include "model/folder_model.hpp"

#include <QDebug>
#include <QFileDialog>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    duplicate_img_model_(new duplicate_img_model(parent)),
    folder_model_(new folder_model(parent)),
    pf_img_hash_(nullptr),
    scf_thread_(nullptr),
    timer_(new QTimer(this))
{
    ui->setupUi(this);
    ui->list_view_folder->setModel(folder_model_);
    ui->list_view_folder->setAcceptDrops(true);
    ui->list_view_folder->setSelectionMode(QAbstractItemView::ExtendedSelection);

    enable_folder_edit_ui();
    enable_image_edit_ui();
    ui->table_view_similar_pics->setModel(duplicate_img_model_);
    ui->table_view_similar_pics->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->table_view_similar_pics->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(folder_model_, &folder_model::drop_data,
            this, &MainWindow::enable_folder_edit_ui);
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

void MainWindow::on_pb_refresh_clicked()
{
    folder_model_->setStringList({});
    enable_folder_edit_ui();
}

void MainWindow::enable_folder_edit_ui()
{
    bool const enable = !folder_model_->stringList().isEmpty();
    ui->pb_delete_folder->setEnabled(enable);
    ui->pb_find_folder->setEnabled(enable);
    ui->pb_up->setEnabled(enable);
    ui->pb_down->setEnabled(enable);
    ui->pb_refresh->setEnabled(enable);
    ui->action_start_search->setEnabled(enable);
    ui->cb_scan_subdir->setEnabled(enable);
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
        explicit guard_update(QListView *view) :
            view_(view)
        {
            view_->setUpdatesEnabled(false);
        }

        ~guard_update() { view_->setUpdatesEnabled(true); }

        QListView *view_;
    };

    guard_update guard(ui->list_view_folder);
    auto indexes = ui->list_view_folder->selectionModel()->selectedRows();
    std::sort(std::begin(indexes), std::end(indexes));
    for(int i = indexes.count() - 1; i > -1; --i){
        folder_model_->removeRow(indexes.at(i).row());
    }
    enable_folder_edit_ui();
}