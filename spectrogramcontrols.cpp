#include "spectrogramcontrols.h"
#include <QIntValidator>
#include <QFileDialog>
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

	centerFreq = new QLineEdit("0");
	centerFreq->setValidator(new QIntValidator(this));
	layout->addRow(new QLabel(tr("Center freq:")), centerFreq);

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
	connect(fileOpenButton, SIGNAL(clicked()), this, SLOT(fileOpenButtonClicked()));
}

void SpectrogramControls::fftSizeSliderChanged(int size)
{
	emit fftSizeChanged((int)pow(2, size));
}

void SpectrogramControls::fileOpenButtonClicked()
{
	QString fileName = QFileDialog::getOpenFileName(
		this, tr("Open File"), "", tr("Sample file (*.cfile *.bin);;All files (*)")
	);
	emit openFile(fileName);
}