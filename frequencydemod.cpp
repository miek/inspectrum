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

#include "frequencydemod.h"
#include <liquid/liquid.h>
#include "util.h"

FrequencyDemod::FrequencyDemod(std::shared_ptr<SampleSource<std::complex<float>>> src) : SampleBuffer(src)
{

}

void FrequencyDemod::work(void *input, void *output, int count, off_t sampleid)
{
    auto in = static_cast<std::complex<float>*>(input);
    auto out = static_cast<float*>(output);
    freqdem fdem = freqdem_create(0.05f);
    for (int i = 0; i < count; i++) {
        freqdem_demodulate(fdem, in[i], &out[i]);
    }
    freqdem_destroy(fdem);
}
