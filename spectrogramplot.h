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

#pragma once

#include <QCache>
#include <QWidget>
#include "fft.h"
#include "inputsource.h"
#include "plot.h"
#include "tuner.h"
#include "tunertransform.h"

#include <memory>
#include <math.h>

class TileCacheKey;

class SpectrogramPlot : public Plot, public Subscriber
{
    Q_OBJECT

public:
    SpectrogramPlot(std::shared_ptr<SampleSource<std::complex<float>>> src);
    void invalidateEvent() override;
    std::shared_ptr<AbstractSampleSource> output() override;
    void paintFront(QPainter &painter, QRect &rect, range_t<off_t> sampleRange) override;
    void paintMid(QPainter &painter, QRect &rect, range_t<off_t> sampleRange) override;
    bool mouseEvent(QEvent::Type type, QMouseEvent event) override;
    void setSampleRate(off_t sampleRate);
    void enableScales(bool enabled);

public slots:
    void setFFTSize(int size);
    void setPowerMax(int power);
    void setPowerMin(int power);
    void setZoomLevel(int zoom);
    void tunerMoved();

private:
    const int linesPerGraduation = 50;
    static const int tileSize = 65536; // This must be a multiple of the maximum FFT size

    std::shared_ptr<SampleSource<std::complex<float>>> inputSource;
    std::unique_ptr<FFT> fft;
    std::unique_ptr<float[]> window;
    QCache<TileCacheKey, QPixmap> pixmapCache;
    QCache<TileCacheKey, std::array<float, tileSize>> fftCache;
    uint colormap[256];

    int fftSize;
    int zoomLevel;
    float powerMax;
    float powerMin;
    off_t sampleRate;
    bool frequencyScaleEnabled;

    Tuner tuner;
    std::shared_ptr<TunerTransform> tunerTransform;

    QPixmap* getPixmapTile(off_t tile);
    float* getFFTTile(off_t tile);
    void getLine(float *dest, off_t sample);
    int getStride();
    float getTunerPhaseInc();
    std::vector<float> getTunerTaps();
    int linesPerTile();
    bool tunerEnabled();
    void paintFrequencyScale(QPainter &painter, QRect &rect);
};

class TileCacheKey
{

public:
    TileCacheKey(int fftSize, int zoomLevel, off_t sample) {
        this->fftSize = fftSize;
        this->zoomLevel = zoomLevel;
        this->sample = sample;
    }

    bool operator==(const TileCacheKey &k2) const {
        return (this->fftSize == k2.fftSize) &&
               (this->zoomLevel == k2.zoomLevel) &&
               (this->sample == k2.sample);
    }

    int fftSize;
    int zoomLevel;
    off_t sample;
};
