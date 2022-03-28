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
#include <QString>
#include <QWidget>
#include "fft.h"
#include "inputsource.h"
#include "plot.h"
#include "tuner.h"
#include "tunertransform.h"

#include <memory>
#include <array>
#include <math.h>
#include <vector>

class TileCacheKey;
class AnnotationLocation;

class SpectrogramPlot : public Plot
{
    Q_OBJECT

public:
    SpectrogramPlot(std::shared_ptr<SampleSource<std::complex<float>>> src);
    void invalidateEvent() override;
    std::shared_ptr<AbstractSampleSource> output() override;
    void paintFront(QPainter &painter, QRect &rect, range_t<size_t> sampleRange) override;
    void paintMid(QPainter &painter, QRect &rect, range_t<size_t> sampleRange) override;
    bool mouseEvent(QEvent::Type type, QMouseEvent event) override;
    std::shared_ptr<SampleSource<std::complex<float>>> input() { return inputSource; };
    void setSampleRate(double sampleRate);
    bool tunerEnabled();
    void enableScales(bool enabled);
    void enableAnnotations(bool enabled);
    bool isAnnotationsEnabled();
    void enableAnnoColors(bool enabled);
    QString *mouseAnnotationComment(const QMouseEvent *event);

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
    std::vector<AnnotationLocation> visibleAnnotationLocations;
    std::unique_ptr<FFT> fft;
    std::unique_ptr<float[]> window;
    QCache<TileCacheKey, QPixmap> pixmapCache;
    QCache<TileCacheKey, std::array<float, tileSize>> fftCache;
    uint colormap[256];

    int fftSize;
    int zoomLevel;
    float powerMax;
    float powerMin;
    double sampleRate;
    bool frequencyScaleEnabled;
    bool sigmfAnnotationsEnabled;
    bool sigmfAnnotationColors;

    Tuner tuner;
    std::shared_ptr<TunerTransform> tunerTransform;

    QPixmap* getPixmapTile(size_t tile);
    float* getFFTTile(size_t tile);
    void getLine(float *dest, size_t sample);
    int getStride();
    float getTunerPhaseInc();
    std::vector<float> getTunerTaps();
    int linesPerTile();
    void paintFrequencyScale(QPainter &painter, QRect &rect);
    void paintAnnotations(QPainter &painter, QRect &rect, range_t<size_t> sampleRange);
};

class TileCacheKey
{

public:
    TileCacheKey(int fftSize, int zoomLevel, size_t sample) {
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
    size_t sample;
};

class AnnotationLocation
{
public:
    Annotation annotation;

    AnnotationLocation(Annotation annotation, int x, int y, int width, int height)
        : annotation(annotation), x(x), y(y), width(width), height(height) {}

    bool isInside(int pos_x, int pos_y) {
        return (x <= pos_x) && (pos_x <= x + width)
            && (y <= pos_y) && (pos_y <= y + height);
    }

private:
    int x;
    int y;
    int width;
    int height;
};
