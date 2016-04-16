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

#pragma once

#include <functional>
#include <memory>
#include "plot.h"
#include "samplesource.h"

class Plots
{

public:
    static std::multimap<std::type_index, std::function<Plot*(std::shared_ptr<AbstractSampleSource>)>> plots;

    static Plot* samplePlot(std::shared_ptr<AbstractSampleSource> source);
    static Plot* frequencyPlot(std::shared_ptr<AbstractSampleSource> source);

    static class _init
    {
    public:
        _init() {
            plots.emplace(typeid(std::complex<float>), samplePlot);
            plots.emplace(typeid(std::complex<float>), frequencyPlot);
        };
    } _initializer;
};