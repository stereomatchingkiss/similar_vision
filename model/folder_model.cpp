#include "folder_model.hpp"

#include <QDebug>
#include <QFileInfo>
#include <QMimeData>
#include <QUrl>

folder_model::folder_model(QObject *parent) :
    QStringListModel(parent)
{

}

bool folder_model::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(parent);

    if(action == Qt::CopyAction && data->hasUrls()){
        auto list = stringList();
        int const pre_size = list.size();
        for(auto const &url : data->urls()){
            QFileInfo const info(url.toLocalFile());
            if(info.isDir()){
               list.push_back(info.absoluteFilePath());
            }
        }
        list.removeDuplicates();
        setStringList(list);

        emit drop_data();

        return pre_size != list.size();
    }

    return false;
}

Qt::ItemFlags folder_model::flags(const QModelIndex &index) const
{
    return QStringListModel::flags(index) |  Qt::ItemIsDropEnabled;
}

QStringList folder_model::mimeTypes() const
{
    return QStringList()<<"text/uri-list";
}
