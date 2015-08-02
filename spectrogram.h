#pragma once

#include <QWidget>
#include "fft.h"
#include "inputsource.h"

static const double Tau = M_PI * 2.0;

class Spectrogram : public QWidget {
	Q_OBJECT

public:
	Spectrogram();
	~Spectrogram();
	QSize sizeHint() const;

public slots:
	void pickFile();
	void setFFTSize(int size);
	void setPowerMax(int power);
	void setPowerMin(int power);

protected:
	void paintEvent(QPaintEvent *event);


private:
	InputSource *inputSource = nullptr;
	FFT *fft = nullptr;
	std::unique_ptr<float[]> window;
	fftwf_complex *lineBuffer = nullptr;

	int fftSize;
	int zoomLevel;
	float powerMax;
	float powerMin;

	void getLine(float *dest, int y);
	int getHeight();
};