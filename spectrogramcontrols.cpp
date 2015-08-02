#include "spectrogramcontrols.h"
#include <QLabel>

SpectrogramControls::SpectrogramControls(const QString & title, QWidget * parent)
	: QDockWidget::QDockWidget(title, parent)
{
	widget = new QWidget(this);
	layout = new QFormLayout(widget);

	fileOpenButton = new QPushButton("Open file...", widget);
	layout->addRow(fileOpenButton);

	fftSizeSlider = new QSlider(Qt::Horizontal, widget);
	fftSizeSlider->setRange(7, 13);
	fftSizeSlider->setValue(10);
	layout->addRow(new QLabel(tr("FFT size:")), fftSizeSlider);

	zoomLevelSlider = new QSlider(Qt::Horizontal, widget);
	zoomLevelSlider->setRange(0, 5);
	zoomLevelSlider->setValue(0);
	layout->addRow(new QLabel(tr("Zoom:")), zoomLevelSlider);

	powerMaxSlider = new QSlider(Qt::Horizontal, widget);
	powerMaxSlider->setRange(-100, 20);
	powerMaxSlider->setValue(0);
	layout->addRow(new QLabel(tr("Power max:")), powerMaxSlider);

	powerMinSlider = new QSlider(Qt::Horizontal, widget);
	powerMinSlider->setRange(-100, 20);
	powerMinSlider->setValue(-50);
	layout->addRow(new QLabel(tr("Power min:")), powerMinSlider);

	widget->setLayout(layout);
	setWidget(widget);

	connect(fftSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(fftSizeSliderChanged(int)));
}

void SpectrogramControls::fftSizeSliderChanged(int size)
{
	emit fftSizeChanged((int)pow(2, size));
}