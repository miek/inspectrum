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
	void pickFile();
	void setSampleRate(int rate);
	void setFFTSize(int size);
	void setPowerMax(int power);
	void setPowerMin(int power);
	void setZoomLevel(int zoom);

protected:
	void paintEvent(QPaintEvent *event);


private:
	const int linesPerGraduation = 50;

	InputSource *inputSource = nullptr;
	FFT *fft = nullptr;
	std::unique_ptr<float[]> window;
	fftwf_complex *lineBuffer = nullptr;

	int sampleRate;
	int fftSize;
	int zoomLevel;
	float powerMax;
	float powerMin;

	void getLine(float *dest, int y);
	void paintTimeAxis(QPainter *painter, QRect rect);
	off_t lineToSample(int line);
	QString sampleToTime(off_t sample);
};