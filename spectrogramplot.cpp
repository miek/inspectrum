/*
 *  Copyright (C) 2015, Mike Walters <mike@flomp.net>
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

#include "spectrogramplot.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmapCache>
#include <QRect>
#include <gnuradio/top_block.h>
#include <gnuradio/filter/firdes.h>

#include <cstdlib>
#include "util.h"


SpectrogramPlot::SpectrogramPlot(std::shared_ptr<SampleSource<std::complex<float>>> src) : Plot(src), inputSource(src), tuner(this)
{
    setFFTSize(512);
    zoomLevel = 0;
    powerMax = 0.0f;
    powerMin = -50.0f;

    for (int i = 0; i < 256; i++) {
        float p = (float)i / 256;
        colormap[i] = QColor::fromHsvF(p * 0.83f, 1.0, 1.0 - p).rgba();
    }

    auto tunerFlowGraph = gr::make_top_block("tuner");
    auto memSource = gr::blocks::memory_source::make(8);
    auto memSink = gr::blocks::memory_sink::make(8);
    tunerRotator = gr::blocks::rotator_ex_cc::make(getTunerPhaseInc());
    tunerFilter = gr::filter::fir_filter_ccf::make(1, getTunerTaps());

    tunerFlowGraph->connect(memSource, 0, tunerRotator, 0);
    tunerFlowGraph->connect(tunerRotator, 0, tunerFilter, 0);
    tunerFlowGraph->connect(tunerFilter, 0, memSink, 0);

    tunerOutput = std::make_shared<GRSampleBuffer<std::complex<float>, std::complex<float>>>(
        inputSource.get(), tunerFlowGraph, memSource, memSink
    );
    connect(&tuner, &Tuner::tunerMoved, this, &SpectrogramPlot::tunerMoved);
}

void SpectrogramPlot::paintFront(QPainter &painter, QRect &rect, range_t<off_t> sampleRange)
{
    if (tunerEnabled())
        tuner.paintFront(painter, rect, sampleRange);
}

void SpectrogramPlot::paintMid(QPainter &painter, QRect &rect, range_t<off_t> sampleRange)
{
    if (!inputSource || inputSource->count() == 0)
        return;

    off_t sampleOffset = sampleRange.minimum % (getStride() * linesPerTile());
    off_t tileID = sampleRange.minimum - sampleOffset;
    int xoffset = sampleOffset / getStride();

    // Paint first (possibly partial) tile
    painter.drawPixmap(QRect(rect.left(), rect.y(), linesPerTile() - xoffset, fftSize), *getPixmapTile(tileID), QRect(xoffset, 0, linesPerTile() - xoffset, fftSize));
    tileID += getStride() * linesPerTile();

    // Paint remaining tiles
    for (int x = linesPerTile() - xoffset; x < rect.right(); x += linesPerTile()) {
        // TODO: don't draw past rect.right()
        // TODO: handle partial final tile
        painter.drawPixmap(QRect(x, rect.y(), linesPerTile(), fftSize), *getPixmapTile(tileID));
        tileID += getStride() * linesPerTile();
    }
}

QPixmap* SpectrogramPlot::getPixmapTile(off_t tile)
{
    QPixmap *obj = pixmapCache.object(TileCacheKey(fftSize, zoomLevel, tile));
    if (obj != 0)
        return obj;

    float *fftTile = getFFTTile(tile);
    obj = new QPixmap(linesPerTile(), fftSize);
    QImage image(linesPerTile(), fftSize, QImage::Format_RGB32);
    for (int x = 0; x < linesPerTile(); x++) {
        float *line = &fftTile[x * fftSize];
        for (int y = 0; y < fftSize; y++) {
            float powerRange = std::abs(int(powerMin - powerMax));
            float normPower = (line[y] - powerMax) * -1.0f / powerRange;
            normPower = clamp(normPower, 0.0f, 1.0f);

            image.setPixel(x, fftSize - y - 1, colormap[(uint8_t)(normPower * (256 - 1))]);
        }
    }
    obj->convertFromImage(image);
    pixmapCache.insert(TileCacheKey(fftSize, zoomLevel, tile), obj);
    return obj;
}

float* SpectrogramPlot::getFFTTile(off_t tile)
{
    float *obj = fftCache.object(TileCacheKey(fftSize, zoomLevel, tile));
    if (obj != 0)
        return obj;

    float *dest = new float[tileSize];
    float *ptr = dest;
    off_t sample = tile;
    while ((ptr - dest) < tileSize) {
        getLine(ptr, sample);
        sample += getStride();
        ptr += fftSize;
    }
    fftCache.insert(TileCacheKey(fftSize, zoomLevel, tile), dest);
    return dest;
}

void SpectrogramPlot::getLine(float *dest, off_t sample)
{
    if (inputSource && fft) {
        auto buffer = inputSource->getSamples(sample, fftSize);
        if (buffer == nullptr)
            return;

        for (int i = 0; i < fftSize; i++) {
            buffer[i].real(buffer[i].real() * window[i]);
            buffer[i].imag(buffer[i].imag() * window[i]);
        }

        fft->process(buffer.get(), buffer.get());
        for (int i = 0; i < fftSize; i++) {
            // Start from the middle of the FFTW array and wrap
            // to rearrange the data
            int k = (i + fftSize / 2) & (fftSize - 1);
            float re = buffer[k].real();
            float im = buffer[k].imag();
            float mag = sqrt(re * re + im * im) / fftSize;
            float magdb = 10 * log2f(mag) / log2f(10);
            *dest = magdb;
            dest++;
        }
    }
}

int SpectrogramPlot::getStride()
{
    return fftSize / zoomLevel;
}

float SpectrogramPlot::getTunerPhaseInc()
{
    auto freq = 0.5f - tuner.centre() / (float)fftSize;
    return -freq * Tau;
}

std::vector<float> SpectrogramPlot::getTunerTaps()
{
    float gain = pow(10.0f, powerMax / -10.0f);
    float cutoff = tuner.deviation() / (float)fftSize;
    return gr::filter::firdes::low_pass(gain, 1.0, cutoff, cutoff / 2);
}

int SpectrogramPlot::linesPerTile()
{
    return tileSize / fftSize;
}

bool SpectrogramPlot::mouseEvent(QEvent::Type type, QMouseEvent event)
{
    if (tunerEnabled())
        return tuner.mouseEvent(type, event);

    return false;
}

std::shared_ptr<AbstractSampleSource> SpectrogramPlot::output()
{
    return tunerOutput;
}

void SpectrogramPlot::setFFTSize(int size)
{
    fftSize = size;
    fft.reset(new FFT(fftSize));

    window.reset(new float[fftSize]);
    for (int i = 0; i < fftSize; i++) {
        window[i] = 0.5f * (1.0f - cos(Tau * i / (fftSize - 1)));
    }

    setHeight(fftSize);
}

void SpectrogramPlot::setPowerMax(int power)
{
    powerMax = power;
    pixmapCache.clear();
    tunerMoved();
}

void SpectrogramPlot::setPowerMin(int power)
{
    powerMin = power;
    pixmapCache.clear();
}

void SpectrogramPlot::setZoomLevel(int zoom)
{
    zoomLevel = zoom;
}

bool SpectrogramPlot::tunerEnabled()
{
    return (tunerOutput->subscriberCount() > 0);
}

void SpectrogramPlot::tunerMoved()
{
    tunerRotator->set_phase_inc(getTunerPhaseInc());
    tunerFilter->set_taps(getTunerTaps());

    // TODO: for invalidating traceplot cache, this shouldn't really go here
    QPixmapCache::clear();

    emit repaint();
}

uint qHash(const TileCacheKey &key, uint seed)
{
    return key.fftSize ^ key.zoomLevel ^ key.sample ^ seed;
}
