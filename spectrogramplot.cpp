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
#include <QRect>

#include <cstdlib>
#include "util.h"


SpectrogramPlot::SpectrogramPlot(SampleSource<std::complex<float>> *src)
{
    inputSource = src;
    sampleRate = 8000000;
    setFFTSize(512);
    zoomLevel = 0;
    powerMax = 0.0f;
    powerMin = -50.0f;
    timeScaleIsEnabled = true;
    deltaDragIsEnabled = true;

    for (int i = 0; i < 256; i++) {
        float p = (float)i / 256;
        colormap[i] = QColor::fromHsvF(p * 0.83f, 1.0, 1.0 - p).rgba();
    }

}

SpectrogramPlot::~SpectrogramPlot()
{
    delete fft;
    delete inputSource;
}

QSize SpectrogramPlot::sizeHint() const
{
    return QSize(1024, 2048);
}

void SpectrogramPlot::openFile(QString fileName)
{
    if (fileName != nullptr) {
        try {
            InputSource *newFile = new InputSource(fileName.toUtf8().constData());
            delete inputSource;
            pixmapCache.clear();
            fftCache.clear();
            inputSource = newFile;
        } catch (std::runtime_error e) {
            // TODO: display error
        }
    }
}

void SpectrogramPlot::xyToFreqTime(int x, int y, float *freq, float *time)
{
    *freq = labs(x - (fftSize / 2)) * sampleRate / 2 / (float)fftSize;
    *time = (float)lineToSample(y) / sampleRate;
}

void SpectrogramPlot::mouseReleaseEvent(QMouseEvent *event)
{
    if (deltaDragIsEnabled) {
        cursorStartX = -1;
        update();
    }
}

void SpectrogramPlot::mouseMoveEvent(QMouseEvent *event)
{
    float freq, time;
    xyToFreqTime(event->x(), event->y(), &freq, &time);
    emit cursorFrequencyChanged(QString::number(freq) + " Hz");
    emit cursorTimeChanged(QString::number(time) + " s");
    if (cursorStartX != -1) {
        float s_freq, s_time;
        xyToFreqTime(cursorStartX, cursorStartY, &s_freq, &s_time);
        emit deltaFrequencyChanged(QString::number(fabs(s_freq - freq)) + " Hz");
        emit deltaTimeChanged(QString::number(fabs(s_time - time)) + " s");
        cursorEndX = event->x();
        cursorEndY = event->y();
        update();
    }
}

void SpectrogramPlot::mousePressEvent(QMouseEvent *event)
{
    if (cursorStartX == -1) {
        cursorEndX = cursorStartX = event->x();
        cursorEndY = cursorStartY = event->y();
    } else {
        cursorStartX = -1;
    }
    update();
}

void SpectrogramPlot::paintMid(QPainter &painter, QRect &rect, range_t<off_t> sampleRange)
{
    for (int x = rect.left(); x < rect.right(); x += linesPerTile()) {
        QPixmap *tile = getPixmapTile(sampleRange.minimum + x * getStride());
        painter.drawPixmap(QRect(x, rect.y(), linesPerTile(), fftSize), *tile);
    }
}

void SpectrogramPlot::paintFront(QPainter &painter, QRect &rect, range_t<off_t> sampleRange)
{

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

            image.setPixel(x, y, colormap[(uint8_t)(normPower * (256 - 1))]);
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

        for (int i = 0; i < fftSize; i++) {
            buffer[i].real(buffer[i].real() * window[i]);
            buffer[i].imag(buffer[i].imag() * window[i]);
        }

        fft->process(buffer.get(), buffer.get());
        for (int i = 0; i < fftSize; i++) {
            int k = (i + fftSize / 2) % fftSize;
            float re = buffer[k].real();
            float im = buffer[k].imag();
            float mag = sqrt(re * re + im * im) / fftSize;
            float magdb = 10 * log2(mag) / log2(10);
            *dest = magdb;
            dest++;
        }
    }
}

void SpectrogramPlot::paintCursors(QPainter *painter, QRect rect)
{
    if (cursorStartX != -1) {
        painter->save();
        QPen pen(Qt::white, 1, Qt::DashLine);
        painter->setPen(pen);
        painter->drawLine(rect.left(), cursorStartY, rect.right(), cursorStartY);
        painter->drawLine(cursorStartX, rect.top(), cursorStartX, rect.bottom());
        painter->drawLine(rect.left(), cursorEndY, rect.right(), cursorEndY);
        painter->drawLine(cursorEndX, rect.top(), cursorEndX, rect.bottom());
        painter->restore();

    }
}

void SpectrogramPlot::setSampleRate(int rate)
{
    sampleRate = rate;
    update();
}

void SpectrogramPlot::setFFTSize(int size)
{
    fftSize = size;
    delete fft;
    fft = new FFT(fftSize);

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
    update();
}

void SpectrogramPlot::setPowerMin(int power)
{
    powerMin = power;
    pixmapCache.clear();
    update();
}

void SpectrogramPlot::setZoomLevel(int zoom)
{
    zoomLevel = clamp(zoom, 0, (int)log2(fftSize));
}

void SpectrogramPlot::setTimeScaleEnable(int state)
{
    timeScaleIsEnabled = (state == Qt::Checked);
    pixmapCache.clear();
    update();
}

void SpectrogramPlot::setDeltaDragEnable(int state)
{
    deltaDragIsEnabled = (state == Qt::Checked);
}


int SpectrogramPlot::getHeight()
{
    if (!inputSource)
        return 0;

    return inputSource->count() / getStride();
}

int SpectrogramPlot::getStride()
{
    return fftSize / pow(2, zoomLevel);
}

off_t SpectrogramPlot::lineToSample(off_t line)
{
    return line * getStride();
}

int SpectrogramPlot::sampleToLine(off_t sample)
{
    return sample / getStride();
}

QString SpectrogramPlot::sampleToTime(off_t sample)
{
    return QString::number((float)sample / sampleRate).append("s");
}

int SpectrogramPlot::linesPerTile()
{
    return tileSize / fftSize;
}

uint qHash(const TileCacheKey &key, uint seed)
{
    return key.fftSize ^ key.zoomLevel ^ key.sample ^ seed;
}
