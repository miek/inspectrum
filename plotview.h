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
    void setSampleRate(off_t rate);

signals:
    void timeSelectionChanged(float time);
    void zoomIn();
    void zoomOut();

public slots:
    void cursorsMoved();
    void enableCursors(bool enabled);
    void enableScales(bool enabled);
    void invalidateEvent();
    void repaint();
    void setCursorSegments(int segments);
    void setFFTAndZoom(int fftSize, int zoomLevel);
    void setPowerMin(int power);
    void setPowerMax(int power);

protected:
    void contextMenuEvent(QContextMenuEvent * event) override;
    bool eventFilter(QObject * obj, QEvent *event) override;
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent * event);
    void scrollContentsBy(int dx, int dy);

private:
    Cursors cursors;
    SampleSource<std::complex<float>> *mainSampleSource = nullptr;
    SpectrogramPlot *spectrogramPlot = nullptr;
    std::vector<std::unique_ptr<Plot>> plots;
    range_t<off_t> viewRange;
    range_t<off_t> selectedSamples;

    int fftSize = 1024;
    int zoomLevel = 0;
    int powerMin;
    int powerMax;
    bool cursorsEnabled;
    off_t sampleRate = 0;
    bool timeScaleEnabled;

    void addPlot(Plot *plot);
    void emitTimeSelection();
    void extractSymbols(std::shared_ptr<AbstractSampleSource> src);
    enum SampleType
    {
        REAL,
        COMPLEX
    };
    void exportSamples(std::shared_ptr<AbstractSampleSource> src, SampleType exportType);
    template<typename SOURCETYPE> void exportSamples(std::shared_ptr<AbstractSampleSource> src);
    int plotsHeight();
    off_t samplesPerLine();
    void updateViewRange(bool reCenter);
    void updateView(bool reCenter = false);
    void paintTimeScale(QPainter &painter, QRect &rect, range_t<off_t> sampleRange);
};
