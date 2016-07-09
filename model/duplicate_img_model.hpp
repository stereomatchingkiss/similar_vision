#ifndef DUPLICATE_IMG_MODEL_H
#define DUPLICATE_IMG_MODEL_H

#include <QAbstractTableModel>

class duplicate_img_model : public QAbstractTableModel
{
    Q_OBJECT
public:    
    explicit duplicate_img_model(QObject *parent = nullptr);

    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    void set_img_set(QStringList const &origin_img, QStringList const &duplicate_img);

private:
    bool insertRows(int row, int count, const QModelIndex &parent = {}) override;
    bool removeRows(int row, int count, const QModelIndex &parent = {}) override;

    //no efficient for file deletion, should use boost::multi_index
    //to solve this issue
    QStringList duplicate_img_;
    QStringList origin_img_;
};

#endif // DUPLICATE_IMG_MODEL_H
