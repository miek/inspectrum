cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)
project(inspectrum CXX)
enable_testing()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

list(APPEND inspectrum_sources 
	main.cpp
	fft.cpp
	mainwindow.cpp
	inputsource.cpp
	spectrogram.cpp
	spectrogramcontrols.cpp
)

INCLUDE(FindPkgConfig)

find_package(Qt5Widgets)
pkg_check_modules(FFTW REQUIRED fftw3f)
include_directories(${QT_INCLUDES} ${FFTW_INCLUDEDIR})

add_executable(inspectrum ${inspectrum_sources})
qt5_use_modules(inspectrum Widgets)
target_link_libraries(inspectrum ${QT_LIBRARIES} ${FFTW_LIBRARIES})
set_property(TARGET inspectrum PROPERTY CXX_STANDARD 11)
