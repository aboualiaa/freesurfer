project(mri_convert)

include_directories(${FS_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/packages/dicom
                    ${Boost_INCLUDE_DIRS})

add_library(mri_convert STATIC EXCLUDE_FROM_ALL mri_convert_lib.cpp)

add_executable(mri_convert_exec mri_convert_exec.cpp)
hpx_setup_target(mri_convert_exec
                 DEPENDENCIES utils mri_convert ${Boost_LIBRARIES} benchmark
                 fmt
                 COMPONENT_DEPENDENCIES iostreams)
add_help(mri_convert_exec mri_convert.help.xml)
install(TARGETS mri_convert_exec DESTINATION bin)
add_test_script(NAME mri_convert_test SCRIPT test.sh DEPENDS mri_convert)

add_executable(mri_make_uchar mri_make_uchar.cpp)
target_link_libraries(mri_make_uchar utils)
install(TARGETS mri_make_uchar DESTINATION bin)

# tests
add_executable(mri_convert_tests mri_convert_tests.cpp)
add_help(mri_convert_tests mri_convert.help.xml)
set_target_properties(mri_convert_tests PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_link_libraries(mri_convert_tests utils mri_convert ${Boost_LIBRARIES} benchmark fmt)
