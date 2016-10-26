/*
 *  Copyright (C) 2015, Mike Walters <mike@flomp.net>
 *  Copyright (C) 2015, Jared Boone <jared@sharebrained.com>
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
#include "util.h"

SpectrogramControls::SpectrogramControls(const QString & title, QWidget * parent)
    : QDockWidget::QDockWidget(title, parent)
{
    widget = new QWidget(this);
    layout = new QFormLayout(widget);

    fileOpenButton = new QPushButton("Open file...", widget);
    layout->addRow(fileOpenButton);

    sampleRate = new QLineEdit();
    sampleRate->setValidator(new QIntValidator(this));
    layout->addRow(new QLabel(tr("Sample rate:")), sampleRate);

    // Spectrogram settings
    layout->addRow(new QLabel()); // TODO: find a better way to add an empty row?
    layout->addRow(new QLabel(tr("<b>Spectrogram</b>")));

    fftSizeSlider = new QSlider(Qt::Horizontal, widget);
    fftSizeSlider->setRange(7, 13);
    fftSizeSlider->setPageStep(1);

    layout->addRow(new QLabel(tr("FFT size:")), fftSizeSlider);

    zoomLevelSlider = new QSlider(Qt::Horizontal, widget);
    zoomLevelSlider->setRange(0, 10);
    zoomLevelSlider->setPageStep(1);

    layout->addRow(new QLabel(tr("Zoom:")), zoomLevelSlider);

    powerMaxSlider = new QSlider(Qt::Horizontal, widget);
    powerMaxSlider->setRange(-140, 10);
    layout->addRow(new QLabel(tr("Power max:")), powerMaxSlider);

    powerMinSlider = new QSlider(Qt::Horizontal, widget);
    powerMinSlider->setRange(-140, 10);
    layout->addRow(new QLabel(tr("Power min:")), powerMinSlider);

    scalesCheckBox = new QCheckBox(widget);
    scalesCheckBox->setCheckState(Qt::Checked);
    layout->addRow(new QLabel(tr("Scales:")), scalesCheckBox);

    // Time selection settings
    layout->addRow(new QLabel()); // TODO: find a better way to add an empty row?
    layout->addRow(new QLabel(tr("<b>Time selection</b>")));

    cursorsCheckBox = new QCheckBox(widget);
    layout->addRow(new QLabel(tr("Enable cursors:")), cursorsCheckBox);

    cursorSymbolsSpinBox = new QSpinBox();
    cursorSymbolsSpinBox->setMinimum(1);
    cursorSymbolsSpinBox->setMaximum(9999);
    layout->addRow(new QLabel(tr("Symbols:")), cursorSymbolsSpinBox);

    rateLabel = new QLabel();
    layout->addRow(new QLabel(tr("Rate:")), rateLabel);

    periodLabel = new QLabel();
    layout->addRow(new QLabel(tr("Period:")), periodLabel);

    symbolRateLabel = new QLabel();
    layout->addRow(new QLabel(tr("Symbol rate:")), symbolRateLabel);

    symbolPeriodLabel = new QLabel();
    layout->addRow(new QLabel(tr("Symbol period:")), symbolPeriodLabel);

    widget->setLayout(layout);
    setWidget(widget);

    connect(fftSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(fftOrZoomChanged(int)));
    connect(zoomLevelSlider, SIGNAL(valueChanged(int)), this, SLOT(fftOrZoomChanged(int)));
    connect(fileOpenButton, SIGNAL(clicked()), this, SLOT(fileOpenButtonClicked()));
    connect(cursorsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(cursorsStateChanged(int)));
}

void SpectrogramControls::clearCursorLabels()
{
    periodLabel->setText("");
    rateLabel->setText("");
    symbolPeriodLabel->setText("");
    symbolRateLabel->setText("");
}

void SpectrogramControls::cursorsStateChanged(int state)
{
    if (state == Qt::Unchecked) {
        clearCursorLabels();
    }
}

void SpectrogramControls::setDefaults()
{
    sampleRate->setText("8000000");
    fftSizeSlider->setValue(9);
    zoomLevelSlider->setValue(0);
    powerMaxSlider->setValue(0);
    powerMinSlider->setValue(-100);
    cursorsCheckBox->setCheckState(Qt::Unchecked);
    cursorSymbolsSpinBox->setValue(1);
}

void SpectrogramControls::fftOrZoomChanged(int value)
{
    int fftSize = pow(2, fftSizeSlider->value());
    int zoomLevel = std::min(fftSize, (int)pow(2, zoomLevelSlider->value()));
    emit fftOrZoomChanged(fftSize, zoomLevel);
}

void SpectrogramControls::fileOpenButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
                           this, tr("Open File"), "",
                           tr("complex<float> file (*.cfile *.cf32 *.fc32);;"
                              "complex<int8> HackRF file (*.cs8 *.sc8 *.c8);;"
                              "complex<int16> Fancy file (*.cs16 *.sc16 *.c16);;"
                              "complex<uint8> RTL-SDR file (*.cu8 *.uc8);;"
                              "All files (*)")
                       );
    if (!fileName.isEmpty())
        emit openFile(fileName);
}

void SpectrogramControls::timeSelectionChanged(float time)
{
    if (cursorsCheckBox->checkState() == Qt::Checked) {
        periodLabel->setText(QString::fromStdString(formatSIValue(time)) + "s");
        rateLabel->setText(QString::fromStdString(formatSIValue(1 / time)) + "Hz");

        int symbols = cursorSymbolsSpinBox->value();
        symbolPeriodLabel->setText(QString::fromStdString(formatSIValue(time / symbols)) + "s");
        symbolRateLabel->setText(QString::fromStdString(formatSIValue(symbols / time)) + "Hz");
    }
}

void SpectrogramControls::zoomIn()
{
    zoomLevelSlider->setValue(zoomLevelSlider->value() + 1);
}

void SpectrogramControls::zoomOut()
{
    zoomLevelSlider->setValue(zoomLevelSlider->value() - 1);
}
