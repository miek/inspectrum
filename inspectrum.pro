CONFIG += c++11
QT += widgets
TARGET = inspectrum
TEMPLATE = app

LIBS += -lfftw3f
HEADERS += mainwindow.h fft.h inputsource.h spectrogram.h spectrogramcontrols.h
SOURCES += main.cpp fft.cpp mainwindow.cpp inputsource.cpp spectrogram.cpp spectrogramcontrols.cpp
