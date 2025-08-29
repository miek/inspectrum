set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

# For OSX - don't clear RPATH on install
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if (MSVC)
    #force std::complex<> typedefs in liquiddsp
    add_definitions(-D_LIBCPP_COMPLEX)

    #enable math definitions in math.h
    add_definitions(-D_USE_MATH_DEFINES)

    #build a graphical application without the console
    option(BUILD_WIN32 "Build win32 app, false for console" TRUE)
    if (BUILD_WIN32)
        set(EXE_ARGS WIN32)
        set(CMAKE_EXE_LINKER_FLAGS "/entry:mainCRTStartup ${CMAKE_EXE_LINKER_FLAGS}")
    endif (BUILD_WIN32)
endif (MSVC)

if (NOT CMAKE_CXX_FLAGS)
    set(CMAKE_CXX_FLAGS "-O2")
endif (NOT CMAKE_CXX_FLAGS)

# This only works in cmake >3.1
set(CMAKE_CXX_STANDARD 14)

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
    symbolprogoutput.cpp
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
    ${FFTW_INCLUDES}
    ${LIQUID_INCLUDES}
)

add_executable(inspectrum ${EXE_ARGS} ${inspectrum_sources})

target_link_libraries(inspectrum
    Qt5::Core Qt5::Widgets Qt5::Concurrent
    ${FFTW_LIBRARIES}
    ${LIQUID_LIBRARIES}
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
