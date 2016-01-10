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

#include "spectrogram.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QPainter>
#include <QPaintEvent>
#include <QRect>

#include <cstdlib>
#include "util.h"


Spectrogram::Spectrogram()
{
    sampleRate = 8000000;
    setFFTSize(1024);
    zoomLevel = 0;
    powerMax = 0.0f;
    powerMin = -50.0f;

    for (int i = 0; i < 256; i++) {
        float p = (float)i / 256;
        colormap[i] = QColor::fromHsvF(p * 0.83f, 1.0, 1.0 - p).rgba();
    }
}

Spectrogram::~Spectrogram()
{
    delete fft;
    delete inputSource;
}

QSize Spectrogram::sizeHint() const
{
    return QSize(1024, 2048);
}

void Spectrogram::openFile(QString fileName)
{
    if (fileName != nullptr) {
        try {
            InputSource *newFile = new InputSource(fileName.toUtf8().constData());
            delete inputSource;
            pixmapCache.clear();
            fftCache.clear();
            inputSource = newFile;
            resize(fftSize, getHeight());
        } catch (std::runtime_error e) {
            // TODO: display error
        }
    }
}

void Spectrogram::paintEvent(QPaintEvent *event)
{
    QRect rect = event->rect();
    QPainter painter(this);
    painter.fillRect(rect, Qt::black);

    if (inputSource != nullptr) {
        int height = rect.height();
        off_t y = rect.y();

        QImage image(fftSize, height, QImage::Format_RGB32);

        while (height > 0) {
            int tileOffset = y % linesPerTile(); // To handle drawing a partial first tile
            int drawHeight = std::min(linesPerTile() - tileOffset, height); // Draw rest of first tile, full tile, or partial final tile
            off_t tileId = lineToSample(y - tileOffset);
            QPixmap *tile = getPixmapTile(tileId);
            painter.drawPixmap(QRect(0, y, fftSize, drawHeight), *tile, QRect(0, tileOffset, fftSize, drawHeight));
            y += drawHeight;
            height -= drawHeight;
        }

        paintTimeAxis(&painter, rect);
    }
}

QPixmap* Spectrogram::getPixmapTile(off_t tile)
{
    QPixmap *obj = pixmapCache.object(TileCacheKey(fftSize, zoomLevel, tile));
    if (obj != 0)
        return obj;

    float *fftTile = getFFTTile(tile);
    obj = new QPixmap(fftSize, linesPerTile());
    QImage image(fftSize, linesPerTile(), QImage::Format_RGB32);
    for (int y = 0; y < linesPerTile(); y++) {
        float *line = &fftTile[y * fftSize];
        for (int x = 0; x < fftSize; x++) {
            float powerRange = std::abs(int(powerMin - powerMax));
            float normPower = (line[x] - powerMax) * -1.0f / powerRange;
            normPower = clamp(normPower, 0.0f, 1.0f);

            image.setPixel(x, y, colormap[(uint8_t)(normPower * (256 - 1))]);
        }
    }
    obj->convertFromImage(image);
    pixmapCache.insert(TileCacheKey(fftSize, zoomLevel, tile), obj);
    return obj;
}

float* Spectrogram::getFFTTile(off_t tile)
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

void Spectrogram::getLine(float *dest, off_t sample)
{
    if (inputSource && fft) {
        fftwf_complex buffer[fftSize];
        inputSource->getSamples(buffer, sample, fftSize);

        for (int i = 0; i < fftSize; i++) {
            buffer[i][0] *= window[i];
            buffer[i][1] *= window[i];
        }

        fft->process(buffer, buffer);
        for (int i = 0; i < fftSize; i++) {
            int k = (i + fftSize / 2) % fftSize;
            float re = buffer[k][0];
            float im = buffer[k][1];
            float mag = sqrt(re * re + im * im) / fftSize;
            float magdb = 10 * log2(mag) / log2(10);
            *dest = magdb;
            dest++;
        }
    }
}

void Spectrogram::paintTimeAxis(QPainter *painter, QRect rect)
{
    // Round up for firstLine and round each to nearest linesPerGraduation
    int firstLine = ((rect.y() + linesPerGraduation - 1) / linesPerGraduation) * linesPerGraduation;
    int lastLine = ((rect.y() + rect.height()) / linesPerGraduation) * linesPerGraduation;

    painter->save();
    QPen pen(Qt::white, 1, Qt::SolidLine);
    painter->setPen(pen);
    QFontMetrics fm(painter->font());
    int textOffset = fm.ascent() / 2 - 1;
    for (int line = firstLine; line <= lastLine; line += linesPerGraduation) {
        painter->drawLine(0, line, 10, line);
        painter->drawText(12, line + textOffset, sampleToTime(lineToSample(line)));
    }
    painter->restore();
}

void Spectrogram::setSampleRate(int rate)
{
    sampleRate = rate;
    update();
}

void Spectrogram::setFFTSize(int size)
{
    fftSize = size;
    delete fft;
    fft = new FFT(fftSize);

    window.reset(new float[fftSize]);
    for (int i = 0; i < fftSize; i++) {
        window[i] = 0.5f * (1.0f - cos(Tau * i / (fftSize - 1)));
    }

    resize(fftSize, getHeight());
}

void Spectrogram::setPowerMax(int power)
{
    powerMax = power;
    pixmapCache.clear();
    update();
}

void Spectrogram::setPowerMin(int power)
{
    powerMin = power;
    pixmapCache.clear();
    update();
}

void Spectrogram::setZoomLevel(int zoom)
{
    zoomLevel = clamp(zoom, 0, (int)log2(fftSize));
    resize(fftSize, getHeight());
}

int Spectrogram::getHeight()
{
    if (!inputSource)
        return 0;

    return inputSource->getSampleCount() / getStride();
}

int Spectrogram::getStride()
{
    return fftSize / pow(2, zoomLevel);
}

off_t Spectrogram::lineToSample(off_t line)
{
    return line * getStride();
}

int Spectrogram::sampleToLine(off_t sample)
{
    return sample / getStride();
}

QString Spectrogram::sampleToTime(off_t sample)
{
    return QString::number((float)sample / sampleRate).append("s");
}

int Spectrogram::linesPerTile()
{
    return tileSize / fftSize;
}

uint qHash(const TileCacheKey &key, uint seed)
{
    return key.fftSize ^ key.zoomLevel ^ key.sample ^ seed;
}
