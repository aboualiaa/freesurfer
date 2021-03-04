add_custom_target(utils_unittests)

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(filename ${files})
  get_filename_component(stripped_filename ${filename} NAME)
  string(REPLACE ".cpp" "" name ${stripped_filename})

  add_executable(${name} ${stripped_filename})
  add_dependencies(utils_unittests ${name})
  target_include_directories(${name} PUBLIC ${gtest_SOURCE_DIR}/include)
  target_link_libraries(${name} utils gtest)

  add_test(${name} ${name})
  set_property(TEST ${name} PROPERTY LABELS Unit)

  if(CMAKE_BUILD_TYPE STREQUAL "coverage")
    set_property(TARGET ${name}
                 PROPERTY ADDITIONAL_CLEAN_FILES
                          "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${name}.dir/${name}.cpp.gcno;${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${name}.dir/${name}.cpp.gcda"
                 )

  endif()
endforeach()
