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
#include <liquid/liquid.h>

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

    tunerTransform = std::make_shared<TunerTransform>(src.get());
    connect(&tuner, &Tuner::tunerMoved, this, &SpectrogramPlot::tunerMoved);
    tunerMoved();
}

void SpectrogramPlot::paintFront(QPainter &painter, QRect &rect, range_t<off_t> sampleRange)
{
    if (tunerEnabled())
        tuner.paintFront(painter, rect, sampleRange);

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
        int tickLine = (tickSample - sampleRange.minimum) / getStride();

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
        int tickLine = (tickSample - sampleRange.minimum) / getStride();

        painter.drawLine(tickLine, 0, tickLine, 10);
        tick += durationPerTick;
    }

    painter.restore();
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
    float powerRange = -1.0f / std::abs(int(powerMin - powerMax));
    for (int y = 0; y < fftSize; y++) {
        auto scanLine = (QRgb*)image.scanLine(fftSize - y - 1);
        for (int x = 0; x < linesPerTile(); x++) {
            float *fftLine = &fftTile[x * fftSize];
            float normPower = (fftLine[y] - powerMax) * powerRange;
            normPower = clamp(normPower, 0.0f, 1.0f);

            scanLine[x] = colormap[(uint8_t)(normPower * (256 - 1))];
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
            buffer[i] *= window[i];
        }

        fft->process(buffer.get(), buffer.get());
        const float invFFTSize = 1.0f / fftSize;
        const float logMultiplier = 10.0f / log2f(10.0f);
        for (int i = 0; i < fftSize; i++) {
            // Start from the middle of the FFTW array and wrap
            // to rearrange the data
            int k = i ^ (fftSize >> 1);
            auto s = buffer[k] * invFFTSize;
            float power = s.real() * s.real() + s.imag() * s.imag();
            float logPower = log2f(power) * logMultiplier;
            *dest = logPower;
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
    return freq * Tau;
}

std::vector<float> SpectrogramPlot::getTunerTaps()
{
    float cutoff = tuner.deviation() / (float)fftSize;
    float gain = pow(10.0f, powerMax / -10.0f);
    auto atten = 60.0f;
    auto len = estimate_req_filter_len(0.05f, atten);
    auto taps = std::vector<float>(len);
    liquid_firdes_kaiser(len, cutoff, atten, 0.0f, taps.data());
    std::transform(taps.begin(), taps.end(), taps.begin(),
                   std::bind1st(std::multiplies<float>(), gain));
    return taps;
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
    return tunerTransform;
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
    return (tunerTransform->subscriberCount() > 0);
}

void SpectrogramPlot::tunerMoved()
{
    tunerTransform->setFrequency(getTunerPhaseInc());
    tunerTransform->setTaps(getTunerTaps());

    // TODO: for invalidating traceplot cache, this shouldn't really go here
    QPixmapCache::clear();

    emit repaint();
}

uint qHash(const TileCacheKey &key, uint seed)
{
    return key.fftSize ^ key.zoomLevel ^ key.sample ^ seed;
}

void SpectrogramPlot::setSampleRate(off_t rate)
{
    sampleRate = rate;
    emit repaint();
}

