project(mri_convert)

# ATH: temporarily turning off this warning
add_compile_options(-Wno-self-assign)

include_directories(${FS_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/packages/dicom ${Boost_INCLUDE_DIRS})

add_hpx_executable(mri_convert
  SOURCES mri_convert_17.cpp
  DEPENDENCIES utils ${Boost_LIBRARIES} benchmark fmt
  COMPONENT_DEPENDENCIES iostreams)

#add_executable(mri_convert mri_convert.cpp)
add_help(mri_convert mri_convert.help.xml)
#target_link_libraries(mri_convert utils fmt)

install(TARGETS mri_convert DESTINATION bin)

add_test_script(NAME mri_convert_test SCRIPT test.sh DEPENDS mri_convert)

add_executable(mri_make_uchar mri_make_uchar.cpp)
target_link_libraries(mri_make_uchar utils)
install(TARGETS mri_make_uchar DESTINATION bin)

# tests
add_executable(mri_convert_tests mri_convert_tests.cpp)
set_target_properties(mri_convert_tests PROPERTIES EXCLUDE_FROM_ALL TRUE)
set_target_properties(mri_convert PROPERTIES ENABLE_EXPORTS TRUE)
target_link_libraries(mri_convert_tests utils ${Boost_LIBRARIES} benchmark fmt mri_convert mri_convert)