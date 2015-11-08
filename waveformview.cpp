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

#include "waveformview.h"
#include <QDebug>
#include <QPainter>

WaveformView::WaveformView()
{
    for (int i = 0; i < 128; i++) {
        colormap[i] = qRgb(i/2, i*1.5, i*1.5);
    }
    for (int i = 128; i < 256; i++) {
        colormap[i] = qRgb(64 + (i-128)*1.5, 192+(i-128)/2, 192+(i-128)/2);
    }
}

void WaveformView::inputSourceChanged(InputSource *input)
{
    inputSource = input;
    update();
}

void WaveformView::viewChanged(off_t firstSample, off_t lastSample)
{
    this->firstSample = firstSample;
    this->lastSample = lastSample;
    qDebug() << "viewChanged(" << firstSample << ", " << lastSample << ")";
    update();
}

void WaveformView::paintEvent(QPaintEvent *event)
{
    if (inputSource == nullptr) return;
    if (lastSample - firstSample <= 0) return;

    QRect rect = QRect(0, 0, width(), height());
    QPainter painter(this);
    painter.fillRect(rect, Qt::black);

    off_t length = lastSample - firstSample;
    std::complex<float> *samples = new std::complex<float>[length];
    inputSource->getSamples(samples, firstSample, length);
    for (int iq = 0; iq < 2; iq++) {
        switch (iq) {
            case 0:
                painter.setPen(Qt::red);
                break;
            case 1:
                painter.setPen(Qt::blue);
                break;
        }
        int xprev = 0;
        int yprev = 0;
        for (off_t i = 0; i < length; i++) {
            int x = (float)i / length * rect.width();
            int y = (samples[i].real() * 50 * rect.height()/2) + rect.height()/2;

            if (x < 0) x = 0;
            if (y < 0) y = 0;
            if (x >= rect.width()-1) x = rect.width()-2;
            if (y >= rect.height()-1) y = rect.height()-2;

            painter.drawLine(xprev, yprev, x, y);
            xprev = x;
            yprev = y;
        }
    }

    delete[] samples;
}