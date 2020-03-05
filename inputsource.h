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

#pragma once

#include <complex>
#include <QFile>
#include "samplesource.h"

class SampleAdapter {
public:
    virtual size_t sampleSize() = 0;
    virtual void copyRange(const void* const src, size_t start, size_t length, std::complex<float>* const dest) = 0;
};

class InputSource : public SampleSource<std::complex<float>>
{
private:
    QFile *inputFile = nullptr;
    size_t sampleCount = 0;
    double sampleRate = 0.0;
    uchar *mmapData = nullptr;
    std::unique_ptr<SampleAdapter> sampleAdapter;
    std::string _fmt;
    bool _realSignal = false;

public:
    InputSource();
    ~InputSource();
    void cleanup();
    void openFile(const char *filename);
    std::unique_ptr<std::complex<float>[]> getSamples(size_t start, size_t length);
    size_t count() {
        return sampleCount;
    };
    void setSampleRate(double rate);
    void setFormat(std::string fmt);
    double rate();
    bool realSignal() {
        return _realSignal;
    };
    float relativeBandwidth() {
        return 1;
    }
};
