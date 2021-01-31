# X11 Find Module

find_path(X11_INCLUDE_DIR
          HINTS /usr/X11
          NAMES GL/gl.h
          PATH_SUFFIXES include
          )
find_library(X11_LIBRARY
             HINTS /usr/X11
             NAMES X11
             PATH_SUFFIXES lib
             )
find_library(XMU_LIBRARY
             HINTS /usr/X11
             NAMES Xmu
             PATH_SUFFIXES lib
             )
find_library(XT_LIBRARY
             HINTS /usr/X11
             NAMES Xt
             PATH_SUFFIXES lib
             )

if(APPLE)
  find_library(GL_LIBRARY
               HINTS /usr/X11
               NAMES GL
               PATH_SUFFIXES lib
               )
  find_library(GLU_LIBRARY
               HINTS /usr/X11
               NAMES GLU
               PATH_SUFFIXES lib
               )
  find_package_handle_standard_args(
    X11
    DEFAULT_MSG
    X11_INCLUDE_DIR
    X11_LIBRARY
    XMU_LIBRARY
    XT_LIBRARY
    GL_LIBRARY
    GLU_LIBRARY
    )
  set(X11_LIBRARIES ${X11_LIBRARY} ${XMU_LIBRARY} ${XT_LIBRARY} ${GL_LIBRARY} ${GLU_LIBRARY})
else()
  find_package_handle_standard_args(X11 DEFAULT_MSG X11_INCLUDE_DIR X11_LIBRARY XMU_LIBRARY XT_LIBRARY)
  set(X11_LIBRARIES ${X11_LIBRARY} ${XMU_LIBRARY} ${XT_LIBRARY})
endif()
