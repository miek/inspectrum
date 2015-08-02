#pragma once

#include <QDockWidget>
#include <QFormLayout>
#include <QPushButton>
#include <QSlider>

class SpectrogramControls : public QDockWidget {
	Q_OBJECT

public:
	SpectrogramControls(const QString & title, QWidget * parent);

signals:
	void fftSizeChanged(int size);

private slots:
	void fftSizeSliderChanged(int size);

private:
	QWidget *widget;
	QFormLayout *layout;
	QSlider *fftSizeSlider;
public:
	QPushButton *fileOpenButton;
	QSlider *zoomLevelSlider;
	QSlider *powerMaxSlider;
	QSlider *powerMinSlider;
};