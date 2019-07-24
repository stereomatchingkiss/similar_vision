#ifndef PICS_FIND_IMG_HASH_HPP
#define PICS_FIND_IMG_HASH_HPP

#include <QElapsedTimer>
#include <QThread>

#include <opencv2/img_hash.hpp>

#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>

class img_reader;

class pics_find_img_hash : public QThread
{
    Q_OBJECT
public:        
    explicit pics_find_img_hash(
            cv::Ptr<cv::img_hash::ImgHashBase> algo,
            QStringList const &abs_file_path,
            double threshold,
            QObject *parent = nullptr);

    QStringList get_duplicate_img() const;
    QStringList get_original_img() const;

signals:
    void end();
    void progress(QString msg);

private:        
    void compare_hash();

    //Different versions of compute hash functions exist for performance
    //measurement
    void compute_hash();    
    void compute_hash_mt();
    void compute_hash_mt2();

    cv::Mat read_img(QString const &img_path);
    void run() override;

    QStringList abs_file_path_;
    cv::Ptr<cv::img_hash::ImgHashBase> algo_;
    std::condition_variable cv_;
    QStringList duplicate_img_;
    QElapsedTimer elapsed_time_;
    bool finished_;
    std::vector<std::pair<QString, cv::Mat>> hash_arr_;    
    std::mutex mutex_;
    QStringList original_img_;
    int const pool_size_;
    double threshold_;
};

#endif // FIND_SIMILAR_PICS_HPP
