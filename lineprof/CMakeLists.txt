project(lineprof)

if(ITK_FOUND AND VTK_FOUND AND PETSC_FOUND)
find_package(MPI)
  include_directories(
    SYSTEM
    ${ITK_INCLUDE_DIRS}
    ${VTK_INCLUDE_DIRS}
    ${PETSC_INCLUDE_DIR}
  )

  # temporary macro to deal with BoundingBoxType
  # todo: this should be removed once we updgrade everything
  add_definitions(-DHAVE_ITK45)

  set(SOURCES
    Tracer.cpp
    Preprocessor.cpp
    PetscSolver.cpp
    LineProf.cpp
  )

  add_compile_options(-Wno-deprecated)

  add_library(lineprof STATIC ${SOURCES})

  if(APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Accelerate")
  endif()

  add_executable(LineProfTest LineProfTest.cpp)
  add_test(LineProfTest LineProfTest.cpp)
  target_link_libraries(LineProfTest
    lineprof
    ${ZLIB_LIBRARIES}
    ${VTK_LIBRARIES}
    ${PETSC_LIBRARIES}
    ${LAPACK_LIBRARIES}
    ${BLAS_LIBRARIES}
    ${GFORTRAN_LIBRARIES}
    ${ITK_LIBRARIES}
    ${MPI_LIBRARIES}
  )

endif()
