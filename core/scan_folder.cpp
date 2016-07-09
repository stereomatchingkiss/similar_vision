#include "scan_folder.hpp"

#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>
#include <QStringList>

#include <opencv2/highgui.hpp>

#include <set>

scan_folder::scan_folder(QStringList const &abs_dirs,
                         bool scan_subfolders,
                         QObject *parent) :
    QThread(parent),
    abs_dirs_(abs_dirs),
    scan_subfolders_(scan_subfolders)
{

}

QStringList scan_folder::get_abs_file_path() const
{
    return  files_;
}

QStringList scan_folder::exclude_child_folders(const QStringList &folders) const
{
    QStringList result = folders;
    result.sort();
    for(int i = 0; i < result.size() - 1; ++i){
        QStringList exclude_fd;
        for(int k = 0; k <= i; ++k){
            exclude_fd.push_back(result[k]);
        }
        for(int j = i; j != result.size(); ++j){
            if(!result[j].startsWith(result[i])){
                exclude_fd.push_back(result[j]);
            }
        }
        result = exclude_fd;
    }

    return result;
}

void scan_folder::scan_folders()
{
    files_.clear();
    if(scan_subfolders_){
        abs_dirs_ = exclude_child_folders(abs_dirs_);
    }

    struct icp
    {
        bool operator()(QString const &lhs, QString const &rhs) const
        {
            return lhs.compare(rhs, Qt::CaseInsensitive) > 0;
        }
    };

    std::set<QString, icp> const is_img
    {"bmp", "dib", "jpeg", "jpg", "jpe",
        "jp2", "png", "webp", "pbm", "pgm", "ppm",
        "sr," "ras", "tiff", "tif"};

    emit progress("Total files found : 0");
    for(auto const &dir : abs_dirs_){
        QDirIterator directories(dir,
                                 QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot,
                                 QDirIterator::Subdirectories);
        while(directories.hasNext()){
            //qDebug()<<directories.fileInfo().suffix();
            if(is_img.find(directories.fileInfo().suffix()) != std::end(is_img)){
                //qDebug()<<"can find suffix";
                files_.push_back(directories.filePath());
                emit progress(QString("Total files found : %1").
                              arg(files_.size()));
            }else{
                //qDebug()<<"cannot find suffix";
            }
            directories.next();
        }
    }
    //qDebug()<<files_;

    emit end();
}

void scan_folder::run()
{    
    scan_folders();
}
