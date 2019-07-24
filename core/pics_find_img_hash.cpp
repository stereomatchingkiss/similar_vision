#include "pics_find_img_hash.hpp"

#include "vp_tree.hpp"

#include <QtConcurrent/QtConcurrentMap>
#include <QtConcurrent/QtConcurrentRun>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QProcess>
#include <QUuid>

#include <opencv2/core/ocl.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>

#include <iostream>

namespace{

using graph_type = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS>;
using edges_type = graph_type::edge_descriptor;

class dfs_visitor : public boost::default_dfs_visitor
{
public:
    explicit dfs_visitor(std::vector<edges_type> &edges) :
        edges_(edges) {}

    template <class Edge, class Graph>
    void tree_edge(Edge u, const Graph&)
    {
        edges_.emplace_back(u);
    }

    std::vector<edges_type> &edges_;
};

struct dist_compare
{
    using value_type = std::pair<QString, cv::Mat>;
    using algo_type = cv::Ptr<cv::img_hash::ImgHashBase>;

    explicit dist_compare(algo_type algo) :
        algo_(algo){}

    double operator()(value_type const &lhs,
                      value_type const &rhs) const
    {
        return algo_->compare(lhs.second, rhs.second);
    }

private:
    algo_type algo_;
};

template<typename Tree, typename Condition>
graph_type create_graph(Tree const &tree,
                        int sample_size, int actual_size,
                        Condition condition)
{
    using value_type = dist_compare::value_type;

    std::map<QString, size_t> name_id;
    auto const &items = tree.get_items();
    for(size_t i = 0; i != items.size(); ++i){
        name_id.insert({items[i].first, i});
    }
    sample_size = actual_size > sample_size ? sample_size : actual_size;
    std::vector<value_type> result;
    std::vector<double> distance;
    graph_type graph;
    for(size_t i = 0; i != items.size(); ++i){
        tree.search(items[i], sample_size, result, distance,
                    condition);
        for(auto const &pair : result){
            auto it = name_id.find(pair.first);
            if(it != std::end(name_id)){
                boost::add_edge(i, it->second, graph);
            }
        }
    }

    return graph;
}

template<typename Graph>
std::vector<edges_type> create_edges(Graph const &graph)
{
    std::vector<edges_type> edges;
    dfs_visitor vis(edges);
    boost::depth_first_search(graph, boost::visitor(vis));

    return edges;
}

cv::Mat qimage_to_cvmat(QImage &img)
{
    qDebug()<<__func__<<"use plugin to read image";
    img = img.convertToFormat(QImage::Format_RGB888);
    auto mat = cv::Mat(img.height(), img.width(), CV_8UC3,
                       img.bits(), static_cast<size_t>(img.bytesPerLine()));
    cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
    return mat.clone();
}

QString const temp_folder("thumbnails");

std::pair<cv::Mat, QString> exiftool_can_read(QImage &img, QString const &img_path)
{
    qDebug()<<__func__<<"exiftool can read thumbnail";
    auto const temp_path = temp_folder + "/" + QUuid::createUuid().toString() + ".jpg";
    if(img.save(temp_path)){
        qDebug()<<__func__<<"can save temp image";
        return {qimage_to_cvmat(img), temp_path};
    }else{
        return {qimage_to_cvmat(img), img_path};
    }
}

}//nameless namespace

pics_find_img_hash::
pics_find_img_hash(cv::Ptr<cv::img_hash::ImgHashBase> algo,
                   QStringList const &abs_file_path,
                   double threshold,
                   QObject *parent) :
    QThread(parent),
    abs_file_path_(abs_file_path),
    algo_(algo),
    finished_(false),
    pool_size_(QThread::idealThreadCount() > 2 ?
                   QThread::idealThreadCount() - 2 : 1),
    threshold_(threshold)
{
    cv::ocl::setUseOpenCL(false);
}

QStringList pics_find_img_hash::get_duplicate_img() const
{
    return duplicate_img_;
}

QStringList pics_find_img_hash::get_original_img() const
{
    return original_img_;
}

void pics_find_img_hash::compare_hash()
{
    using namespace cv::img_hash;
    using value_type = dist_compare::value_type;

    dist_compare dc(algo_);
    vp_tree<value_type, dist_compare> hamming_tree(std::move(dc));
    hamming_tree.create(std::move(hash_arr_));

    graph_type graph;
    graph = create_graph(hamming_tree,
                         abs_file_path_.size(), 10,
                         [=](double val){ return val <= threshold_; });
    auto const edges = create_edges(graph);
    auto const &items = hamming_tree.get_items();
    original_img_.clear();
    duplicate_img_.clear();
    for(size_t i = 0; i != edges.size(); ++i){
        original_img_.push_back(items[edges[i].m_source].first);
        duplicate_img_.push_back(items[edges[i].m_target].first);
    }
}

