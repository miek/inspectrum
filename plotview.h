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

#include <QGraphicsView>
#include <QPaintEvent>

#include "cursors.h"
#include "inputsource.h"
#include "plot.h"
#include "samplesource.h"
#include "spectrogramplot.h"
#include "traceplot.h"

class PlotView : public QGraphicsView, Subscriber
{
    Q_OBJECT

public:
    PlotView(InputSource *input);
    void setSampleRate(size_t rate);

signals:
    void timeSelectionChanged(float time);

public slots:
    void cursorsMoved();
    void enableCursors(bool enabled);
    void enableScales(bool enabled);
    void invalidateEvent();
    void repaint();
    void setCursorSegments(int segments);
    void setFFTSize(int fftSize);
    void setPowerMin(int power);
    void setPowerMax(int power);
    void zoomIn();
    void zoomOut();

protected:
    void contextMenuEvent(QContextMenuEvent * event) override;
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent * event);
    void scrollContentsBy(int dx, int dy);
    bool viewportEvent(QEvent *event) override;

private:
    Cursors cursors;
    SampleSource<std::complex<float>> *mainSampleSource = nullptr;
    SpectrogramPlot *spectrogramPlot = nullptr;
    std::vector<std::unique_ptr<Plot>> plots;
    range_t<size_t> viewRange;
    range_t<size_t> selectedSamples;
    int zoomPos;
    size_t zoomSample;

    int fftSize = 1024;
    int zoomLevel = 1;
    int powerMin;
    int powerMax;
    bool cursorsEnabled;
    size_t sampleRate = 0;
    bool timeScaleEnabled;
    int scrollZoomStepsAccumulated = 0;

    void addPlot(Plot *plot);
    void emitTimeSelection();
    void extractSymbols(std::shared_ptr<AbstractSampleSource> src);
    void exportSamples(std::shared_ptr<AbstractSampleSource> src);
    template<typename SOURCETYPE> void exportSamples(std::shared_ptr<AbstractSampleSource> src);
    int plotsHeight();
    size_t samplesPerColumn();
    void setFFTAndZoom(int fftSize, int zoomLevel);
    void updateViewRange(bool reCenter);
    void updateView(bool reCenter = false);
    void paintTimeScale(QPainter &painter, QRect &rect, range_t<size_t> sampleRange);

    int sampleToColumn(size_t sample);
    size_t columnToSample(int col);
};
