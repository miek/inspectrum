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

#include <QMutex>
#include <complex>
#include <memory>
#include "samplesource.h"

template <typename Tin, typename Tout>
class SampleBuffer : public SampleSource<Tout>, public Subscriber
{
private:
    SampleSource<Tin> *src;
    QMutex mutex;

public:
    SampleBuffer(SampleSource<Tin> *src);
    ~SampleBuffer();
    void invalidateEvent();
    virtual std::unique_ptr<Tout[]> getSamples(off_t start, off_t length);
    virtual void work(void *input, void *output, int count) = 0;
    virtual off_t count() {
        return src->count();
    };
    off_t rate() {
        return src->rate();
    };
};