void pics_find_img_hash::compute_hash()
{
    hash_arr_.clear();
    hash_arr_.reserve(static_cast<size_t>(abs_file_path_.size()));
    for(auto &name : abs_file_path_){
        auto const result = read_img(name);
        if(!result.first.empty()){
            cv::Mat hash;
            name = result.second;
            algo_->compute(result.first, hash);
            hash_arr_.emplace_back(name, hash);
        }
        emit progress("Image preprocess st : " +
                      QString("%1").arg(hash_arr_.size()));
    }
}

void pics_find_img_hash::compute_hash_mt()
{    
    hash_arr_.clear();
    hash_arr_.reserve(static_cast<size_t>(abs_file_path_.size()));
    std::mutex mutex;
    std::atomic<size_t> size(0);
    auto compute_hash = [&](QString &name)
    {
        auto const result = read_img(name);
        if(!result.first.empty()){
            name = result.second;
            ++size;
            cv::Mat hash;
            std::lock_guard<std::mutex> guard(mutex);
            algo_->compute(result.first, hash);
            hash_arr_.emplace_back(name, hash);
        }
        emit progress("Image preprocess mt : " +
                      QString("%1").arg(size.load()));
    };
    QtConcurrent::blockingMap(abs_file_path_, compute_hash);//*/
}

void pics_find_img_hash::compute_hash_mt2()
{
    hash_arr_.clear();
    hash_arr_.reserve(static_cast<size_t>(abs_file_path_.size()));
    int process_size = 0;
    auto process_func = [&](cv::Mat const &img, int i)
    {
        cv::Mat hash;
        int size = 0;
        {
            std::lock_guard<std::mutex> lk(mutex_);
            algo_->compute(img, hash);
            hash_arr_.emplace_back(abs_file_path_[i], hash);
            size = process_size + 1;
            ++process_size;
        }
        emit progress("Image preprocess : " +
                      QString("%1/%2").arg(size).
                      arg(abs_file_path_.size()));
        if(size == abs_file_path_.size()){
            finished_ = true;
            cv_.notify_one();
        }
    };

    for(int i = 0; i != abs_file_path_.size(); ++i){
        auto const result = read_img(abs_file_path_[i]);
        if(!result.first.empty()){
            abs_file_path_[i] = result.second;
            QtConcurrent::run(QThreadPool::globalInstance(), process_func, result.first, i);
        }
    }

    std::unique_lock<std::mutex> lk(mutex_);
    cv_.wait(lk, [this](){return finished_;});
}

std::pair<cv::Mat, QString> pics_find_img_hash::read_img(const QString &img_path)
{
    auto const suffix = QFileInfo(img_path).suffix();
    if(suffix.toLower() == "psb" || suffix.toLower() == "psd"){
        QProcess process;
        QString const command = QString("/usr/local/bin/exiftool -Photoshop:PhotoshopThumbnail -b %1").
                arg(img_path);
        process.start(command);
        qDebug()<<__func__<<command;
        if(process.waitForFinished(5000)){
            auto img = QImage::fromData(process.readAll());
            if(img.isNull()){
                qDebug()<<__func__<<"exiftool cannot read thumbnail";
                img = QImage(img_path);
                if(!img.isNull()){
                    qDebug()<<__func__<<"plugin can read thumbnail";
                    return {qimage_to_cvmat(img), img_path};
                }
            }else{
                return exiftool_can_read(img, img_path);
            }
        }else{
            qDebug()<<__func__<<"process do not end";
            auto img = QImage(img_path);
            if(!img.isNull()){
                qDebug()<<__func__<<"plugin can read image";
                return {qimage_to_cvmat(img), img_path};
            }

        }
    }else{
        return {cv::imread(img_path.toStdString()), img_path};
    }

    return {{}, img_path};
}

void pics_find_img_hash::run()
{    
    if(!abs_file_path_.isEmpty()){
        abs_file_path_.sort();
        abs_file_path_.removeDuplicates();
        elapsed_time_.start();
        QDir(temp_folder).removeRecursively();
        QDir().mkdir(temp_folder);
        if(pool_size_ > 1){
            //compute_hash_mt2();
            compute_hash();
            qDebug()<<"compute hash mt2(ms) : "<<elapsed_time_.elapsed();
        }else{
            compute_hash();
            qDebug()<<"compute hash(ms) : "<<elapsed_time_.elapsed();
        }
        elapsed_time_.restart();
        compare_hash();
        qDebug()<<"compare hash consume(ms) : "<<elapsed_time_.elapsed();
    }

    emit end();
}

