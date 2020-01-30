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

#pragma once

#include <QMainWindow>
#include <QScrollArea>
#include "spectrogramcontrols.h"
#include "plotview.h"

class MainWindow : public QMainWindow, Subscriber
{
    Q_OBJECT

public:
    MainWindow();
    void changeSampleRate(double rate);

public slots:
    void openFile(QString fileName);
    void setSampleRate(QString rate);
    void setSampleRate(double rate);
    void setFormat(QString fmt);
    void invalidateEvent() override;

private:
    SpectrogramControls *dock;
    PlotView *plots;
    InputSource *input;
    Tuner *tuner;
};
