project(sscnn_skullstripping)

install_pyscript(sscnn_skullstrip)

install(DIRECTORY sscnn_skullstripping DESTINATION ${CMAKE_INSTALL_PREFIX}/python/packages)
