# PETSC Find Module

if(NOT PETSC_DIR)
  set(PETSC_DIR "/usr/local/opt/petsc")
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
