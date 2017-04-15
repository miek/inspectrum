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
#include <QSettings>
#include <QLabel>
#include <cmath>
#include "util.h"

SpectrogramControls::SpectrogramControls(const QString & title, QWidget * parent)
    : QDockWidget::QDockWidget(title, parent)
{
    widget = new QWidget(this);
    layout = new QFormLayout(widget);

    sampleRate = new QLineEdit();
    sampleRate->setValidator(new QIntValidator(this));
    layout->addRow(new QLabel(tr("Sample rate:")), sampleRate);

    // Spectrogram settings
    layout->addRow(new QLabel()); // TODO: find a better way to add an empty row?
    layout->addRow(new QLabel(tr("<b>Spectrogram</b>")));

    fftSizeSlider = new QSlider(Qt::Horizontal, widget);
    fftSizeSlider->setRange(4, 13);
    fftSizeSlider->setPageStep(1);

    layout->addRow(new QLabel(tr("FFT size:")), fftSizeSlider);

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

    connect(fftSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(fftSliderChanged(int)));
    connect(powerMinSlider, SIGNAL(valueChanged(int)), this, SLOT(powerMinChanged(int)));
    connect(powerMaxSlider, SIGNAL(valueChanged(int)), this, SLOT(powerMaxChanged(int)));
}

void SpectrogramControls::clearCursorLabels()
{
    periodLabel->setText("");
    rateLabel->setText("");
    symbolPeriodLabel->setText("");
    symbolRateLabel->setText("");
}

void SpectrogramControls::setDefaults()
{
    QSettings settings;
    fftSizeSlider->setValue(settings.value("FFTSize", 9).toInt());

    cursorSymbolsSpinBox->setValue(1);

    int savedSampleRate = settings.value("SampleRate", 8000000).toInt();
    sampleRate->setText(QString::number(savedSampleRate));
    powerMaxSlider->setValue(settings.value("PowerMax", 0).toInt());
    powerMinSlider->setValue(settings.value("PowerMin", -100).toInt());
}

void SpectrogramControls::fftSliderChanged(int value)
{
    QSettings settings;
    settings.setValue("FFTSize", value);
    int fftSize = pow(2, value);
    emit fftSizeChanged(fftSize);
}

void SpectrogramControls::powerMinChanged(int value)
{
    QSettings settings;
    settings.setValue("PowerMin", value);
}

void SpectrogramControls::powerMaxChanged(int value)
{
    QSettings settings;
    settings.setValue("PowerMax", value);
}

void SpectrogramControls::timeSelectionChanged(float time)
{
    //if (cursorsCheckBox->checkState() == Qt::Checked) {
        periodLabel->setText(QString::fromStdString(formatSIValue(time)) + "s");
        rateLabel->setText(QString::fromStdString(formatSIValue(1 / time)) + "Hz");

        int symbols = cursorSymbolsSpinBox->value();
        symbolPeriodLabel->setText(QString::fromStdString(formatSIValue(time / symbols)) + "s");
        symbolRateLabel->setText(QString::fromStdString(formatSIValue(symbols / time)) + "Hz");
    //}
}
