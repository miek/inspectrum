/*
 *  Copyright (C) 2016, Mike Walters <mike@flomp.net>
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

#include <gnuradio/top_block.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include "grsamplebuffer.h"
#include "memory_sink.h"
#include "memory_source.h"
#include "traceplot.h"

#include "plots.h"

Plot* Plots::samplePlot(std::shared_ptr<AbstractSampleSource> source)
{
    return new TracePlot(source);
}

Plot* Plots::frequencyPlot(std::shared_ptr<AbstractSampleSource> source)
{
    gr::top_block_sptr quad_demod_tb = gr::make_top_block("quad_demod");
    auto quad_demod_mem_source = gr::blocks::memory_source::make(8);
    auto quad_demod_mem_sink = gr::blocks::memory_sink::make(4);
    auto quad_demod = gr::analog::quadrature_demod_cf::make(5);
    quad_demod_tb->connect(quad_demod_mem_source, 0, quad_demod, 0);
    quad_demod_tb->connect(quad_demod, 0, quad_demod_mem_sink, 0);

    return new TracePlot(
        std::make_shared<GRSampleBuffer<std::complex<float>, float>>(
            std::dynamic_pointer_cast<SampleSource<std::complex<float>>>(source).get(), quad_demod_tb, quad_demod_mem_source, quad_demod_mem_sink
        )
    );
}
