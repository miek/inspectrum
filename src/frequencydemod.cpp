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

#include "frequencydemod.h"
#include <liquid/liquid.h>
#include "util.h"
#include <QSettings>


FrequencyDemod::FrequencyDemod(std::shared_ptr<SampleSource<std::complex<float>>> src) : SampleBuffer(src)
{

}

void FrequencyDemod::work(void *input, void *output, int count, size_t sampleid)
{
	double power_window[10];
	unsigned int window_size = 10;
	unsigned int window_index = 0;
	double power_sum = 0.0f;
	double avg_power = 0.0f;

    auto in = static_cast<std::complex<float>*>(input);
    auto out = static_cast<float*>(output);
    freqdem fdem = freqdem_create(relativeBandwidth() / 2.0);

    QSettings settings;
    int sqval = settings.value("Squelch", 0).toInt();
    double squelch_threshold = pow(2, sqval+2); // 100.0 * settings.value("Squelch", 0).toInt();
    bool using_squelch = sqval ? true : false;

    if (using_squelch) {
		// Initialize power window
		for (unsigned int i = 0; i < window_size; i++) {
			power_window[i] = 0.0f;
		}
    }


    for (int i = 0; i < count; i++) {

		if (using_squelch) {
			double power = in[i].real() * in[i].real()
								+ in[i].imag() * in[i].imag();
			// Update power averaging window
			power_sum -= power_window[window_index]; // Subtract oldest power
			power_window[window_index] = power;      // Add new power
			power_sum += power;                      // Update sum
			window_index = (window_index + 1) % window_size; // Circular buffer index
			// Compute average power
			avg_power = power_sum / window_size;
		}
		// Check if average power exceeds squelch threshold

         if ( (!using_squelch) || (avg_power >  squelch_threshold)) {
        	 freqdem_demodulate(fdem, in[i], &out[i]);
         } else {
        	 out[i] = 0;
         }
    }
    freqdem_destroy(fdem);
}
