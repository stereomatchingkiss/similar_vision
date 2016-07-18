#include "single_bar_plot.hpp"

#include <qwt_plot_renderer.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_barchart.h>
#include <qwt_column_symbol.h>
#include <qwt_plot_layout.h>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>

namespace{

class label_scale_draw: public QwtScaleDraw
{
public:
    label_scale_draw(Qt::Orientation orientation, const QStringList &labels):
        d_labels(labels)
    {
        setTickLength(QwtScaleDiv::MinorTick, 0);
        setTickLength(QwtScaleDiv::MediumTick, 0);
        setTickLength(QwtScaleDiv::MajorTick, 2);

        enableComponent(QwtScaleDraw::Backbone, false);

        if ( orientation == Qt::Vertical ){
            setLabelRotation( -60.0 );
        }else{
            setLabelRotation( -20.0 );
        }

        setLabelAlignment( Qt::AlignLeft | Qt::AlignVCenter );
    }

    QwtText label(double value) const override
    {
        QwtText lbl;

        const int index = qRound( value );
        if(index >= 0 && index < d_labels.size()){
            lbl = d_labels[index];
        }

        return lbl;
    }

private:
    const QStringList d_labels;
};

} //nameless namespace

single_bar_plot::single_bar_plot(QWidget *parent) :
    QwtPlot(parent),
    bar_chart_(new QwtPlotBarChart)
{
    bar_chart_->setLayoutPolicy(QwtPlotBarChart::AutoAdjustSamples);
    bar_chart_->setLayoutHint(4.0); // minimum width for a single bar
    bar_chart_->setSpacing(10); // spacing between bars

    setAutoFillBackground(true);
    setPalette(QColor("Linen"));

    QwtPlotCanvas *canvas = new QwtPlotCanvas;
    canvas->setLineWidth(2);
    canvas->setFrameStyle(QFrame::Box | QFrame::Sunken);
    canvas->setBorderRadius(10);

    QPalette canvasPalette(QColor("Plum"));
    canvasPalette.setColor(QPalette::Foreground, QColor("Indigo"));
    canvas->setPalette(canvasPalette);

    setCanvas(canvas);

    bar_chart_->attach(this);
    set_orientation(Qt::Vertical);
    setAutoReplot(false);

    symbol_ = new QwtColumnSymbol(QwtColumnSymbol::Box);
    symbol_->setLineWidth(2);
    symbol_->setFrameStyle(QwtColumnSymbol::Raised);
    symbol_->setPalette(QPalette(QColor(115, 186, 37)));
    bar_chart_->setSymbol(symbol_);
}

void single_bar_plot::set_data(const QVector<double> &value,
                               const QStringList &labels,
                               QColor const &color)
{
    if(labels.size() != value.size()){
        //very rough error handle
        exit(1);
    }
    bar_chart_->setSamples(value);
    labels_ = labels;
    symbol_->setPalette(QPalette(color));
    set_orientation(Qt::Vertical);
}

void single_bar_plot::set_main_title(const QString &title)
{
    setTitle(title);
}

void single_bar_plot::set_orientation(Qt::Orientation orientation)
{    
    int axis1 = QwtPlot::xBottom;
    int axis2 = QwtPlot::yLeft;

    if(orientation == Qt::Horizontal){
        std::swap(axis1, axis2);
    }

    bar_chart_->setOrientation( orientation );

    setAxisTitle(axis1, labels_title_);
    setAxisMaxMinor(axis1, 3);
    setAxisScaleDraw(axis1, new label_scale_draw(orientation, labels_));

    setAxisTitle(axis2, value_title_);
    setAxisMaxMinor(axis2, 3);

    QwtScaleDraw *scaleDraw = new QwtScaleDraw;
    scaleDraw->setTickLength( QwtScaleDiv::MediumTick, 4 );
    setAxisScaleDraw(axis2, scaleDraw);

    plotLayout()->setCanvasMargin(0);
    replot();
    updateCanvasMargins();
}

void single_bar_plot::set_labels_title(const QString &title)
{
    labels_title_ = title;
}

void single_bar_plot::set_values_title(const QString &title)
{
    value_title_ = title;
}
