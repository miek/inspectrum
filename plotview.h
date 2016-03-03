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

#include <QAbstractScrollArea>
#include <QPaintEvent>

#include "cursors.h"
#include "inputsource.h"
#include "plot.h"
#include "samplesource.h"
#include "spectrogramplot.h"
#include "traceplot.h"

class PlotView : public QAbstractScrollArea, Subscriber
{
    Q_OBJECT

public:
    PlotView(InputSource *input);

public slots:
    void enableCursors(bool enable);
    void invalidateEvent();
    void selectionChanged(std::pair<off_t, off_t> selectionTime, std::pair<float, float> selectionFreq);
    void selectionCleared();
    void setFFTSize(int size);
    void setZoomLevel(int zoom);
    void setPowerMin(int power);
    void setPowerMax(int power);

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent * event);
    void scrollContentsBy(int dx, int dy);

private:
    Cursors cursors;
    SampleSource<std::complex<float>> *mainSampleSource = nullptr;
    SpectrogramPlot *spectrogramPlot = nullptr;
    TracePlot *iqPlot = nullptr;
    std::vector<std::unique_ptr<Plot>> plots;
    std::pair<off_t, off_t> viewRange;
    bool selection = false;
    std::pair<off_t, off_t> selectionTime;
    std::pair<float, float> selectionFreq;

    int fftSize;
    int zoomLevel;
    int powerMin;
    int powerMax;

    TracePlot* createIQPlot(SampleSource<std::complex<float>> *src);
    TracePlot* createQuadratureDemodPlot(SampleSource<std::complex<float>> *src);
    void updateView();
};