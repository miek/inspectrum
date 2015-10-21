/*
 *  Copyright (C) 2015, Mike Walters <mike@flomp.net>
 *
 *  This file is part of inspectrum.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

	fftSizeSlider = new QSlider(Qt::Horizontal, widget);
	fftSizeSlider->setRange(7, 13);
	fftSizeSlider->setValue(10);
	layout->addRow(new QLabel(tr("FFT size:")), fftSizeSlider);

	zoomLevelSlider = new QSlider(Qt::Horizontal, widget);
	zoomLevelSlider->setRange(0, 10);
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