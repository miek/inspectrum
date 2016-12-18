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

#include <amplitudedemod.h>
#include <frequencydemod.h>
#include "threshold.h"
#include "traceplot.h"

#include "plots.h"

std::multimap<std::type_index, PlotInfo> Plots::plots;
Plots::_init Plots::_initializer;

Plot* Plots::samplePlot(std::shared_ptr<AbstractSampleSource> source)
{
    return new TracePlot(source);
}

Plot* Plots::amplitudePlot(std::shared_ptr<AbstractSampleSource> source)
{
    typedef SampleSource<std::complex<float>> Source;
    std::shared_ptr<Source> concrete = std::dynamic_pointer_cast<Source>(source);
    return new TracePlot( std::make_shared<AmplitudeDemod>(concrete) );
}

Plot* Plots::frequencyPlot(std::shared_ptr<AbstractSampleSource> source)
{
    typedef SampleSource<std::complex<float>> Source;
    std::shared_ptr<Source> concrete = std::dynamic_pointer_cast<Source>(source);
    return new TracePlot( std::make_shared<FrequencyDemod>( concrete ) );
}

Plot* Plots::thresholdPlot(std::shared_ptr<AbstractSampleSource> source)
{
    typedef SampleSource<float> Source;
    std::shared_ptr<Source> concrete= std::dynamic_pointer_cast<Source>(source);
    return new TracePlot( std::make_shared<Threshold>( concrete ) );
}
