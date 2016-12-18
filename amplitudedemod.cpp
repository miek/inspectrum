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

#include "amplitudedemod.h"

AmplitudeDemod::AmplitudeDemod(std::shared_ptr<SampleSource<std::complex<float>>> src) : SampleBuffer(src)
{

}

void AmplitudeDemod::work(void *input, void *output, int count, off_t sampleid)
{
    auto in = static_cast<std::complex<float>*>(input);
    auto out = static_cast<float*>(output);
    std::transform(in, in + count, out,
                   [](std::complex<float> s) { return std::norm(s) * 2.0f - 1.0f; });
}
