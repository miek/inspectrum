# inspectrum
inspectrum is a tool for analysing captured signals, primarily from software-defined radio receivers.

![inspectrum screenshot](/screenshot.jpg)

## Try it
### Prerequisites

 * cmake
 * fftw 3.x
 * [liquid-dsp](https://github.com/jgaeddert/liquid-dsp) >= v1.3.0
 * pkg-config
 * qt5

### Build instructions

Build instructions can be found here: https://github.com/miek/inspectrum/wiki/Build

### Run

    ./inspectrum [filename]

## Input
inspectrum supports the following file types:
 * `*.cf32`, `*.cfile` - Complex 32-bit floating point samples (GNURadio, osmocom_fft)
 * `*.cs16` - Complex 16-bit signed integer samples (BladeRF)
 * `*.cs8` - Complex 8-bit signed integer samples (HackRF)
 * `*.cu8` - Complex 8-bit unsigned integer samples (RTL-SDR)

If an unknown file extension is loaded, inspectrum will default to `*.cf32`.

## Features
 * Large (100GB+) file support
 * Spectrogram with zoom/pan
 * Plots of amplitude, frequency, phase and IQ samples
 * Cursors for measuring period, symbol rate and extracting symbols
 * Export of selected time period, filtered samples and demodulated data
 
## Contact
 * #inspectrum on freenode IRC
