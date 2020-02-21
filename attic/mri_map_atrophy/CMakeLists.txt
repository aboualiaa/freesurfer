project(mri_map_atrophy)

include_directories(${FS_INCLUDE_DIRS})

add_executable(mri_map_atrophy mri_map_atrophy.cpp)
target_link_libraries(mri_map_atrophy utils)

install(TARGETS mri_map_atrophy DESTINATION bin)
