#pragma once

#include <QWidget>
#include "inputsource.h"

class Spectrogram : public QWidget {
public:
	Spectrogram();
	~Spectrogram();

protected:
	void paintEvent(QPaintEvent *event);

private:
	InputSource *inputSource;
};