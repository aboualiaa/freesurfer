project(anatomicuts)

if(VTK_FOUND)

  include_directories(${FS_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/Code ${CMAKE_SOURCE_DIR}/freeview/track_io SYSTEM
                      ${ITK_INCLUDE_DIRS} ${VTK_INCLUDE_DIRS}
                      )

  include(${ITK_USE_FILE})
  include(${VTK_USE_FILE})

  set(TRACKIO ../freeview/track_io/TrackIO.cpp)

  #AnatomiCuts correspondences
  add_executable(dmri_AnatomiCuts AnatomiCuts.cxx ${TRACKIO})
  target_link_libraries(dmri_AnatomiCuts utils ${ITK_LIBRARIES} ${VTK_LIBRARIES})
  install(TARGETS dmri_AnatomiCuts DESTINATION bin)

  #AnatomiCuts
  add_executable(dmri_match AnatomiCuts_correspondences.cxx ${TRACKIO})
  target_link_libraries(dmri_match utils ${ITK_LIBRARIES} ${VTK_LIBRARIES})
  install(TARGETS dmri_match DESTINATION bin)

  #TRK tools for header
  add_executable(trk_tools trk_tools.cxx ${TRACKIO})
  target_link_libraries(trk_tools utils ${ITK_LIBRARIES} ${VTK_LIBRARIES})
  install(TARGETS trk_tools DESTINATION bin)

  #streamline filtering
  add_executable(streamlineFilter StreamlineFilter.cxx ${TRACKIO})
  target_link_libraries(streamlineFilter utils ${ITK_LIBRARIES} ${VTK_LIBRARIES})
  install(TARGETS streamlineFilter DESTINATION bin)

  #dmri_stats_ac
  add_executable(dmri_stats_ac dmri_stats_ac.cxx ${TRACKIO})
  target_link_libraries(dmri_stats_ac utils ${ITK_LIBRARIES} ${VTK_LIBRARIES})
  install(TARGETS dmri_stats_ac DESTINATION bin)

  #orientation planes
  add_executable(testOrientationPlanesFromParcellation testOrientationFromParcellationFilter.cxx)
  target_link_libraries(testOrientationPlanesFromParcellation ${ITK_LIBRARIES} ${VTK_LIBRARIES})
  install(TARGETS testOrientationPlanesFromParcellation DESTINATION bin)

  #Save histograms
  add_executable(dmri_saveHistograms SaveHistograms.cxx ${TRACKIO})
  target_link_libraries(dmri_saveHistograms ${ITK_LIBRARIES} ${VTK_LIBRARIES})
  install(TARGETS dmri_saveHistograms DESTINATION bin)

  #dmri_groupByEndpoints
  add_executable(dmri_groupByEndpoints dmri_groupByEndpoints.cxx ${TRACKIO})
  target_link_libraries(dmri_groupByEndpoints utils ${ITK_LIBRARIES} ${VTK_LIBRARIES})
  install(TARGETS dmri_groupByEndpoints DESTINATION bin)

  #dmri_extractSurfaceMeasurements
  add_executable(dmri_extractSurfaceMeasurements dmri_extractSurfaceMeasurements.cxx ${TRACKIO})
  target_link_libraries(dmri_extractSurfaceMeasurements utils ${ITK_LIBRARIES} ${VTK_LIBRARIES})
  install(TARGETS dmri_extractSurfaceMeasurements DESTINATION bin)

  #dmri_coloredFA
  add_executable(dmri_coloredFA dmri_coloredFA.cxx ${TRACKIO})
  target_link_libraries(dmri_coloredFA utils ${OPENGL_LIBRARIES} ${ITK_LIBRARIES} ${VTK_LIBRARIES})
  install(TARGETS dmri_coloredFA DESTINATION bin)

  #dmri_projectEndPoints
  add_executable(dmri_projectEndPoints dmri_projectEndPoints.cxx ${TRACKIO})
  target_link_libraries(dmri_projectEndPoints utils ${ITK_LIBRARIES} ${VTK_LIBRARIES})
  install(TARGETS dmri_projectEndPoints DESTINATION bin)

  #dmri_neighboringRegions
  add_executable(dmri_neighboringRegions dmri_neighboringRegions.cxx ${TRACKIO})
  target_link_libraries(dmri_neighboringRegions utils ${ITK_LIBRARIES} ${VTK_LIBRARIES})
  install(TARGETS dmri_neighboringRegions DESTINATION bin)

endif()

install(PROGRAMS dmri_ac.sh DESTINATION bin)

install_pyscript(diffusionUtils anatomiCutsUtils dmri_violinPlots)
