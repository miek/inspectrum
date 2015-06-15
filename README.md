# inspectrum
`inspectrum` is a tool for analysing captured signals, primarily from software-defined radio receivers.

## Status
Currently very early and rough, but usable.

## Try it
### Building on Debian-based distros

    sudo apt-get install libwxgtk3.0-dev libfftw3-dev
    git clone https://github.com/miek/inspectrum.git
    cd inspectrum
    make

### Run
Currently `inspectrum` can only read files with interleaved (complex) 32-bit floats, such as those produced by GNURadio or osmocom_fft.

    ./inspectrum <path-to-cfile>

## Features
 * Spectrogram with <s>zoom</s>/pan
 * Large (multi-gigabyte) file support

## Goals
 * Be fast and light
 * Make as many settings as possible work in realtime. I want this to be a useful tool for visually learning about DSP by tweaking around with things like FFT settings, filter widths or clock synchronisation.
 * Support large files - as large as possible, but at least several GBs.
 * Future features: filtering, demodulation, clock sync. Probably based on GNURadio.
 
## Author
 * Mike Walters - `miek` on freenode IRC (preferred) - <mike@flomp.net>
