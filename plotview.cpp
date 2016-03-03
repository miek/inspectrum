/*
 *  Copyright (C) 2015-2016, Mike Walters <mike@flomp.net>
 *
 *  This file is part of inspectrum.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "plotview.h"
#include <QDebug>
#include <QPainter>
#include <QScrollBar>
#include <gnuradio/top_block.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/blocks/multiply_const_cc.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/freq_xlating_fir_filter_ccf.h>
#include "grsamplebuffer.h"
#include "memory_sink.h"
#include "memory_source.h"

PlotView::PlotView(InputSource *input) : cursors(this), viewRange({0, 0})
{
    mainSampleSource = input;
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    enableCursors(false);

    spectrogramPlot = new SpectrogramPlot(mainSampleSource);
    plots.emplace_back(spectrogramPlot);

    iqPlot = createIQPlot(mainSampleSource);
    plots.emplace_back(iqPlot);
    plots.emplace_back(createQuadratureDemodPlot(static_cast<SampleSource<std::complex<float>>*>(iqPlot->source().get())));

    mainSampleSource->subscribe(this);
}

TracePlot* PlotView::createIQPlot(SampleSource<std::complex<float>> *src)
{
    gr::top_block_sptr iq_tb = gr::make_top_block("multiply");
    auto iq_mem_source = gr::blocks::memory_source::make(8);
    auto iq_mem_sink = gr::blocks::memory_sink::make(8);
    auto multiply = gr::blocks::multiply_const_cc::make(20);
    if (selection || true) {
        float centre = -0.05; //(selectionFreq.first + selectionFreq.second) / 2;
        float cutoff = 0.02; //std::abs(selectionFreq.first - centre);
        auto lp_taps = gr::filter::firdes::low_pass(1.0, 1.0, cutoff, cutoff / 2);
        auto filter = gr::filter::freq_xlating_fir_filter_ccf::make(1, lp_taps, centre, 1.0);

        iq_tb->connect(iq_mem_source, 0, filter, 0);
        iq_tb->connect(filter, 0, multiply, 0);
        iq_tb->connect(multiply, 0, iq_mem_sink, 0);
    } else {
        iq_tb->connect(iq_mem_source, 0, multiply, 0);
        iq_tb->connect(multiply, 0, iq_mem_sink, 0);
    }

    auto iq_src = std::make_shared<GRSampleBuffer<std::complex<float>, std::complex<float>>>(mainSampleSource, iq_tb, iq_mem_source, iq_mem_sink);
    return new TracePlot(iq_src);
}

TracePlot* PlotView::createQuadratureDemodPlot(SampleSource<std::complex<float>> *src)
{
    gr::top_block_sptr quad_demod_tb = gr::make_top_block("quad_demod");
    auto quad_demod_mem_source = gr::blocks::memory_source::make(8);
    auto quad_demod_mem_sink = gr::blocks::memory_sink::make(4);
    auto quad_demod = gr::analog::quadrature_demod_cf::make(5);
    quad_demod_tb->connect(quad_demod_mem_source, 0, quad_demod, 0);
    quad_demod_tb->connect(quad_demod, 0, quad_demod_mem_sink, 0);

    return new TracePlot(
        std::make_shared<GRSampleBuffer<std::complex<float>, float>>(
            dynamic_cast<SampleSource<std::complex<float>>*>(src), quad_demod_tb, quad_demod_mem_source, quad_demod_mem_sink
        )
    );
}

void PlotView::enableCursors(bool enabled)
{
    if (enabled)
        cursors.show();
    else
        cursors.hide();
}

void PlotView::invalidateEvent()
{
    horizontalScrollBar()->setMinimum(0);
    horizontalScrollBar()->setMaximum(mainSampleSource->count());
}

void PlotView::selectionChanged(std::pair<off_t, off_t> selectionTime, std::pair<float, float> selectionFreq)
{
    this->selectionTime = selectionTime;
    this->selectionFreq = selectionFreq;
    selection = true;
}

void PlotView::selectionCleared()
{
    selection = false;
}

void PlotView::setFFTSize(int size)
{
    fftSize = size;
    if (spectrogramPlot != nullptr)
        spectrogramPlot->setFFTSize(size);
    horizontalScrollBar()->setSingleStep(size * 10 / pow(2, zoomLevel));
    horizontalScrollBar()->setPageStep(size * 100 / pow(2, zoomLevel));
    updateView();
}

void PlotView::setZoomLevel(int zoom)
{
    zoomLevel = zoom;
    if (spectrogramPlot != nullptr)
        spectrogramPlot->setZoomLevel(zoom);
    updateView();
}

void PlotView::paintEvent(QPaintEvent *event)
{
    if (mainSampleSource == nullptr) return;

    QRect rect = QRect(0, 0, width(), height());
    QPainter painter(viewport());
    painter.fillRect(rect, Qt::black);


#define PLOT_LAYER(paintFunc)                                                   \
    {                                                                           \
        int y = 0;                                                              \
        for (auto&& plot : plots) {                                             \
            QRect rect = QRect(0, y, width(), plot->height());                  \
            plot->paintFunc(painter, rect, {viewRange.first, viewRange.second});\
            y += plot->height();                                                \
        }                                                                       \
    }

    PLOT_LAYER(paintBack);
    PLOT_LAYER(paintMid);
    PLOT_LAYER(paintFront);

#undef PLOT_LAYER
}

void PlotView::resizeEvent(QResizeEvent * event)
{
    QRect rect = viewport()->rect();

    // Resize cursors
    // TODO: don't hardcode this
    int margin = rect.width() / 3;
    cursors.setGeometry(QRect(rect.left() + margin, rect.top(), rect.right() - rect.left() - 2 * margin, rect.height()));

    updateView();
}

void PlotView::scrollContentsBy(int dx, int dy)
{
    updateView();
}

void PlotView::updateView()
{
    viewRange = {
        horizontalScrollBar()->value(),
        horizontalScrollBar()->value() + fftSize * width() / (int)pow(2, zoomLevel)
    };
    viewport()->update();
}
