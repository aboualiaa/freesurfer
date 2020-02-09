# PETSC Find Module

if(NOT PETSC_DIR)
  if(APPLE)
    set(PETSC_DIR "/usr/local/opt/petsc")
  else()
    set(PETSC_DIR ${FS_PACKAGES_DIR}/petsc/2.3.3)
  endif()
endif()

set(PETSC_INCLUDE_DIR ${PETSC_DIR}/include)

find_package_handle_standard_args(PETSC DEFAULT_MSG PETSC_INCLUDE_DIR)

library_paths(
  NAME PETSC_LIBRARIES
  LIBDIR ${PETSC_DIR}/lib
  LIBRARIES
  petscts
  petscsnes
  petscksp
  petscdm
  petscmat
  petscvec
  petsc
  petsccontrib
  mpich
  pmpich
)
