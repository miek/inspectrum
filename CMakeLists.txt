cmake_minimum_required(VERSION 2.8)
project(inspectrum CXX)
enable_testing()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

# For OSX - don't clear RPATH on install
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if (WIN32)
    find_library (MMAN mman)
    if(NOT(MMAN))
    message(FATAL_ERROR "please install mman-win32")
    else(NOT(MMAN))
    set (extraLibs  ${extraLibs} ${MMAN})
    endif(NOT(MMAN))
ENDIF (WIN32)

if (MSVC)
    find_path(MMAN_INCLUDE_DIR sys/mman.h)

    #force std::complex<> typedefs in liquiddsp
    add_definitions(-D_LIBCPP_COMPLEX)

    #enable math definitions in math.h
    add_definitions(-D_USE_MATH_DEFINES)

endif (MSVC)

if (NOT CMAKE_CXX_FLAGS)
    set(CMAKE_CXX_FLAGS "-O2")
endif (NOT CMAKE_CXX_FLAGS)

# This only works in cmake >3.1
#set_property(TARGET inspectrum PROPERTY CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

list(APPEND inspectrum_sources 
    abstractsamplesource.cpp
    amplitudedemod.cpp
    cursor.cpp
    cursors.cpp
    main.cpp
    fft.cpp
    frequencydemod.cpp
    mainwindow.cpp
    inputsource.cpp
    phasedemod.cpp
    plot.cpp
    plots.cpp
    plotview.cpp
    samplebuffer.cpp
    samplesource.cpp
    spectrogramcontrols.cpp
    spectrogramplot.cpp
    threshold.cpp
    traceplot.cpp
    tuner.cpp
    tunertransform.cpp
    util.cpp
)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(FFTW REQUIRED)
find_package(Liquid REQUIRED)

include_directories(
    ${MMAN_INCLUDE_DIR}
    ${QT_INCLUDES}
    ${FFTW_INCLUDES}
    ${LIQUID_INCLUDES}
)

add_executable(inspectrum ${inspectrum_sources})
qt5_use_modules(inspectrum Widgets Concurrent)

target_link_libraries(inspectrum
    ${QT_LIBRARIES}
    ${FFTW_LIBRARIES}
    ${LIQUID_LIBRARIES}
    ${extraLibs}
)
set(INSTALL_DEFAULT_BINDIR "bin" CACHE STRING "Appended to CMAKE_INSTALL_PREFIX")

install(TARGETS inspectrum RUNTIME DESTINATION ${INSTALL_DEFAULT_BINDIR})

# Create uninstall target
configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
@ONLY)

add_custom_target(uninstall
    ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)
