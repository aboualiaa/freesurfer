set(BUILD_TESTING OFF CACHE BOOL "build test target")
set(FS_MINIMAL_BUILD OFF CACHE BOOL "create a minimal build")
set(FS_INTEGRATION_TESTING OFF CACHE BOOL "get data for integration testing")
set(FS_BUILD_ATTIC ON CACHE BOOL "build deprecated tools")
set(FS_INFANT_MODULE ON CACHE BOOL "build infant module")
set(FS_BUILD_DNG ON CACHE BOOL "build dng's testing tools")
set(FS_SUPPRESS_WARNINGS ON CACHE BOOL "suppress compiler warnings")
set(GEMS_BUILD_MATLAB ON CACHE BOOL "build matlab wrappers")
set(FS_GEMS_BUILD_GUI ON CACHE BOOL "build gems guis")
set(CMAKE_INSTALL_PREFIX "/Users/aboualiaa/Desktop/.temp/install/" CACHE STRING "install prefix")
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "build type")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release;RelWithDebInfo;MinSizeRel")
set(CMAKE_GENERATOR "Xcode" CACHE STRING "Ninja, Unix Makefiles, Xcode")
set_property(CACHE CMAKE_GENERATOR PROPERTY STRINGS "Ninja;Unix Makefiles;Xcode")
set(FS_ADD_INSTRUMENTATION "None" CACHE STRING "choose an instrumentation")
set_property(CACHE FS_ADD_INSTRUMENTATION PROPERTY STRINGS "none;ubsan;asan;tsan;msan;cfisan;coverage;profile")
set(FS_INSTALL_PYTHON_DEPS ON CACHE BOOL "python deps")
set(FS_USE_CCACHE ON CACHE BOOL "use ccache if present")

set(FS_BUILD_BOOST_TESTS ON CACHE BOOL "")
set(FS_GEMS_BUILD_CUDA OFF CACHE BOOL "")
set(FS_GEMS_BUILD_EXECUTABLES ON CACHE BOOL "")
set(FS_GEMS_BUILD_PYTHON OFF CACHE BOOL "")
set(FS_GEMS_BUILD_SHARED_LIBS ON CACHE BOOL "")
set(FS_GEMS_BUILD_TESTING ON CACHE BOOL "")
set(FS_GEMS_MAKE_SPARSE_INITIAL_MESHES OFF CACHE BOOL "")
set(FS_BUILD_GUIS ON CACHE BOOL "")
set(DISTRIBUTE_FSPYTHON OFF CACHE BOOL "")


string(TIMESTAMP TODAY "%Y%m%d")
set(BUILD_STAMP "freesurfer-local-build-${TODAY}" CACHE STRING "Distribution build stamp")
set(FS_VERSION "$ENV{USER}-local" CACHE STRING "Distribution version")
