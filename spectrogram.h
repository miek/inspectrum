#pragma once

#include <QWidget>
#include "inputsource.h"

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
	InputSource *inputSource;

	int fftSize;
	float powerMax;
	float powerMin;
};