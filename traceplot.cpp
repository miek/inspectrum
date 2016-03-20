/*
 *  Copyright (C) 2016, Mike Walters <mike@flomp.net>
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

#include "samplesource.h"
#include "traceplot.h"

TracePlot::TracePlot(std::shared_ptr<AbstractSampleSource> source) : sampleSource(source) {

}

void TracePlot::paintMid(QPainter &painter, QRect &rect, range_t<off_t> sampleRange)
{
	auto firstSample = sampleRange.minimum;
	auto length = sampleRange.length();

	// Is it a 2-channel (complex) trace?
    if (auto src = dynamic_cast<SampleSource<std::complex<float>>*>(sampleSource.get())) {
        auto samples = src->getSamples(firstSample, length);
        if (samples == nullptr)
            return;

        painter.setPen(Qt::red);
        plotTrace(painter, rect, reinterpret_cast<float*>(samples.get()), length, 2);
        painter.setPen(Qt::blue);
        plotTrace(painter, rect, reinterpret_cast<float*>(samples.get())+1, length, 2);

    // Otherwise is it single channel?
    } else if (auto src = dynamic_cast<SampleSource<float>*>(sampleSource.get())) {
        auto samples = src->getSamples(firstSample, length);
        if (samples == nullptr)
            return;

        painter.setPen(Qt::green);
        plotTrace(painter, rect, samples.get(), length, 1);
    } else {
    	throw std::runtime_error("TracePlot::paintMid: Unsupported source type");
    }
}

void TracePlot::plotTrace(QPainter &painter, QRect &rect, float *samples, off_t count, int step = 1)
{
    QVector<QPoint> points(count);
    range_t<int> xRange{0, rect.width() - 2};
    range_t<int> yRange{0, rect.height() - 2};
    const float xStep = 1.0 / count * rect.width();
    for (off_t i = 0; i < count; i++) {
        float sample = samples[i*step];
        int x = i * xStep;
        int y = rect.height() - ((sample * rect.height()/2) + rect.height()/2);

        x = xRange.clip(x);
        y = yRange.clip(y);

        points[i] = {x + rect.x(), y + rect.y()};
    }
    painter.drawLines(points);
}
