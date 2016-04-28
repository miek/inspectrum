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

#ifndef INCLUDED_GR_MEMORY_SOURCE_H
#define INCLUDED_GR_MEMORY_SOURCE_H

#include <gnuradio/blocks/api.h>
#include <gnuradio/sync_block.h>

namespace gr
{
namespace blocks
{
class memory_source : virtual public sync_block
{
public:
    typedef boost::shared_ptr<memory_source> sptr;

    static sptr make(size_t itemsize);

    virtual void set_source(void *source, size_t length, uint64_t sampleid) = 0;
};

} /* namespace blocks */
} /* namespace gr */

#endif /* INCLUDED_GR_MEMORY_SOURCE_H */