#include "advance_setting_dialog.hpp"
#include "ui_advance_setting_dialog.h"

#include <QSettings>

#include <functional>

advance_setting_dialog::advance_setting_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::advance_setting_dialog),
    hash_name_{"Average hash", "Block mean hash 0", "Block mean hash 1",
               "Color moment hash", "Marr Hildreth hash", "Phash",
               "Radial variance hash"},
    hash_origin_state_(hash_name_.size())
{
    ui->setupUi(this);

    hash_buttons_.emplace_back(ui->rb_avg_hash);
    hash_buttons_.emplace_back(ui->rb_bm_hash_0);
    hash_buttons_.emplace_back(ui->rb_bm_hash_1);
    hash_buttons_.emplace_back(ui->rb_color_moment_hash);
    hash_buttons_.emplace_back(ui->rb_marr_hash);
    hash_buttons_.emplace_back(ui->rb_phash);
    hash_buttons_.emplace_back(ui->rb_radial_hash);

    QSettings settings(tr("freedom"), tr("similar_vision"));
    if(settings.contains(hash_name_[0])){
        for(size_t i = 0; i != hash_buttons_.size(); ++i){
            if(settings.value(hash_name_[i]).toBool()){
                hash_buttons_[i]->setChecked(true);
                break;
            }
        }
    }

    update_hash_origin_state();

    ui->combox_hash_orien->addItems({"Vertical", "Horizontal"});
    ui->combox_hash_orien->setCurrentIndex(0);

    plot_accuracy_chart();

    ui->accuracy_chart->set_labels_title(tr("Attack"));
    ui->accuracy_chart->set_values_title(tr("Percentage(%)"));

    for(size_t i = 0; i != hash_buttons_.size(); ++i){
        connect(hash_buttons_[i], &QRadioButton::clicked, this,
                &advance_setting_dialog::plot_accuracy_chart);

    }
    connect(ui->combox_hash_orien, &QComboBox::currentTextChanged,
            this, [this](QString const&)
    {
       plot_accuracy_chart();
    });
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this,
            &advance_setting_dialog::update_hash_origin_state);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this,
            &advance_setting_dialog::cancel_clicked);
}

advance_setting_dialog::~advance_setting_dialog()
{
    QSettings settings(tr("freedom"), tr("similar_vision"));
    for(size_t i = 0; i != hash_buttons_.size(); ++i){
        settings.setValue(hash_name_[i], hash_buttons_[i]->isChecked());
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
        [](){ return ColorMomentHash::create(); },
        [](){ return MarrHildrethHash::create(); },
        [](){ return PHash::create(); },
        [](){ return RadialVarianceHash::create(); }
    };
    for(size_t i = 0; i != creators.size(); ++i){
        if(hash_buttons_[i]->isChecked()){
            return creators[i]();
        }
    }

    return AverageHash::create();
}

void advance_setting_dialog::cancel_clicked()
{
    for(size_t i = 0; i != hash_buttons_.size(); ++i){
        if(hash_origin_state_[i]){
            hash_buttons_[i]->setChecked(true);
            break;
        }
    }
}

void advance_setting_dialog::plot_accuracy_chart(bool)
{
    std::vector<QVector<double>> const values
    {
        {95.6522, 100,     97.8261, 99.5652, 84.3478, 1.44928, 6.95652, 100},
        {89.5652, 100,     100,     100,     97.393,  0,       68.6957, 100},
        {92.1739, 100,     100,     100,     97.3913, 0,       72.1739, 99.1304},
        {64.3478, 100,     42.029,  95.2174, 100,     97.1014, 30.4348, 91.3043},
        {86.9565, 98.2609, 86.2319, 84.7826, 52.1739, 0,       2.6087,  83.4783},
        {92.1739, 94.7826, 100,     98.6957, 74.7826, 0,       2.6087,  83.4784},
        {85.2174, 100,     100,     100,     13.0435, 0,       54.7826, 97.3913},
    };
    QStringList const labels{"contrast", "gaussian blur", "gaussian noise",
                             "jpeg compression", "resize", "rotation",
                             "salt pepper noise", "watermark"};
    for(size_t i = 0; i != hash_buttons_.size(); ++i){
        if(hash_buttons_[i]->isChecked()){
            ui->accuracy_chart->set_data(values[i], labels);
            ui->accuracy_chart->set_main_title(hash_name_[i]);
            if(ui->combox_hash_orien->currentText() == "Vertical"){
                ui->accuracy_chart->set_orientation(Qt::Vertical);
            }else{
                ui->accuracy_chart->set_orientation(Qt::Horizontal);
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
