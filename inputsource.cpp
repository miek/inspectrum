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

InputSource::InputSource()
{
}

InputSource::~InputSource()
{
    cleanup();
}

void InputSource::cleanup()
{
    if (mmapData != nullptr) {
        munmap(mmapData, fileSize);
        mmapData = nullptr;
        fileSize = 0;
    }

    if (inputFile != nullptr) {
        fclose(inputFile);
        inputFile = nullptr;
    }
}

void InputSource::openFile(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (file == nullptr)
        throw std::runtime_error("Error opening file");

    struct stat sb;
    if (fstat(fileno(file), &sb) != 0)
        throw std::runtime_error("Error fstating file");
    off_t size = sb.st_size;
    sampleCount = size / sizeof(std::complex<float>);

    auto data = (std::complex<float>*)mmap(NULL, size, PROT_READ, MAP_SHARED, fileno(file), 0);
    if (data == nullptr)
        throw std::runtime_error("Error mmapping file");

    cleanup();

    inputFile = file;
    fileSize = size;
    mmapData = data;

    invalidate();
}

std::unique_ptr<std::complex<float>[]> InputSource::getSamples(off_t start, off_t length)
{
    if (inputFile == nullptr)
        return nullptr;

    if (mmapData == nullptr)
        return nullptr;

    if(start < 0 || length < 0)
        return nullptr;

    if (start + length >= sampleCount)
        return nullptr;

    std::unique_ptr<std::complex<float>[]> dest(new std::complex<float>[length]);
    memcpy(dest.get(), &mmapData[start], length * sizeof(std::complex<float>));
    return dest;
}
