# Armadillo Find Module

if(NOT ARMADILLO_DIR)
  set(ARMADILLO_DIR ${FS_PACKAGES_DIR}/armadillo/9.200.7)
endif()

find_path(ARMADILLO_INCLUDE_DIRS
          HINTS ${ARMADILLO_DIR}
          NAMES armadillo
          PATH_SUFFIXES include
          )
find_library(ARMADILLO_LIBRARIES
             HINTS ${ARMADILLO_DIR}
             NAMES armadillo
             PATH_SUFFIXES lib64 lib
             )
find_package_handle_standard_args(
  ARMADILLO DEFAULT_MSG ARMADILLO_INCLUDE_DIRS ARMADILLO_LIBRARIES
  )
