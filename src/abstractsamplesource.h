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
#include <set>
#include <typeindex>
#include "subscriber.h"

class AbstractSampleSource
{

public:
    virtual ~AbstractSampleSource() {};
    virtual std::type_index sampleType() = 0;
    void subscribe(Subscriber *subscriber);
    int subscriberCount();
    void unsubscribe(Subscriber *subscriber);

protected:
    virtual void invalidate();

private:
    std::set<Subscriber*> subscribers;
};
