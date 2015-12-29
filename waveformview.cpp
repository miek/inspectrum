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
#include <gnuradio/top_block.h>
#include <gnuradio/blocks/multiply_const_cc.h>
#include "grsamplebuffer.h"
#include "memory_sink.h"
#include "memory_source.h"

WaveformView::WaveformView()
{
    for (int i = 0; i < 128; i++) {
        colormap[i] = qRgb(i/2, i*1.5, i*1.5);
    }
    for (int i = 128; i < 256; i++) {
        colormap[i] = qRgb(64 + (i-128)*1.5, 192+(i-128)/2, 192+(i-128)/2);
    }
}

void WaveformView::inputSourceChanged(SampleSource *src)
{
    gr::top_block_sptr tb = gr::make_top_block("multiply");
    auto mem_source = gr::blocks::memory_source::make(8);
    auto mem_sink = gr::blocks::memory_sink::make(8);
    auto multiply = gr::blocks::multiply_const_cc::make(20);
    tb->connect(mem_source, 0, multiply, 0);
    tb->connect(multiply, 0, mem_sink, 0);
    sampleSource = new GRSampleBuffer(src, tb, mem_source, mem_sink);
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
    if (sampleSource == nullptr) return;
    if (lastSample - firstSample <= 0) return;

    QRect rect = QRect(0, 0, width(), height());
    QPainter painter(this);
    painter.fillRect(rect, Qt::black);

    off_t length = lastSample - firstSample;
    std::complex<float> *samples = new std::complex<float>[length];
    sampleSource->getSamples(samples, firstSample, length);

    painter.setPen(Qt::red);
    plot(&painter, reinterpret_cast<float*>(samples), length, 2);
    painter.setPen(Qt::blue);
    plot(&painter, reinterpret_cast<float*>(samples)+1, length, 2);

    delete[] samples;
}

void WaveformView::plot(QPainter *painter, float *samples, off_t count, int step = 1)
{
    QRect rect = QRect(0, 0, width(), height());
    int xprev = 0;
    int yprev = 0;
    for (off_t i = 0; i < count; i++) {
        float sample = samples[i*step];
        int x = (float)i / count * rect.width();
        int y = (sample * rect.height()/2) + rect.height()/2;

        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x >= rect.width()-1) x = rect.width()-2;
        if (y >= rect.height()-1) y = rect.height()-2;

        painter->drawLine(xprev, yprev, x, y);
        xprev = x;
        yprev = y;
    }
}