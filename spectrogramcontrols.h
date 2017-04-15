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

#include <QDockWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>

class SpectrogramControls : public QDockWidget
{
    Q_OBJECT

public:
    SpectrogramControls(const QString & title, QWidget * parent);
    void setDefaults();

signals:
    void fftSizeChanged(int fftSize);

public slots:
    void timeSelectionChanged(float time);

private slots:
    void fftSliderChanged(int value);
    void powerMinChanged(int value);
    void powerMaxChanged(int value);
    void cursorsStateChanged(int state);

private:
    QWidget *widget;
    QFormLayout *layout;
    void clearCursorLabels();

public:
    QPushButton *fileOpenButton;
    QLineEdit *sampleRate;
    QSlider *fftSizeSlider;
    QSlider *powerMaxSlider;
    QSlider *powerMinSlider;
    QCheckBox *cursorsCheckBox;
    QSpinBox *cursorSymbolsSpinBox;
    QLabel *rateLabel;
    QLabel *periodLabel;
    QLabel *symbolRateLabel;
    QLabel *symbolPeriodLabel;
    QCheckBox *scalesCheckBox;
};
