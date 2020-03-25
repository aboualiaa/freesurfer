project(pysurfer)
set(CMAKE_CXX_STANDARD 17)
add_subdirectories(bindings)

# if FS_INSTALL_PYTHON_DEPS is on, all required python packages will get
# installed to freesurfer/python, otherwise, only the freesurfer package gets installed
option(FS_INSTALL_PYTHON_DEPS "Install freesurfer python dependencies" ON)
if(NOT FS_INSTALL_PYTHON_DEPS)
  set(PIP_FLAGS "--no-dependencies")
endif()

# run 'pip install' and direct package installs to freesufer/python
install(CODE "
  message(STATUS \"Installing: ${CMAKE_INSTALL_PREFIX}/python/freesurfer\")
  execute_process(COMMAND bash -c \"${PYTHON_EXECUTABLE} -m pip install ${CMAKE_CURRENT_SOURCE_DIR} ${PIP_FLAGS} --upgrade --target=${CMAKE_INSTALL_PREFIX}/python/packages\" RESULT_VARIABLE retcode)
  if(NOT \${retcode} STREQUAL 0)
    # message(FATAL_ERROR \"Could not install freesurfer python module\")
    message(SEND_ERROR \"Could not install freesurfer python module\")
  endif()"
        )

install(PROGRAMS fspython DESTINATION bin)
