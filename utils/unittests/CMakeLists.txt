add_executable(dmatrix_tests dmatrix_tests.cpp)

target_include_directories(dmatrix_tests PUBLIC ${gtest_SOURCE_DIR}/include)
target_link_libraries(dmatrix_tests utils gtest ${GTEST_LIBRARIES})

if(CMAKE_CONFIGURATION_TYPES)
  foreach(X IN LISTS CMAKE_CONFIGURATION_TYPES)
    add_test(dmatrix_tests_${X} dmatrix_tests)
    set_property(TEST dmatrix_tests_${X} PROPERTY LABELS Unit)
  endforeach()
else()
  add_test(dmatrix_tests dmatrix_tests)
  set_property(TEST dmatrix_tests PROPERTY LABELS Unit)
endif()

