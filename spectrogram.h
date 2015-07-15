#pragma once

#include <QWidget>
#include "inputsource.h"

class Spectrogram : public QWidget {
	Q_OBJECT

public:
	Spectrogram();
	~Spectrogram();

public slots:
	void setFFTSize(int size);
	void setPowerMax(int power);
	void setPowerRange(int power);

protected:
	void paintEvent(QPaintEvent *event);


private:
	InputSource *inputSource;

	float powerMax;
	float powerRange;
};