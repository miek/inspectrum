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

#include "memory_sink_impl.h"
#include <gnuradio/io_signature.h>
#include <stdexcept>

namespace gr {
  namespace blocks {

    memory_sink::sptr
    memory_sink::make(size_t itemsize)
    {
      return gnuradio::get_initial_sptr
        (new memory_sink_impl(itemsize));
    }

    memory_sink_impl::memory_sink_impl(size_t itemsize)
      : sync_block("memory_sink",
                      io_signature::make(1, 1, itemsize),
                      io_signature::make(0, 0, 0)),
        d_itemsize(itemsize)
    {
    }

    memory_sink_impl::~memory_sink_impl()
    {
    }

    void
    memory_sink_impl::set_sink(void *sink, size_t length)
    {
      d_sink = sink;
      d_length = length;
      d_ptr = 0;
    }

    int
    memory_sink_impl::work(int noutput_items,
                         gr_vector_const_void_star &input_items,
                         gr_vector_void_star &output_items)
    {
      char *inbuf = (char*)input_items[0];
      long nwritten = 0;

      if(!d_sink)
        return noutput_items;

      nwritten = std::min((long)(d_length - d_ptr), (long)noutput_items);
      if (nwritten >= 0) {
        memcpy((char*)d_sink + d_ptr * d_itemsize, inbuf, nwritten * d_itemsize);
      }
      d_ptr += nwritten;

      return (nwritten == 0) ? -1 : nwritten;
    }

  } /* namespace blocks */
} /* namespace gr */
