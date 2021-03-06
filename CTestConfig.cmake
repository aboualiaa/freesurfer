## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
##
## # The following are required to submit to the CDash dashboard:
##   ENABLE_TESTING()
##   INCLUDE(CTest)

set(CTEST_PROJECT_NAME "freesurfer")
set(CTEST_NIGHTLY_START_TIME "01:00:00 UTC")

set(DART_TESTING_TIMEOUT 1000000)
set(BUILDNAME
    "${CMAKE_HOST_SYSTEM}_${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}_${CMAKE_BUILD_TYPE}$ENV{TESTING_TYPE}"
    )

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "my.cdash.org")
set(CTEST_DROP_LOCATION "/submit.php?project=freesurfer")
set(CTEST_DROP_SITE_CDASH TRUE)
