#pragma once

#include <QMainWindow>
#include <QScrollArea>
#include "spectrogram.h"
#include "spectrogramcontrols.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

public slots:
	void setZoomLevel(int zoom);

private:
    QScrollArea scrollArea;
    Spectrogram spectrogram;
    SpectrogramControls *dock;

	off_t getCenterSample();
	int getScrollPos(off_t sample);
};
