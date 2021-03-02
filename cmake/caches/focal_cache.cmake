include(${CMAKE_SOURCE_DIR}/cmake/caches/prepopulate_cache.cmake)

set(CMAKE_INSTALL_PREFIX "/media/psf/Home/Desktop/.temp/install/ubuntu/fs/")

set(CMAKE_GENERATOR "Ninja")

set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

set(FS_BUILD_GUIS OFF)

set(FS_PACKAGES_DIR "/media/psf/Home/Downloads/prebuilt_packages")

set(VTK_DIR "${FS_PACKAGES_DIR}/vtk/5.10.1")

set(ITK_DIR "${FS_PACKAGES_DIR}/itk/4.13.0")

set(FS_QATOOLS_MODULE ON)

set(FS_REPO_ENVIRONMENT "ci_ubuntu_focal")

set(FS_USE_CCACHE OFF)

set(FS_VERSION "$ENV{USER}-ci-ubuntu-focal")

set(FS_BUILD_MATLAB OFF)

set(FS_GEMS_BUILD_GUI OFF)

set(FS_GEMS_BUILD_MATLAB OFF)

set(FS_USE_ARRAYFIRE OFF)

set(FS_USE_HPX OFF)

add_definitions(-DFS_ITK_LEGACY_REMOVE)
