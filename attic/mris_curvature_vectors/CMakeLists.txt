project(mris_curvature_vectors)

include_directories(${FS_INCLUDE_DIRS})

add_executable(mris_curvature_vectors mris_curvature_vectors.cpp)
target_link_libraries(mris_curvature_vectors utils)

install(TARGETS mris_curvature_vectors DESTINATION bin)
