# Install script for directory: /home/mustafa.vora/minotaur

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX ".")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/mustafa.vora/minotaur/build/src/base/cmake_install.cmake")
  include("/home/mustafa.vora/minotaur/build/src/engines/Bqpd/cmake_install.cmake")
  include("/home/mustafa.vora/minotaur/build/src/engines/FilterSQP/cmake_install.cmake")
  include("/home/mustafa.vora/minotaur/build/src/engines/Ipopt/cmake_install.cmake")
  include("/home/mustafa.vora/minotaur/build/src/engines/OsiLP/cmake_install.cmake")
  include("/home/mustafa.vora/minotaur/build/src/engines/Cbc/cmake_install.cmake")
  include("/home/mustafa.vora/minotaur/build/src/engines/cmake_install.cmake")
  include("/home/mustafa.vora/minotaur/build/src/interfaces/ampl/cmake_install.cmake")
  include("/home/mustafa.vora/minotaur/build/src/algorithms/cmake_install.cmake")
  include("/home/mustafa.vora/minotaur/build/src/testing/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/mustafa.vora/minotaur/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
