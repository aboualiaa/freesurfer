project(mri_wbc)

include_directories(${FS_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

add_executable(mri_wbc mri_wbc.cpp)
target_link_libraries(mri_wbc utils Boost::program_options Boost::filesystem benchmark::benchmark ${FMT_LIBRARIES})
#target_link_libraries(mri_wbc utils ${Boost_LIBRARIES} benchmark fmt)
install(TARGETS mri_wbc DESTINATION bin)

add_executable(mri_wbc_tests mri_wbc_tests.cpp)
target_link_libraries(mri_wbc_tests utils Boost::program_options Boost::unit_test_framework Boost::filesystem)
