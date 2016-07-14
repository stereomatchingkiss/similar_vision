#include "duplicate_img_model.hpp"

#include <QDebug>

duplicate_img_model::duplicate_img_model(QObject *parent) :
    QAbstractTableModel(parent)
{

}

int duplicate_img_model::columnCount(const QModelIndex&) const
{
    return 2;
}

QVariant duplicate_img_model::data(const QModelIndex &index, int role) const
{
    switch(role){
    case Qt::DisplayRole:{
        switch(index.column()){
        case 0: {            
            return items_.get<0>()[index.row()].origin_img_;
        }
        case 1: {
            return items_.get<0>()[index.row()].duplicate_img_;
        }
        default:{
            return {};
        }
        }
    }
    }

    return {};
}

QVariant duplicate_img_model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole){
        if(orientation == Qt::Horizontal){
            switch(section){
            case 0: {
                return QStringLiteral("Original Image");
            }
            case 1: {
                return QStringLiteral("Duplicate Image");
            }
            default:{
                return {};
            }
            }
        }else if(orientation == Qt::Vertical){
            return section + 1;
        }
    }
    return {};
}

bool duplicate_img_model::remove_img(const QString &img)
{
    //TODO : use a cheaper solution to remove image
    size_t const ori_size = items_.get<0>().size();
    items_.get<duplicate>().erase(img);
    items_.get<origin>().erase(img);
    if(ori_size != items_.get<0>().size()){
        removeRows(0, static_cast<int>(ori_size));
        insertRows(0, static_cast<int>(items_.get<0>().size()));
        return true;
    }

    return false;
}

bool duplicate_img_model::insertRows(int row, int count, const QModelIndex&)
{
    if(!items_.get<0>().empty()){
        beginInsertRows({}, row, row + count - 1);
        endInsertRows();
    }

    return true;
}

bool duplicate_img_model::removeRows(int row, int count, const QModelIndex&)
{
    beginRemoveRows({}, row, row + count - 1);
    endRemoveRows();

    return true;
}

int duplicate_img_model::rowCount(const QModelIndex&) const
{
    return static_cast<int>(items_.get<0>().size());
}

void duplicate_img_model::set_img_set(const QStringList &origin_img, const QStringList &duplicate_img)
{
    removeRows(0, static_cast<int>(items_.get<0>().size()));
    auto &items = items_.get<0>();
    items.clear();
    for(int i = 0; i != origin_img.size(); ++i){
        items.emplace_back(origin_img[i], duplicate_img[i]);
    }
    insertRows(0, origin_img.size());
}
