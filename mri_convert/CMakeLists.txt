cmake_policy(SET CMP0079 NEW)
project(mri_convert)
if(NOT APPLE)
  set(EIGEN3_INCLUDE_DIR "/usr/include/eigen")
else()
  set(EIGEN3_INCLUDE_DIR "/usr/local/include/eigen")
endif()

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -fsanitize-recover=address -fsanitize-memory-track-origins -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-common -O0")

include_directories(${FS_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/packages/dicom ${ITK_INCLUDE_DIRS} ${EIGEN3_INCLUDE_DIR}
                    ${Boost_INCLUDE_DIRS} ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_library(mri_convert_lib STATIC EXCLUDE_FROM_ALL mri_convert_lib.cpp)

add_executable(mri_convert_exec mri_convert_exec.cpp)
add_executable(mri_convert mri_convert_exec.cpp)
target_link_libraries(mri_convert_exec utils mri_convert_lib Boost::program_options benchmark fmt absl::strings)
target_link_libraries(mri_convert utils mri_convert_lib Boost::program_options benchmark fmt absl::strings)
add_help(mri_convert_exec mri_convert.help.xml)
add_help(mri_convert mri_convert.help.xml)
install(TARGETS mri_convert_exec DESTINATION bin)
add_test_script(NAME mri_convert_test SCRIPT test.sh DEPENDS mri_convert)

add_executable(mri_make_uchar mri_make_uchar.cpp)
target_link_libraries(mri_make_uchar utils)
install(TARGETS mri_make_uchar DESTINATION bin)

# tests
add_executable(mri_convert_tests mri_convert_tests.cpp)
add_help(mri_convert_tests mri_convert.help.xml)
set_target_properties(mri_convert_tests PROPERTIES EXCLUDE_FROM_ALL FALSE)
target_link_libraries(mri_convert_tests utils mri_convert_lib ${GTEST_LIBRARIES}
                      benchmark fmt gtest ${ARMADILLO_LIBRARIES}
                      Boost::program_options)
add_test(mri_convert_tests mri_convert_tests)
set_property(TEST mri_convert_tests PROPERTY LABELS Unit)
add_test(mri_convert_test mri_convert_tests)
