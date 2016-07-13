#ifndef PAINT_CUSTOM_WORDS_H
#define PAINT_CUSTOM_WORDS_H

#include <QListView>

class paint_custom_words : public QListView
{
    Q_OBJECT
public:
    explicit paint_custom_words(QWidget *parent = nullptr);

    QString const& get_empty_str() const;
    void set_emptry_str(QString const &value);

signals:
    void view_selected();

private:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *e) override;

    QString empty_str_;
};

#endif // PAINT_CUSTOM_WORDS_H
