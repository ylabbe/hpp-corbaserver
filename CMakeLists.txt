# Copyright (c) 2012 CNRS
# Author: Florent Lamiraux
#
# This file is part of hpp-corbaserver.
# hpp-corbaserver is free software: you can redistribute it
# and/or modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation, either version
# 3 of the License, or (at your option) any later version.
#
# hpp-corbaserver is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Lesser Public License for more details.  You should have
# received a copy of the GNU Lesser General Public License along with
# hpp-corbaserver.  If not, see
# <http://www.gnu.org/licenses/>.

# Requires at least CMake 2.6 to configure the package.
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(CXX_DISABLE_WERROR true)

INCLUDE(cmake/base.cmake)
INCLUDE(cmake/hpp.cmake)
INCLUDE(cmake/idl.cmake)
INCLUDE(cmake/python.cmake)
INCLUDE(cmake/test.cmake)

INCLUDE(cmake-modules/omniidl.cmake)

SET(PROJECT_NAME hpp-corbaserver)
SET(PROJECT_DESCRIPTION "Corba server for Humanoid Path Planner applications")

SET(CLIENT_ONLY FALSE CACHE BOOL "Set to true to install the client only")

SET(${PROJECT_NAME}_HEADERS
  include/hpp/corbaserver/client.hh
  )
IF (NOT CLIENT_ONLY)
  SET(${PROJECT_NAME}_HEADERS
    include/hpp/corbaserver/doc.hh
    include/hpp/corbaserver/fwd.hh
    include/hpp/corbaserver/object-map.hh
    include/hpp/corbaserver/problem-solver-map.hh
    include/hpp/corbaserver/servant-base.hh
    include/hpp/corbaserver/server.hh
    include/hpp/corbaserver/server-plugin.hh
    include/hpp/corbaserver/conversions.hh

    ${${PROJECT_NAME}_HEADERS}
    )
ENDIF (NOT CLIENT_ONLY)

SETUP_HPP_PROJECT ()

# Dependencies
ADD_REQUIRED_DEPENDENCY("omniORB4 >= 4.1.4")
ADD_REQUIRED_DEPENDENCY("omniDynamic4 >= 4.1.4")
FINDPYTHON()
IF (NOT CLIENT_ONLY)
  ADD_REQUIRED_DEPENDENCY("hpp-core >= 4")
  ADD_REQUIRED_DEPENDENCY("hpp-constraints >= 4")
  ADD_REQUIRED_DEPENDENCY("hpp-pinocchio > 4")
  ADD_REQUIRED_DEPENDENCY("hpp-template-corba >= 1.1")
ENDIF (NOT CLIENT_ONLY)

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(tests)

PKG_CONFIG_APPEND_LIBS(${PROJECT_NAME})

SET(PKG_CONFIG_EXTRA "${PKG_CONFIG_EXTRA}\ncmake_plugin=${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake/${PROJECT_NAME}-plugin.cmake")
INSTALL(FILES cmake-modules/hpp-corbaserver-plugin.cmake
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake/
  PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE)
# install omniidl backend
INSTALL(FILES
  cmake-modules/omniidl/cxx_impl/__init__.py
  cmake-modules/omniidl/cxx_impl/main.py
  cmake-modules/omniidl/cxx_impl/template.py
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/omniidl/cxx_impl
  PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE)

SETUP_HPP_PROJECT_FINALIZE()
