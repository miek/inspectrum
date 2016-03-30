# inspectrum
inspectrum is a tool for analysing captured signals, primarily from software-defined radio receivers.

![inspectrum screenshot](/screenshot.jpg)

## Try it
### Prerequisites

 * boost >=1.35
 * gnuradio 3.7.x
 * qt5
 * fftw 3.x
 * cmake
 * pkg-config

### Building on Debian-based distros

    sudo apt-get install qt5-default libfftw3-dev cmake pkg-config
    mkdir build
    cd build
    cmake ..
    make
    sudo make install


## Building on OSX - Homebrew

    brew install qt5 fftw cmake pkg-config
    mkdir build
    cd build
    CMAKE_PREFIX_PATH=$(brew --prefix qt5)/lib/cmake cmake ..
    make install

## Building on OSX - Macports

    sudo port install fftw-3-single cmake pkgconfig qt5
    mkdir build
    cd build
    cmake ..
    make
    sudo make install


### Run

    ./inspectrum

## Input
inspectrum supports the following file types:
 * `*.cf32`, `*.cfile` - Complex 32-bit floating point samples (GNURadio, osmocom_fft)
 * `*.cs16` - Complex 16-bit signed integer samples (BladeRF)
 * `*.cs8` - Complex 8-bit signed integer samples (HackRF)
 * `*.cu8` - Complex 8-bit unsigned integer samples (RTL-SDR)

If an unknown file extension is loaded, inspectrum will default to `*.cf32`.

## Features
 * Spectrogram with zoom/pan
 * Large (multi-gigabyte) file support

## Goals
 * Be fast and light
 * Make as many settings as possible work in realtime. I want this to be a useful tool for visually learning about DSP by tweaking around with things like FFT settings, filter widths or clock synchronisation.
 * Support large files - as large as possible, but at least several GBs.
 * Future features: filtering, demodulation, clock sync. Probably based on GNURadio.
 
## Contact
 * #inspectrum on freenode IRC
