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
 *ha
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCLUDED_GR_MEMORY_SOURCE_IMPL_H
#define INCLUDED_GR_MEMORY_SOURCE_IMPL_H

#include "memory_source.h"

namespace gr
{
namespace blocks
{

class memory_source_impl : public memory_source
{
private:
    size_t d_itemsize;
    void *d_source;
    size_t d_length;
    size_t d_ptr = 0;
    uint64_t d_sampleid;

public:
    memory_source_impl(size_t itemsize);
    ~memory_source_impl();

    void set_source(void *source, size_t length, uint64_t sampleid);

    int work(int noutput_items,
             gr_vector_const_void_star &input_items,
             gr_vector_void_star &output_items);
};

} /* namespace blocks */
} /* namespace gr */

#endif /* INCLUDED_GR_MEMORY_SOURCE_IMPL_H */