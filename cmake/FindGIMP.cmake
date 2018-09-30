# Finds GIMP libraries.
#
# This module defines:
# LIBZIP_INCLUDE_DIR_ZIP
# LIBZIP_INCLUDE_DIR_ZIPCONF
# LIBZIP_LIBRARY
#

find_package(PkgConfig)
pkg_check_modules(PC_GIMP QUIET gimp)

find_path(GIMP_INCLUDE_DIR NAMES gimp.h
        HINTS /usr/include/gimp-2.0)

find_library(GIMP_LIBRARY
        NAMES gimp-2.0)

set(GIMP_VERSION 2.0)
