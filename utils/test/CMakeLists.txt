add_executable(topology_test topology_test.cpp)
target_link_libraries(topology_test utils)

add_executable(test_TriangleFile_readWrite test_TriangleFile_readWrite.cpp)
target_link_libraries(test_TriangleFile_readWrite utils)

add_executable(testcolortab testcolortab.cpp)
target_link_libraries(testcolortab utils)

add_executable(test_c_nr_wrapper test_c_nr_wrapper.cpp)
target_link_libraries(test_c_nr_wrapper utils)

add_executable(extest extest.cpp)
target_link_libraries(extest utils)

add_executable(inftest inftest.cpp)
target_link_libraries(inftest utils)

add_executable(tiff_write_image tiff_write_image.cpp)
target_link_libraries(tiff_write_image utils)

add_executable(sc_test sc_test.cpp)
target_link_libraries(sc_test utils)

add_executable(sse_mathfun_test sse_mathfun_test.cpp)
target_link_libraries(sse_mathfun_test m)

add_test_script(NAME
                utils_test
                SCRIPT
                test.sh
                DEPENDS
                test_TriangleFile_readWrite
                topology_test
                testcolortab
                test_c_nr_wrapper
                extest
                inftest
                tiff_write_image
                sc_test
                sse_mathfun_test
                )

add_subdirectories(mriBuildVoronoiDiagramFloat MRIScomputeBorderValues mrishash mriSoapBubbleFloat)
