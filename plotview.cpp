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
#include <iostream>
#include <QApplication>
#include <QDebug>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include "plots.h"

PlotView::PlotView(InputSource *input) : cursors(this), viewRange({0, 0})
{
    mainSampleSource = input;
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    enableCursors(false);
    enableTimeScale(true);
    connect(&cursors, SIGNAL(cursorsMoved()), this, SLOT(cursorsMoved()));

    spectrogramPlot = new SpectrogramPlot(std::shared_ptr<SampleSource<std::complex<float>>>(mainSampleSource));
    auto tunerOutput = std::dynamic_pointer_cast<SampleSource<std::complex<float>>>(spectrogramPlot->output());

    addPlot(spectrogramPlot);

    viewport()->installEventFilter(this);
    mainSampleSource->subscribe(this);
}

void PlotView::addPlot(Plot *plot)
{
    plots.emplace_back(plot);
    connect(plot, &Plot::repaint, this, &PlotView::repaint);
}

void PlotView::contextMenuEvent(QContextMenuEvent * event)
{
    QMenu menu;

    // Get selected plot
    Plot *selectedPlot = nullptr;
    int y = -verticalScrollBar()->value();
    for (auto&& plot : plots) {
        if (range_t<int>{y, y + plot->height()}.contains(event->pos().y()))
            selectedPlot = plot.get();
        y += plot->height();
    }
    if (selectedPlot == nullptr)
        return;

    // Add actions to add derived plots
    // that are compatible with selectedPlot's output
    QMenu *plotsMenu = menu.addMenu("Add derived plot");
    auto src = selectedPlot->output();
    auto compatiblePlots = as_range(Plots::plots.equal_range(src->sampleType()));
    for (auto p : compatiblePlots) {
        auto plotInfo = p.second;
        auto action = new QAction(QString("Add %1").arg(plotInfo.name), plotsMenu);
        auto plotCreator = plotInfo.creator;
        connect(
            action, &QAction::triggered,
            this, [=]() {
                addPlot(plotCreator(src));
            }
        );
        plotsMenu->addAction(action);
    }

    // Add action to extract symbols from selected plot
    auto extract = new QAction("Extract symbols...", &menu);
    connect(
        extract, &QAction::triggered,
        this, [=]() {
            extractSymbols(src);
        }
    );
    extract->setEnabled(cursorsEnabled && (src->sampleType() == typeid(float)));
    menu.addAction(extract);

    if (menu.exec(event->globalPos()))
        updateView(false);
}

void PlotView::cursorsMoved()
{
    selectedSamples = {
        horizontalScrollBar()->value() + cursors.selection().minimum * samplesPerLine(),
        horizontalScrollBar()->value() + cursors.selection().maximum * samplesPerLine()
    };

    emitTimeSelection();
    viewport()->update();
}

void PlotView::emitTimeSelection()
{
    off_t sampleCount = selectedSamples.length();
    float selectionTime = sampleCount / (float)mainSampleSource->rate();
    emit timeSelectionChanged(selectionTime);
}

void PlotView::enableCursors(bool enabled)
{
    cursorsEnabled = enabled;
    if (enabled) {
        int margin = viewport()->rect().width() / 3;
        cursors.setSelection({viewport()->rect().left() + margin, viewport()->rect().right() - margin});
        cursorsMoved();
    }
    viewport()->update();
}

bool PlotView::eventFilter(QObject * obj, QEvent *event)
{
    // Pass mouse events to individual plot objects
    if (event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseMove ||
        event->type() == QEvent::MouseButtonRelease) {

        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        if (cursorsEnabled)
            if (cursors.mouseEvent(event->type(), *mouseEvent))
                return true;

        int plotY = -verticalScrollBar()->value();
        for (auto&& plot : plots) {
            bool result = plot->mouseEvent(
                event->type(),
                QMouseEvent(
                    event->type(),
                    QPoint(mouseEvent->pos().x(), mouseEvent->pos().y() - plotY),
                    mouseEvent->button(),
                    mouseEvent->buttons(),
                    QApplication::keyboardModifiers()
                )
            );
            if (result)
                return true;
            plotY += plot->height();
        }
    }

    // Handle wheel events for zooming
    if (event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = (QWheelEvent*)event;
        if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
            if (wheelEvent->angleDelta().y() > 0) {
                emit zoomIn();
            } else if (wheelEvent->angleDelta().y() < 0) {
                emit zoomOut();
            }
            return true;
        }
    }
    return false;
}

void PlotView::extractSymbols(std::shared_ptr<AbstractSampleSource> src)
{
    if (!cursorsEnabled)
        return;
    auto floatSrc = std::dynamic_pointer_cast<SampleSource<float>>(src);
    if (!floatSrc)
        return;
    auto samples = floatSrc->getSamples(selectedSamples.minimum, selectedSamples.length());
    auto step = (float)selectedSamples.length() / cursors.segments();
    auto symbols = std::vector<float>();
    for (auto i = step / 2; i < selectedSamples.length(); i += step)
    {
        symbols.push_back(samples[i]);
    }
    for (auto f : symbols)
        std::cout << f << ", ";
    std::cout << std::endl << std::flush;
}

void PlotView::invalidateEvent()
{
    horizontalScrollBar()->setMinimum(0);
    horizontalScrollBar()->setMaximum(mainSampleSource->count());
}

