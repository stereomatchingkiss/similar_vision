#include "pics_find_img_hash.hpp"

#include "vp_tree.hpp"

#include <ocv_libs/profile/measure.hpp>

#include <QtConcurrent/QtConcurrentMap>
#include <QDebug>

#include <opencv2/core/ocl.hpp>
#include <opencv2/highgui.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>

#include <atomic>
#include <iostream>
#include <mutex>

namespace{

using profiler = ocv::time::measure<std::chrono::milliseconds>;

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
        //std::cout<<u.m_source<<","<<u.m_target<<std::endl;
        //std::cout<<u<<std::endl;
        edges_.emplace_back(u);
    }

    std::vector<edges_type> &edges_;
};

struct norm_hamming_dist
{
    using value_type = std::pair<QString, cv::Mat>;

    double operator()(value_type const &lhs,
                      value_type const &rhs) const
    {
        return cv::norm(lhs.second, rhs.second, cv::NORM_HAMMING);
    }
};

template<typename Tree>
graph_type create_graph(Tree const &tree,
                        int sample_size, int actual_size)
{
    using value_type = norm_hamming_dist::value_type;

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
                    [](double val){ return val <= 5; });
        for(auto const &pair : result){
            auto it = name_id.find(pair.first);
            if(it != std::end(name_id)){
                boost::add_edge(i, it->second, graph);
            }
        }
    }

    return graph;
}

template<typename Graph, typename Items>
std::vector<edges_type> create_edges(Graph const &graph, Items const &items)
{
    std::vector<edges_type> edges;
    dfs_visitor vis(edges);
    boost::depth_first_search(graph, boost::visitor(vis));
    for(auto const &edge : edges){
        qDebug()<<"("<<edge.m_source<<","<<edge.m_target
               <<") : "<<items[edge.m_source].first<<", "
              <<items[edge.m_target].first;
    }

    return edges;
}

}//nameless namespace

pics_find_img_hash::
pics_find_img_hash(cv::Ptr<cv::img_hash::ImgHashBase> algo,
                   QStringList const &abs_file_path,
                   QObject *parent) :
    QThread(parent),
    abs_file_path_(abs_file_path),
    algo_(algo)
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
    using value_type = norm_hamming_dist::value_type;

    vp_tree<value_type, norm_hamming_dist> hamming_tree;
    qDebug()<<"norm ham vp create : "<<
              profiler::execution([&](){hamming_tree.create(std::move(hash_arr_));});

    auto const graph = create_graph(hamming_tree,
                                    abs_file_path_.size(), 10);
    auto const edges = create_edges(graph, hamming_tree.get_items());
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
    for(auto const &name : abs_file_path_){
        auto const img = cv::imread(name.toStdString());
        if(!img.empty()){
            cv::Mat hash;
            algo_->compute(img, hash);
            hash_arr_.emplace_back(name, hash);
        }
        emit progress("Image preprocess st : " +
                      QString("%1").arg(hash_arr_.size()));
    }
}

void pics_find_img_hash::compute_hash_mt()
{
    hash_arr_.clear();
    std::mutex mutex;
    std::atomic<size_t> size = 0;
    auto compute_hash = [&](QString const &name)
    {
        auto const img = cv::imread(name.toStdString());
        if(!img.empty()){
            ++size;
            cv::Mat hash;
            //cv::img_hash::averageHash(img, hash);
            std::lock_guard<std::mutex> guard(mutex);
            algo_->compute(img, hash);
            hash_arr_.emplace_back(name, hash);
        }
        qDebug()<<"cur id : "<<QThread::currentThreadId();
        emit progress("Image preprocess mt : " +
                      QString("%1").arg(size.load()));
    };
    QtConcurrent::blockingMap(abs_file_path_, compute_hash);
}

void pics_find_img_hash::run()
{    
    abs_file_path_.sort();
    abs_file_path_.removeDuplicates();
    qDebug()<<"compute hash mt(ms) : "<<profiler::execution([this](){compute_hash_mt();});
    //qDebug()<<"compute hash(ms) : "<<profiler::execution([this](){compute_hash();});
    qDebug()<<"compare hash(ms) : "<<profiler::execution([this](){compare_hash();});

    emit end();
}
