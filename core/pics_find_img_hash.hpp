#ifndef PICS_FIND_IMG_HASH_HPP
#define PICS_FIND_IMG_HASH_HPP

#include <QThread>

#include <opencv2/img_hash.hpp>
//#include <opencv2/flann.hpp>

#include <map>
#include <tuple>

class pics_find_img_hash : public QThread
{
    Q_OBJECT
public:        
    explicit pics_find_img_hash(
            cv::Ptr<cv::img_hash::ImgHashBase> algo,
            QStringList const &abs_file_path,
            QObject *parent = nullptr);

    QStringList get_duplicate_img() const;
    QStringList get_original_img() const;

signals:
    void end();
    void progress(QString msg);

private:    
    //Exist single thread and multi thread versions for performance measurement
    void compare_hash();
    void compute_hash();
    void compute_hash_mt();

    void run() override;

    QStringList abs_file_path_;
    cv::Ptr<cv::img_hash::ImgHashBase> algo_;
    QStringList duplicate_img_;
    std::vector<std::pair<QString, cv::Mat>> hash_arr_;
    QStringList original_img_;
};

#endif // FIND_SIMILAR_PICS_HPP
