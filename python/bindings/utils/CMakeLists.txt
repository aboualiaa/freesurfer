include_directories(${CMAKE_CURRENT_SOURCE_DIR})

pybind11_add_module(bindings
                    module.cpp
                    morph.cpp
                    numpy.cpp
                    parameterization.cpp
                    surface.cpp
                    volume.cpp
                    xform.cpp
                    )

# link utilities
target_link_libraries(bindings PRIVATE utils)

# make sure the bindings library gets built into the repository even in out-of-source builds
set_target_properties(bindings PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/python/freesurfer)
