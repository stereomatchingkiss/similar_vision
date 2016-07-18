#ifndef SINGLE_BAR_PLOT_HPP
#define SINGLE_BAR_PLOT_HPP

#include <qwt_plot.h>

class QwtColumnSymbol;
class QwtPlotBarChart;

class single_bar_plot : public QwtPlot
{
public:
    explicit single_bar_plot(QWidget *parent = nullptr);

    void set_data(QVector<double> const &value,
                  QStringList const &labels,
                  QColor const &color = QColor(115, 186, 37));

    void set_labels_title(QString const &title);
    void set_main_title(QString const &title);
    void set_orientation(Qt::Orientation orientation);
    void set_values_title(QString const &title);

private:
    QwtPlotBarChart *bar_chart_;    
    QStringList labels_;
    QString labels_title_;
    QwtColumnSymbol *symbol_;
    QString value_title_;
};

#endif // SINGLE_BAR_PLOT_HPP
