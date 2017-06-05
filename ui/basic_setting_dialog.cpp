#include "basic_setting_dialog.hpp"
#include "ui_basic_setting_dialog.h"

#include <QDebug>
#include <QSettings>
#include <QVariant>

basic_setting_dialog::basic_setting_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::basic_setting_dialog)
{
    ui->setupUi(this);

    ui->group_box_search_opt->hide();

    check_box_settings_.emplace_back("img_type/bmp_checked", ui->cb_bmp);
    check_box_settings_.emplace_back("img_type/jpeg_checked", ui->cb_jpeg);
    check_box_settings_.emplace_back("img_type/pbm_checked", ui->cb_pbm);
    check_box_settings_.emplace_back("img_type/pgm_checked", ui->cb_pgm);
    check_box_settings_.emplace_back("img_type/png_checked", ui->cb_png);
    check_box_settings_.emplace_back("img_type/ppm_checked", ui->cb_ppm);
    check_box_settings_.emplace_back("img_type/tiff_checked", ui->cb_tiff);
    check_box_settings_.emplace_back("img_type/webp_checked", ui->cb_webp);
    check_box_settings_.emplace_back("Search/perfect_match", ui->cb_perfect_match);
    check_box_settings_.emplace_back("App/auto_update", ui->cb_auto_update);

    QSettings const settings;
    for(auto &pair : check_box_settings_){
        if(settings.contains(pair.first)){
            pair.second->setChecked(settings.value(pair.first).toBool());
        }
    }
    if(settings.contains("basic_setting_geometry")){
        restoreGeometry(settings.value("basic_setting_geometry").toByteArray());
    }

    origin_state_.emplace_back(ui->cb_bmp->isChecked(), ui->cb_bmp);
    origin_state_.emplace_back(ui->cb_jpeg->isChecked(), ui->cb_jpeg);
    origin_state_.emplace_back(ui->cb_pbm->isChecked(), ui->cb_pbm);
    origin_state_.emplace_back(ui->cb_pgm->isChecked(), ui->cb_pgm);
    origin_state_.emplace_back(ui->cb_png->isChecked(), ui->cb_png);
    origin_state_.emplace_back(ui->cb_ppm->isChecked(), ui->cb_ppm);
    origin_state_.emplace_back(ui->cb_tiff->isChecked(), ui->cb_tiff);
    origin_state_.emplace_back(ui->cb_webp->isChecked(), ui->cb_webp);
    origin_state_.emplace_back(ui->cb_perfect_match->isChecked(), ui->cb_perfect_match);
    origin_state_.emplace_back(ui->cb_auto_update->isChecked(), ui->cb_auto_update);    
}

basic_setting_dialog::~basic_setting_dialog()
{    
    delete ui;
}

bool basic_setting_dialog::auto_update() const
{
    return ui->cb_auto_update->isChecked();
}

bool basic_setting_dialog::perfect_match() const
{
    return ui->cb_perfect_match->isChecked();
}

QStringList basic_setting_dialog::scan_img_type() const
{
    QStringList types;
    std::vector<std::pair<QString, QCheckBox*>> const mapper
    {
        {"bmp", ui->cb_bmp}, {"jpeg", ui->cb_jpeg},
        {"jpg", ui->cb_jpeg},{"pbm", ui->cb_pbm},
        {"pgm", ui->cb_pgm}, {"png", ui->cb_png},
        {"ppm", ui->cb_ppm}, {"tiff", ui->cb_tiff},
        {"webp", ui->cb_webp}
    };
    for(auto const &pair : mapper){
        if(pair.second->isChecked()){
            types.push_back(pair.first);            
        }
    }

    return types;
}

void basic_setting_dialog::closeEvent(QCloseEvent *event)
{
    if(event){
        QSettings settings;
        for(auto &pair : check_box_settings_){
            settings.setValue(pair.first, pair.second->isChecked());
        }
        settings.setValue("version", 1.0);
        settings.setValue("basic_setting_geometry", saveGeometry());
        QDialog::closeEvent(event);
    }
}

void basic_setting_dialog::on_buttonBox_accepted()
{
    for(auto &pair : origin_state_){
        pair.first = pair.second->isChecked();
    }
    close();
}

void basic_setting_dialog::on_buttonBox_rejected()
{
    for(auto &pair : origin_state_){
        pair.second->setChecked(pair.first);
    }
    close();
}
