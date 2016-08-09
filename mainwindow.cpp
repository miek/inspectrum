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

#include <QtWidgets>
#include <QRubberBand>

#include "mainwindow.h"
#include "util.h"

MainWindow::MainWindow()
{
    setWindowTitle(tr("inspectrum"));

    dock = new SpectrogramControls(tr("Controls"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    input = new InputSource();

    plots = new PlotView(input);
    setCentralWidget(plots);

    // Connect dock inputs
    connect(dock, SIGNAL(openFile(QString)), this, SLOT(openFile(QString)));
    connect(dock->sampleRate, SIGNAL(textChanged(QString)), this, SLOT(setSampleRate(QString)));
    connect(dock, SIGNAL(fftOrZoomChanged(int, int)), plots, SLOT(setFFTAndZoom(int, int)));
    connect(dock->powerMaxSlider, SIGNAL(valueChanged(int)), plots, SLOT(setPowerMax(int)));
    connect(dock->powerMinSlider, SIGNAL(valueChanged(int)), plots, SLOT(setPowerMin(int)));
    connect(dock->cursorsCheckBox, &QCheckBox::stateChanged, plots, &PlotView::enableCursors);
    connect(dock->timeScaleCheckBox, &QCheckBox::stateChanged, plots, &PlotView::enableTimeScale);
    connect(dock->cursorSymbolsSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), plots, &PlotView::setCursorSegments);

    // Connect dock outputs
    connect(plots, SIGNAL(timeSelectionChanged(float)), dock, SLOT(timeSelectionChanged(float)));
    connect(plots, SIGNAL(zoomIn()), dock, SLOT(zoomIn()));
    connect(plots, SIGNAL(zoomOut()), dock, SLOT(zoomOut()));

    // Set defaults after making connections so everything is in sync
    dock->setDefaults();
}

void MainWindow::openFile(QString fileName)
{
    QString title="%1: %2";
    this->setWindowTitle(title.arg(QApplication::applicationName(),fileName.section('/',-1,-1)));
    input->openFile(fileName.toUtf8().constData());
}

void MainWindow::setSampleRate(QString rate)
{
    input->setSampleRate(rate.toInt());
    plots->setSampleRate(rate.toInt());
}

void MainWindow::setSampleRate(int rate)
{
    dock->sampleRate->setText(QString::number(rate));
}
