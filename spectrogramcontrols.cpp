#include "spectrogramcontrols.h"
#include <QIntValidator>
#include <QLabel>
#include <cmath>

SpectrogramControls::SpectrogramControls(const QString & title, QWidget * parent)
	: QDockWidget::QDockWidget(title, parent)
{
	widget = new QWidget(this);
	layout = new QFormLayout(widget);

	fileOpenButton = new QPushButton("Open file...", widget);
	layout->addRow(fileOpenButton);

	sampleRate = new QLineEdit("8000000");
	sampleRate->setValidator(new QIntValidator(this));
	layout->addRow(new QLabel(tr("Sample rate:")), sampleRate);

	fftSizeSlider = new QSlider(Qt::Horizontal, widget);
	fftSizeSlider->setRange(7, 13);
	fftSizeSlider->setValue(10);
	layout->addRow(new QLabel(tr("FFT size:")), fftSizeSlider);

	zoomLevelSlider = new QSlider(Qt::Horizontal, widget);
	zoomLevelSlider->setRange(0, 5);
	zoomLevelSlider->setValue(0);
	layout->addRow(new QLabel(tr("Zoom:")), zoomLevelSlider);

	powerSlider = new QxtSpanSlider(Qt::Horizontal);
	powerSlider->setRange(-100, 20);
	powerSlider->setSpan(-50, 0);
	layout->addRow(new QLabel(tr("Power range:")), powerSlider);

	widget->setLayout(layout);
	setWidget(widget);

	connect(fftSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(fftSizeSliderChanged(int)));
}

void SpectrogramControls::fftSizeSliderChanged(int size)
{
	emit fftSizeChanged((int)pow(2, size));
}