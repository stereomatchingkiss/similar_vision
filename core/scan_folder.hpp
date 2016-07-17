#ifndef SCAN_FOLDER_HPP
#define SCAN_FOLDER_HPP

#include <QThread>

#include <opencv2/core.hpp>

class scan_folder : public QThread
{
    Q_OBJECT
public:
    explicit scan_folder(QStringList const &abs_dirs,
                         QStringList const &scan_types,
                         bool scan_subfolders = true,
                         QObject *parent = nullptr);

    QStringList get_abs_file_path() const;

signals:
    void end();
    void progress(QString msg);

private:
    QStringList exclude_child_folders(QStringList const &folders) const;

    void run() override;

    void scan_folders();

    QStringList abs_dirs_;
    QStringList files_;
    bool scan_subfolders_;
    QStringList scan_types_;
};

#endif // SCAN_FOLDER_HPP
