#pragma once

#include <QDockWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>

class SpectrogramControls : public QDockWidget {
	Q_OBJECT

public:
	SpectrogramControls(const QString & title, QWidget * parent);

signals:
	void fftSizeChanged(int size);
	void openFile(QString fileName);

private slots:
	void fftSizeSliderChanged(int size);
	void fileOpenButtonClicked();

private:
	QWidget *widget;
	QFormLayout *layout;
public:
	QPushButton *fileOpenButton;
	QLineEdit *sampleRate;
	QLineEdit *centerFreq;
	QSlider *fftSizeSlider;
	QSlider *zoomLevelSlider;
	QSlider *powerMaxSlider;
	QSlider *powerMinSlider;
};