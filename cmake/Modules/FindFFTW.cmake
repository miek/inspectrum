# - Find FFTW
# Find the native FFTW includes and library
#
#  FFTW_INCLUDES    - where to find fftw3.h
#  FFTW_LIBRARIES   - List of libraries when using FFTW.
#  FFTW_FOUND       - True if FFTW found.

if (FFTW_INCLUDES)
  # Already in cache, be silent
  set (FFTW_FIND_QUIETLY TRUE)
endif (FFTW_INCLUDES)

find_package(PkgConfig)
pkg_check_modules(PC_FFTW QUIET fftw3f)

find_path (FFTW_INCLUDES fftw3.h
    HINTS ${PC_FFTW_INCLUDEDIR}  ${PC_FFTW_INCLUDE_DIRS})

find_library (FFTW_LIBRARIES NAMES fftw3f
    HINTS ${PC_FFTW_LIBDIR} ${PC_FFTW_LIBRARY_DIRS})

# handle the QUIETLY and REQUIRED arguments and set FFTW_FOUND to TRUE if
# all listed variables are TRUE
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (FFTW DEFAULT_MSG FFTW_LIBRARIES FFTW_INCLUDES)

mark_as_advanced (FFTW_LIBRARIES FFTW_INCLUDES)
