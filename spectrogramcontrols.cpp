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
    // dockable control panel holding groups of controls
    controlDockWidget = new QWidget(this);                    // dockable container for the controls
    QVBoxLayout *controlDockLayout = new QVBoxLayout();       // a vertical layout 
    controlDockWidget->setLayout(controlDockLayout);          // add the vertical layout to the dock

    // file control group
    QGroupBox *fileGroup = new QGroupBox(tr("File"));         // group with a title
    QVBoxLayout *fileGroupLayout = new QVBoxLayout();         // and a vertical layout 
    fileGroup->setLayout(fileGroupLayout);

    fileOpenButton = new QPushButton(tr("&Open..."), controlDockWidget);              // button to open a file
    fileGroupLayout->addWidget(fileOpenButton);

    sampleRate = new QLineEdit();                                                     // sample rate entry
    sampleRate->setValidator(new QIntValidator(this));
    sampleRate->setToolTip(tr("The frequency scale, time scale and timing analysis depend on this being correct."));
    QFormLayout *sampleRateLayout = new QFormLayout;                                  // lay it out as a qform
    sampleRateLayout->addRow(new QLabel(tr("Sample rate:")), sampleRate);
    fileGroupLayout->addLayout(sampleRateLayout);                                     // and add it to the file group

    // plot settings group
    QGroupBox *plotSettingsGroup = new QGroupBox(tr("Spectrogram"));                  // group with a title
    QFormLayout *plotSettingsLayout = new QFormLayout();                              // and a form layout
    plotSettingsGroup->setLayout(plotSettingsLayout);                                 // add the layout to the group

    fftSizeSlider = new QSlider(Qt::Horizontal, controlDockWidget);                   // slider for the height of the FFT plot
    fftSizeSlider->setRange(7, 13);
    fftSizeSlider->setPageStep(1);
    fftSizeSlider->setToolTip(tr("Adjust the height of the spectrogram."));
    plotSettingsLayout->addRow(new QLabel(tr("FFT size:")), fftSizeSlider);

    zoomLevelSlider = new QSlider(Qt::Horizontal, controlDockWidget);                 // slider for the plot zoom
    zoomLevelSlider->setRange(0, 10);
    zoomLevelSlider->setPageStep(1);
    zoomLevelSlider->setToolTip(tr("Adjust the width of the spectrogram."));
    plotSettingsLayout->addRow(new QLabel(tr("Zoom:")), zoomLevelSlider);

    powerMaxSlider = new QSlider(Qt::Horizontal, controlDockWidget);                  // max power slider
    powerMaxSlider->setRange(-140, 10);
    powerMaxSlider->setToolTip(tr("Adjust this to help visualise the signal."));
    plotSettingsLayout->addRow(new QLabel(tr("Power max:")), powerMaxSlider);

    powerMinSlider = new QSlider(Qt::Horizontal, controlDockWidget);                  // min power slider
    powerMinSlider->setRange(-140, 10);
    powerMinSlider->setToolTip(tr("Adjust this to help visualise the signal."));
    plotSettingsLayout->addRow(new QLabel(tr("Power min:")), powerMinSlider);

    scalesCheckBox = new QCheckBox(controlDockWidget);                                // time scales checkbox
    scalesCheckBox->setToolTip(tr("Display time and frequency scales on the spectrogram."));
    scalesCheckBox->setCheckState(Qt::Checked);
    plotSettingsLayout->addRow(new QLabel(tr("Scales:")), scalesCheckBox);

    // time selection group, a vertical form for timing details inside a checkable group box
    cursorsGroup = new QGroupBox(tr("Timing analysis"));                              // group with a title
    cursorsGroup->setCheckable(true);                                                 // that's checkable
    //TODO: the tooltip should be on the actual checkbox, not the whole group
    cursorsGroup->setToolTip(tr("Position cursors on the plot to discover timing information about the signal."));
    QFormLayout *cursorsLayout = new QFormLayout;                                     // with a form layout
    cursorsGroup->setLayout(cursorsLayout);

    cursorSymbolsSpinBox = new QSpinBox();                                            // symbol count
    cursorSymbolsSpinBox->setMinimum(1);
    cursorSymbolsSpinBox->setMaximum(9999);
    cursorSymbolsSpinBox->setToolTip(tr("Between 1 and 9999. This many symbols are contained between the cursors."));
    cursorsLayout->addRow(new QLabel(tr("Symbols:")), cursorSymbolsSpinBox);

    rateLabel = new QLabel();                                                         // rate
    rateLabel->setToolTip(tr("1 / Time between the cursors. (Hz)"));
    cursorsLayout->addRow(new QLabel(tr("Rate:")), rateLabel);

    periodLabel = new QLabel();                                                       // period
    periodLabel->setToolTip(tr("Number of seconds between the timing cursors."));
    cursorsLayout->addRow(new QLabel(tr("Period:")), periodLabel);

    symbolRateLabel = new QLabel();                                                   // symbol rate
    symbolRateLabel->setToolTip(tr("Number of symbols / Amount of time selected (Hz)"));
    cursorsLayout->addRow(new QLabel(tr("Symbol rate:")), symbolRateLabel);

    symbolPeriodLabel = new QLabel();                                                 // symbol period
    symbolPeriodLabel->setToolTip(tr("Symbol period tooltip"));
    cursorsLayout->addRow(new QLabel(tr("Symbol period:")), symbolPeriodLabel);
    symbolPeriodLabel->setToolTip(tr("Selected time / Number of symbols"));

    // add the groups to the dock layout
    controlDockLayout->addWidget(fileGroup);
    controlDockLayout->addWidget(plotSettingsGroup);
    controlDockLayout->addWidget(cursorsGroup);

    setWidget(controlDockWidget);

    connect(fileOpenButton, SIGNAL(clicked()), this, SLOT(fileOpenButtonClicked()));
    connect(fftSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(fftOrZoomChanged(int)));
    connect(zoomLevelSlider, SIGNAL(valueChanged(int)), this, SLOT(fftOrZoomChanged(int)));
    connect(cursorsGroup, SIGNAL(toggled(bool)), this, SLOT(cursorsStateChanged(bool)));
}

void SpectrogramControls::clearCursorLabels()
{
    periodLabel->setText("");
    rateLabel->setText("");
    symbolPeriodLabel->setText("");
    symbolRateLabel->setText("");
}

void SpectrogramControls::cursorsStateChanged(bool state)
{
    if (state == false) {
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
    cursorsGroup->setChecked(false);
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
    if (cursorsGroup->isChecked()){
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
