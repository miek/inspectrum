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
    void changeSampleRate(int rate);
    void openAnnotationFile(QString fileName);

public slots:
    void openFile(QString fileName);
	void setSampleRate(QString rate);
	void setFFTSize(int size);
	void setZoomLevel(int zoom);

protected:
	bool eventFilter(QObject *obj, QEvent *event);

private:
    QScrollArea scrollArea;
    Spectrogram spectrogram;
    SpectrogramControls *dock;

	off_t getCenterSample();
	int getScrollPos(off_t sample);
};
