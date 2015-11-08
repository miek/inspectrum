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

#include "inputsource.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <stdexcept>

InputSource::InputSource(const char *filename) {
    m_file = fopen(filename, "rb");
    if (m_file == nullptr)
        throw std::runtime_error("Error opening file");

    struct stat sb;
    if (fstat(fileno(m_file), &sb) != 0)
        throw std::runtime_error("Error fstating file");
    m_file_size = sb.st_size;
    sampleCount = m_file_size / sizeof(std::complex<float>);

    m_data = (std::complex<float>*)mmap(NULL, m_file_size, PROT_READ, MAP_SHARED, fileno(m_file), 0);
    if (m_data == 0)
        throw std::runtime_error("Error mmapping file");
}

InputSource::~InputSource() {
    munmap(m_data, m_file_size);
    fclose(m_file);
}

bool InputSource::getSamples(std::complex<float> *dest, off_t start, off_t length)
{
    if(start < 0 || length < 0)
        return false;

    if (start + length >= sampleCount)
        return false;

    memcpy(dest, &m_data[start], length * sizeof(std::complex<float>));
    return true;
}
