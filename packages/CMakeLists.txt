project(packages)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectories(
        tiff
        GSL
        googletest
        benchmark
        jpeg
        cephes
        dicom
        gifti
        glut
        minc
        netcdf
        nifti
        nrrdio
        svm
        tetgen
)

mark_as_advanced(GSL_TEST
        GSL_CXX_STANDARD)

mark_as_advanced(INTL_LIBRARY
        GTS_LIBRARY
        GTS_INCLUDE_DIR
        GLIB_LIBRARY
        GLIB_INCLUDE_DIR
        GLIB_CONFIG_INCLUDE_DIR)

mark_as_advanced(INSTALL_GTEST
        BUILD_GMOCK
        )

mark_as_advanced(BENCHMARK_DOWNLOAD_DEPENDENCIES
        BENCHMARK_USE_LIBCXX
        BENCHMARK_ENABLE_TESTING
        BENCHMARK_ENABLE_LTO
        BENCHMARK_ENABLE_INSTALL
        BENCHMARK_ENABLE_GTEST_TESTS
        BENCHMARK_ENABLE_EXCEPTIONS
        BENCHMARK_ENABLE_ASSEMBLY_TESTS
        BENCHMARK_BUILD_32_BITS
        LIBRT)

mark_as_advanced(FMT_WERROR
        FMT_TEST
        FMT_PEDANTIC
        FMT_INSTALL
        FMT_FUZZ
        FMT_DOC)
