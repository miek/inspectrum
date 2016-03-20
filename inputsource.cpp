/*
 *  Copyright (C) 2015, Mike Walters <mike@flomp.net>
 *  Copyright (C) 2015, Jared Boone <jared@sharebrained.com>
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
#include <algorithm>

#include <QFileInfo>

class ComplexF32SampleAdapter : public SampleAdapter {
public:
    size_t sampleSize() override {
        return sizeof(std::complex<float>);
    }

    void copyRange(const void* const src, off_t start, off_t length, std::complex<float>* const dest) override {
        auto s = reinterpret_cast<const std::complex<float>*>(src);
        std::copy(&s[start], &s[start + length], dest);
    }
};

class ComplexS16SampleAdapter : public SampleAdapter {
public:
    size_t sampleSize() override {
        return sizeof(std::complex<int16_t>);
    }

    void copyRange(const void* const src, off_t start, off_t length, std::complex<float>* const dest) override {
        auto s = reinterpret_cast<const std::complex<int16_t>*>(src);
        std::transform(&s[start], &s[start + length], dest,
            [](const std::complex<int16_t>& v) -> std::complex<float> {
                const float k = 1.0f / 32768.0f;
                return { v.real() * k, v.imag() * k };
            }
        );
    }
};

class ComplexS8SampleAdapter : public SampleAdapter {
public:
    size_t sampleSize() override {
        return sizeof(std::complex<int8_t>);
    }
    
    void copyRange(const void* const src, off_t start, off_t length, std::complex<float>* const dest) override {
        auto s = reinterpret_cast<const std::complex<int8_t>*>(src);
        std::transform(&s[start], &s[start + length], dest,
            [](const std::complex<int8_t>& v) -> std::complex<float> {
                const float k = 1.0f / 128.0f;
                return { v.real() * k, v.imag() * k };
            }
        );
    }
};

class ComplexU8SampleAdapter : public SampleAdapter {
public:
    size_t sampleSize() override {
        return sizeof(std::complex<uint8_t>);
    }
    
    void copyRange(const void* const src, off_t start, off_t length, std::complex<float>* const dest) override {
        auto s = reinterpret_cast<const std::complex<uint8_t>*>(src);
        std::transform(&s[start], &s[start + length], dest,
            [](const std::complex<uint8_t>& v) -> std::complex<float> {
                const float k = 1.0f / 128.0f;
                return { v.real() * k - 1.0f, v.imag() * k - 1.0f };
            }
        );
    }
};

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
    QFileInfo fileInfo(filename);
    const auto suffix = fileInfo.suffix();
    if( (suffix == "cfile") || (suffix == "cf32") ) {
        sampleAdapter = std::unique_ptr<SampleAdapter>(new ComplexF32SampleAdapter());
    }
    else if( suffix == "cs16" ) {
        sampleAdapter = std::unique_ptr<SampleAdapter>(new ComplexS16SampleAdapter());
    }
    else if( suffix == "cs8" ) {
        sampleAdapter = std::unique_ptr<SampleAdapter>(new ComplexS8SampleAdapter());
    }
    else if( suffix == "cu8" ) {
        sampleAdapter = std::unique_ptr<SampleAdapter>(new ComplexU8SampleAdapter());
    }
    else {
        throw std::runtime_error("Invalid file extension");
    }

    FILE *file = fopen(filename, "rb");
    if (file == nullptr)
        throw std::runtime_error("Error opening file");

    struct stat sb;
    if (fstat(fileno(file), &sb) != 0)
        throw std::runtime_error("Error fstating file");
    off_t size = sb.st_size;
    sampleCount = size / sampleAdapter->sampleSize();

    auto data = mmap(NULL, size, PROT_READ, MAP_SHARED, fileno(file), 0);
    if (data == nullptr)
        throw std::runtime_error("Error mmapping file");

    cleanup();

    inputFile = file;
    fileSize = size;
    mmapData = data;

    invalidate();
}

void InputSource::setSampleRate(off_t rate)
{
    sampleRate = rate;
    invalidate();
}

off_t InputSource::rate()
{
    return sampleRate;
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
    sampleAdapter->copyRange(mmapData, start, length, dest.get());

    return dest;
}
