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

#include <QPixmapCache>
#include <QTextStream>
#include "samplesource.h"
#include "traceplot.h"

TracePlot::TracePlot(std::shared_ptr<AbstractSampleSource> source) : sampleSource(source) {

}

void TracePlot::paintMid(QPainter &painter, QRect &rect, range_t<off_t> sampleRange)
{
    int samplesPerColumn = sampleRange.length() / rect.width();
    int samplesPerTile = tileWidth * samplesPerColumn;
    off_t tileID = sampleRange.minimum / samplesPerTile;
    off_t tileOffset = sampleRange.minimum % samplesPerTile; // Number of samples to skip from first image tile
    int xOffset = tileOffset / samplesPerColumn; // Number of columns to skip from first image tile

    // Paint first (possibly partial) tile
    painter.drawPixmap(
        QRect(rect.x(), rect.y(), tileWidth - xOffset, height()),
        getTile(tileID++, samplesPerTile),
        QRect(xOffset, 0, tileWidth - xOffset, height())
    );

    // Paint remaining tiles
    for (int x = tileWidth - xOffset; x < rect.right(); x += tileWidth) {
        painter.drawPixmap(
            QRect(x, rect.y(), tileWidth, height()),
            getTile(tileID++, samplesPerTile)
        );
    }
}

QPixmap TracePlot::getTile(off_t tileID, off_t sampleCount)
{
    QPixmap pixmap;
    QString key;
    QTextStream(&key) << "traceplot_" << this << "_" << tileID << "_" << sampleCount;
    if (QPixmapCache::find(key, &pixmap))
        return pixmap;

    auto image = drawTile(QRect(0, 0, tileWidth, height()), {tileID * sampleCount, (tileID + 1) * sampleCount});
    pixmap = QPixmap::fromImage(image);
    QPixmapCache::insert(key, pixmap);
    return pixmap;
}

QImage TracePlot::drawTile(const QRect &rect, range_t<off_t> sampleRange)
{
    QImage image(rect.size(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);

	auto firstSample = sampleRange.minimum;
	auto length = sampleRange.length();

	// Is it a 2-channel (complex) trace?
    if (auto src = dynamic_cast<SampleSource<std::complex<float>>*>(sampleSource.get())) {
        auto samples = src->getSamples(firstSample, length);
        if (samples == nullptr)
            return image;

        painter.setPen(Qt::red);
        plotTrace(painter, rect, reinterpret_cast<float*>(samples.get()), length, 2);
        painter.setPen(Qt::blue);
        plotTrace(painter, rect, reinterpret_cast<float*>(samples.get())+1, length, 2);

    // Otherwise is it single channel?
    } else if (auto src = dynamic_cast<SampleSource<float>*>(sampleSource.get())) {
        auto samples = src->getSamples(firstSample, length);
        if (samples == nullptr)
            return image;

        painter.setPen(Qt::green);
        plotTrace(painter, rect, samples.get(), length, 1);
    } else {
    	throw std::runtime_error("TracePlot::paintMid: Unsupported source type");
    }

    return image;
}

void TracePlot::plotTrace(QPainter &painter, const QRect &rect, float *samples, off_t count, int step = 1)
{
    QPainterPath path;
    range_t<float> xRange{0, rect.width() - 2.f};
    range_t<float> yRange{0, rect.height() - 2.f};
    const float xStep = 1.0 / count * rect.width();
    for (off_t i = 0; i < count; i++) {
        float sample = samples[i*step];
        float x = i * xStep;
        float y = (1 - sample) * (rect.height() / 2);

        x = xRange.clip(x) + rect.x();
        y = yRange.clip(y) + rect.y();

        if (i == 0)
            path.moveTo(x, y);
        else
            path.lineTo(x, y);
    }
    painter.drawPath(path);
}
