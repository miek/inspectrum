CONFIG += c++11
QT += widgets
TARGET = inspectrum
TEMPLATE = app

LIBS += -lfftw3f
HEADERS += mainwindow.h inputsource.h spectrogram.h spectrogramcontrols.h
SOURCES += main.cpp mainwindow.cpp inputsource.cpp spectrogram.cpp spectrogramcontrols.cpp
