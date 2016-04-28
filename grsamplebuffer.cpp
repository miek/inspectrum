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

#include "grsamplebuffer.h"

template<typename Tin, typename Tout>
void GRSampleBuffer<Tin, Tout>::work(void *input, void *output, int length, off_t sampleid)
{
    mem_source->set_source(input, length, sampleid);
    mem_sink->set_sink(output, length);
    tb->run();
}

template class GRSampleBuffer<std::complex<float>, std::complex<float>>;
template class GRSampleBuffer<std::complex<float>, float>;
template class GRSampleBuffer<float, float>;
