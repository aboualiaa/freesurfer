cmake_policy(SET CMP0079 NEW)
project(mri_convert)
if(NOT APPLE)
  set(EIGEN3_INCLUDE_DIR "/usr/include/eigen")
else()
  set(EIGEN3_INCLUDE_DIR "/usr/local/include/eigen")
endif()

add_library(mri_convert_lib STATIC EXCLUDE_FROM_ALL mri_convert_lib.cpp)
set(COMMON_LIBS utils mri_convert_lib Boost::program_options benchmark::benchmark fmt::fmt ${SPDLOG_LIBRARIES})

target_include_directories(mri_convert_lib
                           PUBLIC ${FS_INCLUDE_DIRS}
                                  ${CMAKE_SOURCE_DIR}/packages/dicom
                                  ${ITK_INCLUDE_DIRS}
                                  ${EIGEN3_INCLUDE_DIR}
                                  ${Boost_INCLUDE_DIRS}
                                  ${gtest_SOURCE_DIR}/include
                                  ${gtest_SOURCE_DIR}
                           )

add_executable(mri_convert mri_convert_exec.cpp)
target_link_libraries(mri_convert ${COMMON_LIBS})

install(TARGETS mri_convert DESTINATION bin)

add_executable(mri_make_uchar mri_make_uchar.cpp)
target_include_directories(mri_make_uchar PUBLIC ${FS_INCLUDE_DIRS})
target_link_libraries(mri_make_uchar utils)
install(TARGETS mri_make_uchar DESTINATION bin)

add_help(mri_convert mri_convert.help.xml)

if(BUILD_TESTING)
  # tests
  add_test_script(NAME mri_convert_test SCRIPT test.sh DEPENDS mri_convert)

  add_executable(mri_convert_unit mri_convert_unit.cpp)
  add_help(mri_convert_unit mri_convert.help.xml)
  target_link_libraries(mri_convert_unit ${COMMON_LIBS} ${GTEST_LIBRARIES} ${ARMADILLO_LIBRARIES})

  if(CMAKE_CONFIGURATION_TYPES)
    foreach(X IN LISTS CMAKE_CONFIGURATION_TYPES)
      add_test(mri_convert_unit_${X} mri_convert_unit)
      set_property(TEST mri_convert_unit_${X} PROPERTY LABELS Unit)
    endforeach()
  else()
    add_test(mri_convert_unit mri_convert_unit)
    set_property(TEST mri_convert_unit PROPERTY LABELS Unit)
  endif()

  add_executable(mri_convert_bench mri_convert_bench.cpp)
  add_help(mri_convert_bench mri_convert.help.xml)
  target_link_libraries(mri_convert_bench ${COMMON_LIBS} ${GTEST_LIBRARIES} ${ARMADILLO_LIBRARIES})
endif()
