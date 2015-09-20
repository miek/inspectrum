#pragma once

#include <QCache>
#include <QWidget>
#include "fft.h"
#include "inputsource.h"

#include <math.h>

static const double Tau = M_PI * 2.0;

class TileCacheKey;

class Spectrogram : public QWidget {
	Q_OBJECT

public:
	Spectrogram();
	~Spectrogram();
	QSize sizeHint() const;
	int getHeight();
	int getStride();
	int getSampleRate();
	int getFFTSize();
	QString getFileName();

public slots:
	void openFile(QString fileName);
	void setSampleRate(int rate);
	void setFFTSize(int size);
	void setPowerMax(int power);
	void setPowerMin(int power);
	void setZoomLevel(int zoom);
	off_t lineToSample(int line);

protected:
	void paintEvent(QPaintEvent *event);


private:
	const int linesPerGraduation = 50;
	const int tileSize = 65536; // This must be a multiple of the maximum FFT size

	InputSource *inputSource = nullptr;
	FFT *fft = nullptr;
	std::unique_ptr<float[]> window;
	fftwf_complex *lineBuffer = nullptr;
	QCache<TileCacheKey, QPixmap> pixmapCache;
	QCache<TileCacheKey, float> fftCache;
	uint colormap[256];

	int sampleRate;
	int fftSize;
	int zoomLevel;
	float powerMax;
	float powerMin;
	QString curName;

	QPixmap* getPixmapTile(off_t tile);
	float* getFFTTile(off_t tile);
	void getLine(float *dest, off_t sample);
	void paintTimeAxis(QPainter *painter, QRect rect);
	int sampleToLine(off_t sample);
	QString sampleToTime(off_t sample);
	int linesPerTile();
};

class TileCacheKey {

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
