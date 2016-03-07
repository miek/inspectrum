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

#include "memory_source_impl.h"
#include <gnuradio/io_signature.h>
#include <stdexcept>

namespace gr {
  namespace blocks {

    memory_source::sptr
    memory_source::make(size_t itemsize)
    {
      return gnuradio::get_initial_sptr
        (new memory_source_impl(itemsize));
    }

    memory_source_impl::memory_source_impl(size_t itemsize)
      : sync_block("memory_source",
                      io_signature::make(0, 0, 0),
                      io_signature::make(1, 1, itemsize)),
        d_itemsize(itemsize)
    {
    }

    memory_source_impl::~memory_source_impl()
    {
    }

    void
    memory_source_impl::set_source(void *source, size_t length)
    {
      d_source = source;
      d_length = length;
      d_ptr = 0;
    }

    int
    memory_source_impl::work(int noutput_items,
                         gr_vector_const_void_star &input_items,
                         gr_vector_void_star &output_items)
    {
      char *outbuf = (char*)output_items[0];
      long nwritten = 0;

      if(!d_source)
        return noutput_items;

      nwritten = std::min((long)(d_length - d_ptr), (long)noutput_items);

      if (nwritten >= 0) {
        memcpy(outbuf, (char*)d_source + d_ptr * d_itemsize, nwritten * d_itemsize);
      }
      d_ptr += nwritten;

      return (nwritten == 0) ? -1 : nwritten;
    }

  } /* namespace blocks */
} /* namespace gr */
