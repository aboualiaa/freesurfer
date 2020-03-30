if(NOT FS_BUILD_GUIS)
  return()
endif()

# FLTK is required
find_package(FLTK REQUIRED)
mark_as_advanced(FLTK_MATH_LIBRARY FLTK_INCLUDE_DIR FLTK_FLUID_EXECUTABLE FLTK_DIR
                 FLTK_CONFIG_SCRIPT
                 )

include_directories(${FLTK_INCLUDE_DIR})
include_directories(${ITK_INCLUDE_DIRS})
include_directories(${VTK_INCLUDE_DIRS})
include_directories(${FS_INCLUDE_DIRS})

# VTK as well
if(NOT VTK_FOUND)
  find_package(VTK REQUIRED)
  include(${VTK_USE_FILE})
endif()

# FLTK's fluid generates C++ files in the build directory
include_directories(${GEMS_SOURCE_DIR}/GUI)
include_directories(${GEMS_BINARY_DIR}/GUI)

# add libary
add_library(kvlGEMSGUI vtkFlRenderWindowInteractor.cxx kvlImageViewer.cxx)
target_link_libraries(kvlGEMSGUI ${ITK_LIBRARIES} ${VTK_LIBRARIES} ${FLTK_LIBRARIES})

# add an executable
#fltk_wrap_ui(kvlEstimateAtlasParameters kvlAtlasParameterEstimationConsoleGUI.fl)

add_executable(kvlEstimateAtlasParameters
               kvlAtlasParameterEstimationConsoleGUI.cxx kvlEstimateAtlasParameters.cxx
               kvlAtlasParameterEstimationConsole.cxx
               )
target_link_libraries(kvlEstimateAtlasParameters kvlGEMSCommon kvlGEMSGUI)

# add executable
#fltk_wrap_ui(kvlViewMeshCollectionWithGUI kvlAtlasMeshViewingConsoleGUI.fl)

add_executable(kvlViewMeshCollectionWithGUI
               kvlAtlasMeshViewingConsoleGUI.cxx kvlViewMeshCollectionWithGUI.cxx
               kvlAtlasMeshViewingConsole.cxx
               )
target_link_libraries(kvlViewMeshCollectionWithGUI kvlGEMSCommon kvlGEMSGUI)
