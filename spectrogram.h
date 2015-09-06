#pragma once

#include <QWidget>
#include "fft.h"
#include "inputsource.h"

#include <math.h>

static const double Tau = M_PI * 2.0;

class Spectrogram : public QWidget {
	Q_OBJECT

public:
	Spectrogram();
	~Spectrogram();
	QSize sizeHint() const;
	int getHeight();
	int getStride();

public slots:
	void openFile(QString fileName);
	void setSampleRate(int rate);
	void setFFTSize(int size);
	void setPowerMax(int power);
	void setPowerMin(int power);
	void setZoomLevel(int zoom);

protected:
	void paintEvent(QPaintEvent *event);


private:
	const int linesPerGraduation = 50;
	const int tileSize = 32768; // This must be a multiple of the maximum FFT size

	InputSource *inputSource = nullptr;
	FFT *fft = nullptr;
	std::unique_ptr<float[]> window;
	fftwf_complex *lineBuffer = nullptr;
	QHash<QPair<int, off_t>, float*> fftCache;
	uint colormap[256];

	int sampleRate;
	int fftSize;
	int zoomLevel;
	float powerMax;
	float powerMin;

	float* getTile(off_t tile);
	void getLine(float *dest, off_t sample);
	void paintTimeAxis(QPainter *painter, QRect rect);
	off_t lineToSample(int line);
	QString sampleToTime(off_t sample);
	int linesPerTile();
};