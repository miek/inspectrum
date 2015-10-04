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

#include "fft.h"
#include <fftw3.h>
#include <memory>

class InputSource
{
private:
    FILE *m_file;
    off_t m_file_size;
    off_t sampleCount;
    fftwf_complex *m_data;

public:
    InputSource(const char *filename);
    ~InputSource();

    bool getSamples(fftwf_complex *dest, off_t start, int length);
    off_t getSampleCount() { return sampleCount; };
};
