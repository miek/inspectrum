#!/usr/bin/env python
import os
import sys
import getopt
import numpy
import scipy.signal
import subprocess

if __name__ == "__main__":
    options, remainder = getopt.getopt(sys.argv[1:], 'v', [
                                                            'verbose',
                                                            ])
    verbose = False

    for opt, arg in options:
        if opt in ('-v', '--verbose'):
            verbose = True

    if len(remainder)!=7:
        print >> sys.stderr, "ERR: need 7 arguments"

    filename=    remainder[0]
    sample_rate= long(remainder[1])
    start_sample=long(remainder[2])
    end_sample=  long(remainder[3])
    min_freq=    long(remainder[4])
    max_freq=    long(remainder[5])
    center_freq= long(remainder[6])

    bps=8 # Byte per sample
    outfile="/tmp/sample.bin"

    freq_offset=(max_freq+min_freq)/2 - center_freq
    freq_window=(max_freq-min_freq)

    if verbose:
        print "Cutting",start_sample,"-",end_sample,"from",filename,"..."
        print "Center is",center_freq,"sample_rate=",sample_rate
    
    f = open(filename, "rb")
    f.seek(start_sample*bps, os.SEEK_SET)
    signal = numpy.fromfile(f, dtype=numpy.complex64, count=end_sample-start_sample)  # read the data into numpy

    # shift marked signal to center freq
    if verbose:
        print "Shifting signal by",freq_offset,"Hz"
    shift = numpy.exp(complex(0,-1)*numpy.arange(len(signal))*2*numpy.pi*freq_offset/float(sample_rate))
    signal = signal*shift

    # Filter unwanted part
    if verbose:
        print "Filtering ",freq_window,"Hz"
    low_pass = scipy.signal.firwin(401, freq_window/2, nyq=sample_rate)
    signal = numpy.convolve(signal, low_pass, mode='same')

    # Decimate a bit (if possible)
    decimate = int(sample_rate/freq_window)
    if decimate > 4:
        decimate=4
    if verbose:
        print "Decimating by ",decimate
    signal=signal[::decimate]

    # Write file
    if type(signal)!=numpy.complex64:
        signal=numpy.asarray(signal,dtype=numpy.complex64)
    signal.tofile(outfile)

    # Run inspectrum to display result
    command=["inspectrum","-c",str(center_freq+freq_offset),"-r",str(sample_rate/decimate),outfile]
    if verbose:
        print "Running command:",command
    subprocess.call(command)