void PlotView::repaint()
{
    viewport()->update();
}

void PlotView::setCursorSegments(int segments)
{
    // Calculate number of samples per segment
    float sampPerSeg = (float)selectedSamples.length() / cursors.segments();

    // Alter selection to keep samples per segment the same
    selectedSamples.maximum = selectedSamples.minimum + (segments * sampPerSeg + 0.5f);

    cursors.setSegments(segments);
    updateView();
    emitTimeSelection();
}

void PlotView::setFFTAndZoom(int size, int zoom)
{
    // Set new FFT size
    fftSize = size;
    if (spectrogramPlot != nullptr)
        spectrogramPlot->setFFTSize(size);

    // Set new zoom level
    zoomLevel = zoom;
    if (spectrogramPlot != nullptr)
        spectrogramPlot->setZoomLevel(zoom);

    // Update horizontal (time) scrollbar
    horizontalScrollBar()->setSingleStep(size * 10 / zoomLevel);
    horizontalScrollBar()->setPageStep(size * 100 / zoomLevel);

    updateView(true);
}

void PlotView::setPowerMin(int power)
{
    powerMin = power;
    if (spectrogramPlot != nullptr)
        spectrogramPlot->setPowerMin(power);
    updateView();
}

void PlotView::setPowerMax(int power)
{
    powerMax = power;
    if (spectrogramPlot != nullptr)
        spectrogramPlot->setPowerMax(power);
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
        int y = -verticalScrollBar()->value();                                  \
        for (auto&& plot : plots) {                                             \
            QRect rect = QRect(0, y, width(), plot->height());                  \
            plot->paintFunc(painter, rect, viewRange);                          \
            y += plot->height();                                                \
        }                                                                       \
    }

    PLOT_LAYER(paintBack);
    PLOT_LAYER(paintMid);
    PLOT_LAYER(paintFront);
    if (cursorsEnabled)
        cursors.paintFront(painter, rect, viewRange);

    if (timeScaleEnabled)
        paintTimeScale(painter, rect, viewRange);

#undef PLOT_LAYER
}

void PlotView::paintTimeScale(QPainter &painter, QRect &rect, range_t<off_t> sampleRange)
{
    float startTime = (float)sampleRange.minimum / sampleRate;
    float stopTime = (float)sampleRange.maximum / sampleRate;
    float duration = stopTime - startTime;

    painter.save();

    QPen pen(Qt::white, 1, Qt::SolidLine);
    painter.setPen(pen);
    QFontMetrics fm(painter.font());

    int tickWidth = 80;
    int maxTicks = rect.width() / tickWidth;

    double durationPerTick = 10 * pow(10, floor(log(duration / maxTicks) / log(10)));

    double firstTick = int(startTime / durationPerTick) * durationPerTick;

    double tick = firstTick;

    while (tick <= stopTime) {

        off_t tickSample = tick * sampleRate;
        int tickLine = (tickSample - sampleRange.minimum) / samplesPerLine();

        char buf[128];
        snprintf(buf, sizeof(buf), "%.06f", tick);
        painter.drawLine(tickLine, 0, tickLine, 30);
        painter.drawText(tickLine + 2, 25, buf);

        tick += durationPerTick;
    }

    // Draw small ticks
    durationPerTick /= 10;
    firstTick = int(startTime / durationPerTick) * durationPerTick;
    tick = firstTick;
    while (tick <= stopTime) {

        off_t tickSample = tick * sampleRate;
        int tickLine = (tickSample - sampleRange.minimum) / samplesPerLine();

        painter.drawLine(tickLine, 0, tickLine, 10);
        tick += durationPerTick;
    }

    painter.restore();
}


int PlotView::plotsHeight()
{
    int height = 0;
    for (auto&& plot : plots) {
        height += plot->height();
    }
    return height;
}

void PlotView::resizeEvent(QResizeEvent * event)
{
    updateView();
}

off_t PlotView::samplesPerLine()
{
    return fftSize / zoomLevel;
}

void PlotView::scrollContentsBy(int dx, int dy)
{
    updateView();
}

void PlotView::updateView(bool reCenter)
{
    // Store old view for recentering
    auto oldViewRange = viewRange;

    // Update current view
    viewRange = {
        horizontalScrollBar()->value(),
        horizontalScrollBar()->value() + width() * samplesPerLine()
    };

    // Adjust time offset to zoom around central sample
    if (reCenter) {
        horizontalScrollBar()->setValue(
            horizontalScrollBar()->value() + (oldViewRange.length() - viewRange.length()) / 2
        );
    }

    horizontalScrollBar()->setMaximum(mainSampleSource->count() - ((width() - 1) * samplesPerLine()));

    verticalScrollBar()->setMaximum(std::max(0, plotsHeight() - viewport()->height()));

    // Update cursors
    QRect rect = viewport()->rect();
    range_t<int> newSelection = {
        (int)((selectedSamples.minimum - horizontalScrollBar()->value()) / samplesPerLine()),
        (int)((selectedSamples.maximum - horizontalScrollBar()->value()) / samplesPerLine())
    };
    cursors.setSelection(newSelection);

    // Re-paint
    viewport()->update();
}

void PlotView::setSampleRate(off_t rate)
{
    sampleRate = rate;
    emitTimeSelection();
}

void PlotView::enableTimeScale(bool enabled)
{
    timeScaleEnabled = enabled;
    viewport()->update();
}

