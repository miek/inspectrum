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

void WaveformView::inputSourceChanged(AbstractSampleSource *src)
{
    sampleSources.clear();

    gr::top_block_sptr iq_tb = gr::make_top_block("multiply");
    auto iq_mem_source = gr::blocks::memory_source::make(8);
    auto iq_mem_sink = gr::blocks::memory_sink::make(8);
    auto multiply = gr::blocks::multiply_const_cc::make(20);
    iq_tb->connect(iq_mem_source, 0, multiply, 0);
    iq_tb->connect(multiply, 0, iq_mem_sink, 0);

    auto derived = dynamic_cast<SampleSource<std::complex<float>>*>(src);
    if (derived == nullptr)
        throw new std::runtime_error("SampleSource doesn't provide correct type for GRSampleBuffer");

    sampleSources.emplace_back(new GRSampleBuffer<std::complex<float>, std::complex<float>>(derived, iq_tb, iq_mem_source, iq_mem_sink));
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
    if (lastSample - firstSample <= 0) return;

    QRect rect = QRect(0, 0, width(), height());
    QPainter painter(this);
    painter.fillRect(rect, Qt::black);

    // Split space equally between waveforms for now
    int waveHeight = height() / sampleSources.size();
    int wave = 0;
    for (auto&& sampleSource : sampleSources) {
        QRect waveRect = QRect(0, wave * waveHeight, width(), waveHeight);
        off_t length = lastSample - firstSample;
        if (auto src = dynamic_cast<SampleSource<std::complex<float>>*>(sampleSource.get())) {
            auto samples = src->getSamples(firstSample, length);
            painter.setPen(Qt::red);
            plot(&painter, waveRect, reinterpret_cast<float*>(samples.get()), length, 2);
            painter.setPen(Qt::blue);
            plot(&painter, waveRect, reinterpret_cast<float*>(samples.get())+1, length, 2);
        } else if (auto src = dynamic_cast<SampleSource<float>*>(sampleSource.get())) {
            auto samples = src->getSamples(firstSample, length);
            painter.setPen(Qt::green);
            plot(&painter, waveRect, samples.get(), length, 1);
        }
        wave++;
    }
}

void WaveformView::plot(QPainter *painter, QRect &rect, float *samples, off_t count, int step = 1)
{
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

        painter->drawLine(xprev + rect.x(), yprev + rect.y(), x + rect.x(), y + rect.y());
        xprev = x;
        yprev = y;
    }
}