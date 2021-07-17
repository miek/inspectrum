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
    void setSampleRate(double rate);

signals:
    void timeSelectionChanged(float time);
    void zoomIn();
    void zoomOut();

public slots:
    void cursorsMoved();
    void enableCursors(bool enabled);
    void enableScales(bool enabled);
    void enableAnnos(bool enabled);
    void invalidateEvent() override;
    void repaint();
    void setCursorSegments(int segments);
    void setFFTAndZoom(int fftSize, int zoomLevel);
    void setPowerMin(int power);
    void setPowerMax(int power);

protected:
    void contextMenuEvent(QContextMenuEvent * event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent * event) override;
    void scrollContentsBy(int dx, int dy) override;
    bool viewportEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;

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
    int nfftSkip = 1;
    int powerMin;
    int powerMax;
    bool cursorsEnabled;
    double sampleRate = 0.0;
    bool timeScaleEnabled;
    int scrollZoomStepsAccumulated = 0;

    void addPlot(Plot *plot);
    void emitTimeSelection();
    void extractSymbols(std::shared_ptr<AbstractSampleSource> src, bool toClipboard);
    void exportSamples(std::shared_ptr<AbstractSampleSource> src);
    template<typename SOURCETYPE> void exportSamples(std::shared_ptr<AbstractSampleSource> src);
    int plotsHeight();
    size_t samplesPerColumn();
    void updateViewRange(bool reCenter);
    void updateView(bool reCenter = false, bool expanding = false);
    void paintTimeScale(QPainter &painter, QRect &rect, range_t<size_t> sampleRange);

    int sampleToColumn(size_t sample);
    size_t columnToSample(int col);
};
