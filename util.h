/*
 *  Copyright (C) 2016, Mike Walters <mike@flomp.net>
 *  Copyright (C) 2016, Jared Boone, ShareBrained Technology, Inc.
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
#include <algorithm>

template <class T> const T& clamp (const T& value, const T& min, const T& max)
{
    return std::min(max, std::max(min, value));
}

template<class T>
struct range_t {
    const T minimum;
    const T maximum;

    const T length() {
        return maximum - minimum;
    }

    const T& clip(const T& value) const {
        return clamp(value, minimum, maximum);
    }

    void reset_if_outside(T& value, const T& reset_value) const {
        if( (value < minimum ) ||
            (value > maximum ) ) {
            value = reset_value;
        }
    }

    bool below_range(const T& value) const {
        return value < minimum;
    }

    bool contains(const T& value) const {
        // TODO: Subtle gotcha here! Range test doesn't include maximum!
        return (value >= minimum) && (value < maximum);
    }

    bool out_of_range(const T& value) const {
        // TODO: Subtle gotcha here! Range test in contains() doesn't include maximum!
        return !contains(value);
    }
};
