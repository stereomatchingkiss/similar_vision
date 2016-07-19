#ifndef ADVANCE_SETTING_DIALOG_HPP
#define ADVANCE_SETTING_DIALOG_HPP

#include <opencv2/img_hash.hpp>

#include <QDialog>

#include <deque>
#include <vector>

namespace Ui {
class advance_setting_dialog;
}

class QRadioButton;

class advance_setting_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit advance_setting_dialog(QWidget *parent = 0);
    ~advance_setting_dialog();

    cv::Ptr<cv::img_hash::ImgHashBase> get_hash_algo() const;

private:
    void cancel_clicked();
    void create_connection();
    void plot_accuracy_chart(bool val = false);
    void plot_hash_chart();
    void plot_hash_speed_chart();
    void update_hash_origin_state();
    void update_hash_speed_chart_orientation();

    Ui::advance_setting_dialog *ui;

    std::vector<QRadioButton*> hash_buttons_;
    QStringList hash_name_;
    std::vector<bool> hash_origin_state_;
};

#endif // ADVANCE_SETTING_DIALOG_HPP
