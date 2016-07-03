# inspectrum
inspectrum is a tool for analysing captured signals, primarily from software-defined radio receivers.

![inspectrum screenshot](/screenshot.jpg)

## Try it
### Prerequisites

 * cmake
 * fftw 3.x
 * [liquid-dsp](https://github.com/jgaeddert/liquid-dsp)
 * pkg-config
 * qt5

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
 * Large (multi-gigabyte) file support
 * Spectrogram with zoom/pan
 * Plots of amplitude, frequency and IQ samples
 
## Contact
 * #inspectrum on freenode IRC
