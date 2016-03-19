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

#pragma once

#include <gnuradio/filter/freq_xlating_fir_filter_ccf.h>
#include <QAbstractScrollArea>
#include <QPaintEvent>

#include "cursors.h"
#include "inputsource.h"
#include "plot.h"
#include "samplesource.h"
#include "spectrogramplot.h"
#include "traceplot.h"
#include "tuner.h"

class PlotView : public QAbstractScrollArea, Subscriber
{
    Q_OBJECT

public:
    PlotView(InputSource *input);

signals:
    void timeSelectionChanged(float time);
    void zoomIn();
    void zoomOut();

public slots:
    void cursorsMoved();
    void enableCursors(bool enable);
    void invalidateEvent();
    void setCursorSegments(int segments);
    void setFFTAndZoom(int fftSize, int zoomLevel);
    void setPowerMin(int power);
    void setPowerMax(int power);
    void tunerMoved();

protected:
    bool eventFilter(QObject * obj, QEvent *event) override;
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent * event);
    void scrollContentsBy(int dx, int dy);

private:
    Cursors cursors;
    Tuner tuner;
    SampleSource<std::complex<float>> *mainSampleSource = nullptr;
    SpectrogramPlot *spectrogramPlot = nullptr;
    TracePlot *iqPlot = nullptr;
    gr::filter::freq_xlating_fir_filter_ccf::sptr plotFilter = nullptr;
    std::vector<std::unique_ptr<Plot>> plots;
    range_t<off_t> viewRange;
    bool selection = false;
    range_t<off_t> selectedSamples;
    std::pair<float, float> selectionFreq;

    int fftSize = 1024;
    int zoomLevel = 0;
    int powerMin;
    int powerMax;
    bool cursorsEnabled;

    TracePlot* createIQPlot(SampleSource<std::complex<float>> *src);
    TracePlot* createQuadratureDemodPlot(SampleSource<std::complex<float>> *src);
    int plotsHeight();
    off_t samplesPerLine();
    void updateView(bool reCenter = false);
};