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
            return origin_img_[index.row()];
        }
        case 1: {
            return duplicate_img_[index.row()];
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

bool duplicate_img_model::insertRows(int row, int count, const QModelIndex&)
{
    if(!origin_img_.isEmpty()){
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
    return duplicate_img_.size();
}

void duplicate_img_model::set_img_set(const QStringList &origin_img, const QStringList &duplicate_img)
{
    removeRows(0, origin_img_.size());
    duplicate_img_ = duplicate_img;
    origin_img_ = origin_img;
    insertRows(0, origin_img_.size());
}
