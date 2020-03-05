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

#include <complex>
#include <memory>
#include "abstractsamplesource.h"

template<typename T>
class SampleSource : public AbstractSampleSource
{

public:
    virtual ~SampleSource() {};

    virtual std::unique_ptr<T[]> getSamples(size_t start, size_t length) = 0;
    virtual void invalidateEvent() { };
    virtual size_t count() = 0;
    virtual double rate() = 0;
    virtual float relativeBandwidth() = 0;
    std::type_index sampleType() override;
    virtual bool realSignal() { return false; };
};
