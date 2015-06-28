CONFIG += c++11
QT += widgets
TARGET = inspectrum
TEMPLATE = app

LIBS += -lfftw3f
SOURCES += main.cpp inputsource.cpp
