project(mri_cc_ma_fill)

include_directories(${FS_INCLUDE_DIRS})

add_executable(mri_cc_ma_fill mri_cc_ma_fill.cpp)
target_link_libraries(mri_cc_ma_fill utils)

install(TARGETS mri_cc_ma_fill DESTINATION bin)
