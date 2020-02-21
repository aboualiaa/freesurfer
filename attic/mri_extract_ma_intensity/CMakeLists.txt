project(mri_extract_ma_intensity)

include_directories(${FS_INCLUDE_DIRS})

add_executable(mri_extract_ma_intensity mri_extract_ma_intensity.cpp)
target_link_libraries(mri_extract_ma_intensity utils)

install(TARGETS mri_extract_ma_intensity DESTINATION bin)
