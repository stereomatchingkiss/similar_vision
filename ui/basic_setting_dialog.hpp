#ifndef BASIC_SETTING_DIALOG_HPP
#define BASIC_SETTING_DIALOG_HPP

#include <QDialog>

#include <utility>
#include <vector>

namespace Ui {
class basic_setting_dialog;
}

class QAbstractButton;
class QCheckBox;

class basic_setting_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit basic_setting_dialog(QWidget *parent = 0);
    ~basic_setting_dialog();

    bool auto_update() const;
    bool perfect_match() const;
    QStringList scan_img_type() const;

private:
    void cancel_clicked();
    void ok_clicked();

    Ui::basic_setting_dialog *ui;

    std::vector<std::pair<QString, QCheckBox*>> check_box_settings_;
    std::vector<std::pair<bool, QCheckBox*>> origin_state_;
};

#endif // BASIC_SETTING_DIALOG_HPP
