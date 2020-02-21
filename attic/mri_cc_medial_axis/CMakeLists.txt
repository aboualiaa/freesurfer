project(mri_cc_medial_axis)

include_directories(${FS_INCLUDE_DIRS})

add_executable(mri_cc_medial_axis mri_cc_medial_axis.cpp)
target_link_libraries(mri_cc_medial_axis utils)

install(TARGETS mri_cc_medial_axis DESTINATION bin)
