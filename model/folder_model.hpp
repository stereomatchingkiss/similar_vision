#ifndef FOLDER_MODEL_HPP
#define FOLDER_MODEL_HPP

#include <QStringListModel>

class folder_model : public QStringListModel
{
    Q_OBJECT
public:
    explicit folder_model(QObject *parent = nullptr);

    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column,
                      const QModelIndex &parent) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;

signals:
    void drop_data();

private:

};

#endif // FOLDER_MODEL_HPP
