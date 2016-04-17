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
    connect(&cursors, SIGNAL(cursorsMoved()), this, SLOT(cursorsMoved()));

    spectrogramPlot = new SpectrogramPlot(std::shared_ptr<SampleSource<std::complex<float>>>(mainSampleSource));
    auto tunerOutput = std::dynamic_pointer_cast<SampleSource<std::complex<float>>>(spectrogramPlot->output());

    addPlot(spectrogramPlot);
    addPlot(Plots::samplePlot(tunerOutput));
    addPlot(Plots::frequencyPlot(tunerOutput));

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

    Plot *selectedPlot = nullptr;
    int y = -verticalScrollBar()->value();
    for (auto&& plot : plots) {
        if (range_t<int>{y, y + plot->height()}.contains(event->pos().y()))
            selectedPlot = plot.get();
        y += plot->height();
    }
    if (selectedPlot == nullptr)
        return;

    auto src = selectedPlot->output();
    auto compatiblePlots = as_range(Plots::plots.equal_range(src->sampleType()));
    for (auto p : compatiblePlots) {
        auto action = new QAction("Add plot", &menu);
        auto plotCreator = p.second;
        connect(
            action, &QAction::triggered,
            this, [=]() {
                addPlot(plotCreator(src));
            }
        );
        menu.addAction(action);
    }
    if (menu.exec(event->globalPos()))
        updateView(false);
}

void PlotView::cursorsMoved()
{
    selectedSamples = {
        horizontalScrollBar()->value() + cursors.selection().minimum * samplesPerLine(),
        horizontalScrollBar()->value() + cursors.selection().maximum * samplesPerLine()
    };

    off_t sampleCount = selectedSamples.length();
    float selectionTime = sampleCount / (float)mainSampleSource->rate();
    emit timeSelectionChanged(selectionTime);
    viewport()->update();
}

void PlotView::enableCursors(bool enabled)
{
    cursorsEnabled = enabled;
    if (enabled) {
        int margin = viewport()->rect().width() / 3;
        cursors.setSelection({viewport()->rect().left() + margin, viewport()->rect().right() - margin});
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
    cursors.setSegments(segments);
    cursorsMoved();
    viewport()->update();
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

#undef PLOT_LAYER
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
