#include "spectrogramcontrols.h"
#include <QLabel>

SpectrogramControls::SpectrogramControls(const QString & title, QWidget * parent)
	: QDockWidget::QDockWidget(title, parent)
{
	QWidget *widget = new QWidget;
	fftSizeSlider = new QSlider(Qt::Horizontal);
	fftSizeSlider->setRange(7, 13);
	fftSizeSlider->setValue(10);

	layout = new QFormLayout;
	layout->addRow(new QLabel(tr("FFT size:")), fftSizeSlider);
	widget->setLayout(layout);
	setWidget(widget);

	connect(fftSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(fftSizeSliderChanged(int)));
}

void SpectrogramControls::fftSizeSliderChanged(int size)
{
	emit fftSizeChanged((int)pow(2, size));
}