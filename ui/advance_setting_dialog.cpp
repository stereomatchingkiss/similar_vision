#include "advance_setting_dialog.hpp"
#include "ui_advance_setting_dialog.h"

#include <QSettings>

#include <functional>

void advance_setting_dialog::plot_hash_chart()
{
    ui->hash_accuracy_chart->set_labels_title(tr("Attack"));
    ui->hash_accuracy_chart->set_values_title(tr("Percentage(%)"));
    plot_accuracy_chart();
}

void advance_setting_dialog::create_connection()
{
    for(size_t i = 0; i != hash_buttons_.size(); ++i){
        connect(hash_buttons_[i], &QRadioButton::clicked, this,
                &advance_setting_dialog::plot_accuracy_chart);

    }
    connect(ui->combox_hash_orien, &QComboBox::currentTextChanged,
            [this](QString const&)
    {
        plot_accuracy_chart();
    });
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this,
            &advance_setting_dialog::update_hash_origin_state);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this,
            &advance_setting_dialog::cancel_clicked);
}

advance_setting_dialog::advance_setting_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::advance_setting_dialog),
    default_hash_threshold_{5, 12, 48, 30, 5},
    default_thresh_name_{"avg_default_thresh", "bmh_0_default_thresh",
                      "bmh_1_default_thresh", "marr_hash_default_thresh",
                      "phash_default_thresh"},
    hash_name_{"Average hash", "Block mean hash 0", "Block mean hash 1",
               "Marr Hildreth hash", "Phash"},
    hash_origin_state_(hash_name_.size())
{
    ui->setupUi(this);

    hash_buttons_.emplace_back(ui->rb_avg_hash);
    hash_buttons_.emplace_back(ui->rb_bm_hash_0);
    hash_buttons_.emplace_back(ui->rb_bm_hash_1);
    hash_buttons_.emplace_back(ui->rb_marr_hash);
    hash_buttons_.emplace_back(ui->rb_phash);

    sliders_.emplace_back(ui->hs_avg_hash);
    sliders_.emplace_back(ui->hs_bmh_0);
    sliders_.emplace_back(ui->hs_bmh_1);
    sliders_.emplace_back(ui->hs_marr_hash);
    sliders_.emplace_back(ui->hs_phash);

    QSettings settings;
    if(settings.contains(hash_name_[0])){
        for(size_t i = 0; i != hash_buttons_.size(); ++i){
            if(settings.value(hash_name_[static_cast<int>(i)]).toBool()){
                hash_buttons_[i]->setChecked(true);
                break;
            }
        }
    }
    if(settings.contains(default_thresh_name_[0])){
        for(size_t i = 0; i != sliders_.size(); ++i){
            sliders_[i]->setValue(settings.value(default_thresh_name_[i]).toInt());
        }
    }
    if(settings.contains("adv_settings_geometry")){
        restoreGeometry(settings.value("adv_settings_geometry").toByteArray());
    }

    update_hash_origin_state();

    ui->combox_hash_orien->addItems({"Vertical", "Horizontal"});
    ui->combox_hash_orien->setCurrentIndex(0);
    ui->combox_hash_orien->setCurrentIndex(0);

    plot_hash_chart();
    create_connection();
}

advance_setting_dialog::~advance_setting_dialog()
{
    QSettings settings;
    for(size_t i = 0; i != hash_buttons_.size(); ++i){
        settings.setValue(hash_name_[static_cast<int>(i)],
                hash_buttons_[static_cast<int>(i)]->isChecked());
    }
    settings.setValue("adv_settings_geometry", saveGeometry());

    for(size_t i = 0; i != sliders_.size(); ++i){
        settings.setValue(default_thresh_name_[i], sliders_[i]->value());
    }

    delete ui;
}

cv::Ptr<cv::img_hash::ImgHashBase> advance_setting_dialog::get_hash_algo() const
{
    using namespace cv::img_hash;
    using creator_func = std::function<cv::Ptr<ImgHashBase>()>;

    std::vector<creator_func> creators
    {
        [](){ return AverageHash::create(); },
        [](){ return BlockMeanHash::create(0); },
        [](){ return BlockMeanHash::create(1); },
        [](){ return MarrHildrethHash::create(); },
        [](){ return PHash::create(); },
    };
    for(size_t i = 0; i != creators.size(); ++i){
        if(hash_buttons_[i]->isChecked()){
            return creators[i]();
        }
    }

    return AverageHash::create();
}

double advance_setting_dialog::get_threshold() const
{
    for(size_t i = 0; i != sliders_.size(); ++i){
        if(hash_buttons_[i]->isChecked()){
            return sliders_[i]->value();
        }
    }

    return 0;
}

void advance_setting_dialog::cancel_clicked()
{
    for(size_t i = 0; i != hash_buttons_.size(); ++i){
        if(hash_origin_state_[i]){
            hash_buttons_[i]->setChecked(true);
            break;
        }
    }

    for(size_t i = 0; i != sliders_.size(); ++i){
        sliders_[i]->setValue(default_hash_threshold_[i]);
    }
}

void advance_setting_dialog::plot_accuracy_chart(bool)
{
    std::vector<QVector<double>> const values
    {
        {95.6522, 100,     97.8261, 99.5652, 84.3478, 1.44928, 6.95652, 100},
        {89.5652, 100,     100,     100,     97.393,  0,       68.6957, 100},
        {92.1739, 100,     100,     100,     97.3913, 0,       72.1739, 99.1304},
        {86.9565, 98.2609, 86.2319, 84.7826, 52.1739, 0,       2.6087,  83.4783},
        {92.1739, 94.7826, 100,     98.6957, 74.7826, 0,       2.6087,  83.4784},
    };
    QStringList const labels{"contrast", "gaussian blur", "gaussian noise",
                             "jpeg compression", "resize", "rotation",
                             "salt pepper noise", "watermark"};
    for(size_t i = 0; i != hash_buttons_.size(); ++i){
        if(hash_buttons_[i]->isChecked()){
            ui->hash_accuracy_chart->set_data(values[i], labels);
            ui->hash_accuracy_chart->set_main_title(hash_name_[static_cast<int>(i)]);
            if(ui->combox_hash_orien->currentIndex() == 0){
                ui->hash_accuracy_chart->set_orientation(Qt::Vertical);
            }else{
                ui->hash_accuracy_chart->set_orientation(Qt::Horizontal);
            }
        }
    }
}

void advance_setting_dialog::update_hash_origin_state()
{
    for(size_t i = 0; i != hash_buttons_.size(); ++i){
        hash_origin_state_[i] = hash_buttons_[i]->isChecked();
    }
}

void advance_setting_dialog::on_pb_avg_hash_default_clicked()
{
    ui->hs_avg_hash->setValue(5);
}

void advance_setting_dialog::on_pb_bmh_0_default_clicked()
{
    ui->hs_bmh_0->setValue(12);
}

void advance_setting_dialog::on_pb_bmh_1_default_clicked()
{
    ui->hs_bmh_1->setValue(48);
}

void advance_setting_dialog::on_pb_marr_hildreth_default_clicked()
{
    ui->hs_marr_hash->setValue(30);
}

void advance_setting_dialog::on_pb_phash_default_clicked()
{
    ui->hs_phash->setValue(5);
}
