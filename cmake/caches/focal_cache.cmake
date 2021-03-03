include(${CMAKE_SOURCE_DIR}/cmake/caches/prepopulate_cache.cmake)

set(CMAKE_INSTALL_PREFIX
    "/media/psf/Home/Desktop/.temp/install/ubuntu/fs/"
    CACHE STRING "install prefix" FORCE
    )

set(CMAKE_GENERATOR
    "Ninja"
    CACHE STRING "" FORCE
    )

set(CMAKE_EXPORT_COMPILE_COMMANDS
    OFF
    CACHE BOOL "" FORCE
    )

set(FS_BUILD_GUIS
    OFF
    CACHE BOOL "" FORCE
    )

set(FS_PACKAGES_DIR
    "/media/psf/Home/Downloads/prebuilt_packages"
    CACHE STRING "" FORCE
    )

set(VTK_DIR
    "${FS_PACKAGES_DIR}/vtk/5.10.1"
    CACHE PATH "" FORCE
    )

set(ITK_DIR
    "${FS_PACKAGES_DIR}/itk/4.13.0"
    CACHE PATH "" FORCE
    )

set(FS_QATOOLS_MODULE
    ON
    CACHE BOOL "" FORCE
    )

set(FS_REPO_ENVIRONMENT
    "ci_ubuntu_focal"
    CACHE STRING "" FORCE
    )

set(FS_USE_CCACHE
    OFF
    CACHE BOOL "" FORCE
    )

set(FS_VERSION
    "$ENV{USER}-ci-ubuntu-focal"
    CACHE STRING "" FORCE
    )

set(FS_BUILD_MATLAB
    OFF
    CACHE BOOL "" FORCE
    )

set(FS_GEMS_BUILD_GUI
    OFF
    CACHE BOOL "" FORCE
    )

set(FS_GEMS_BUILD_MATLAB
    OFF
    CACHE BOOL "" FORCE
    )

set(FS_USE_ARRAYFIRE
    OFF
    CACHE BOOL "" FORCE
    )

set(FS_USE_HPX
    OFF
    CACHE BOOL "" FORCE
    )

add_definitions(-DFS_ITK_LEGACY_REMOVE)
