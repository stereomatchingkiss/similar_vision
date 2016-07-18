#ifndef DUPLICATE_IMG_MODEL_HPP
#define DUPLICATE_IMG_MODEL_HPP

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <QAbstractTableModel>

class duplicate_img_model : public QAbstractTableModel
{
    Q_OBJECT
public:    
    explicit duplicate_img_model(QObject *parent = nullptr);

    void clear();

    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    bool remove_img(QString const &img);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    void set_img_set(QStringList const &origin_img, QStringList const &duplicate_img);

private:
    bool insertRows(int row, int count, const QModelIndex &parent = {}) override;
    bool removeRows(int row, int count, const QModelIndex &parent = {}) override;

    struct model_items
    {
        model_items(QString const &duplicate_img,
                    QString const &origin_img) :
            duplicate_img_(duplicate_img),
            origin_img_(origin_img)
        {}
        QString duplicate_img_;
        QString origin_img_;
    };

    struct duplicate{};
    struct origin{};

    // define a multiply indexed set with indices by id and name
    using items_set = boost::multi_index::multi_index_container
    <
        model_items,
        boost::multi_index::indexed_by<
            boost::multi_index::random_access<>,
            boost::multi_index::ordered_non_unique<
                boost::multi_index::tag<duplicate>,
                boost::multi_index::member<model_items,QString,&model_items::duplicate_img_>
            >,
            boost::multi_index::ordered_non_unique<
                boost::multi_index::tag<origin>,
                boost::multi_index::member<model_items,QString,&model_items::origin_img_>
            >
        >
    >;

    items_set items_;
};

#endif // DUPLICATE_IMG_MODEL_HPP
