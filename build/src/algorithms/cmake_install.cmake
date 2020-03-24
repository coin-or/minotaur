# Install script for directory: /home/mustafa.vora/minotaur/src/algorithms

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

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/lstoa" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/lstoa")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/lstoa"
         RPATH ":/home/mustafa.vora/minotaur/build/./lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/home/mustafa.vora/minotaur/build/src/algorithms/lstoa")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/lstoa" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/lstoa")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/lstoa"
         OLD_RPATH "/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/src/interfaces/ampl:/home/mustafa.vora/minotaur/build/src/engines:/home/mustafa.vora/minotaur/build/src/engines/Ipopt:/home/mustafa.vora/minotaur/build/src/engines/Cbc:/home/mustafa.vora/minotaur/build/src/engines/OsiLP:/home/mustafa.vora/minotaur/build/src/engines/FilterSQP:/home/mustafa.vora/minotaur/build/src/base:"
         NEW_RPATH ":/home/mustafa.vora/minotaur/build/./lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/lstoa")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/oa" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/oa")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/oa"
         RPATH ":/home/mustafa.vora/minotaur/build/./lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/home/mustafa.vora/minotaur/build/src/algorithms/oa")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/oa" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/oa")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/oa"
         OLD_RPATH "/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/src/interfaces/ampl:/home/mustafa.vora/minotaur/build/src/engines:/home/mustafa.vora/minotaur/build/src/engines/Ipopt:/home/mustafa.vora/minotaur/build/src/engines/Cbc:/home/mustafa.vora/minotaur/build/src/engines/OsiLP:/home/mustafa.vora/minotaur/build/src/engines/FilterSQP:/home/mustafa.vora/minotaur/build/src/base:"
         NEW_RPATH ":/home/mustafa.vora/minotaur/build/./lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/oa")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/bnb" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/bnb")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/bnb"
         RPATH ":/home/mustafa.vora/minotaur/build/./lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/home/mustafa.vora/minotaur/build/src/algorithms/bnb")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/bnb" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/bnb")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/bnb"
         OLD_RPATH "/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/src/interfaces/ampl:/home/mustafa.vora/minotaur/build/src/engines:/home/mustafa.vora/minotaur/build/src/engines/Ipopt:/home/mustafa.vora/minotaur/build/src/engines/Cbc:/home/mustafa.vora/minotaur/build/src/engines/OsiLP:/home/mustafa.vora/minotaur/build/src/engines/FilterSQP:/home/mustafa.vora/minotaur/build/src/base:"
         NEW_RPATH ":/home/mustafa.vora/minotaur/build/./lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/bnb")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/mcqg" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/mcqg")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/mcqg"
         RPATH ":/home/mustafa.vora/minotaur/build/./lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/home/mustafa.vora/minotaur/build/src/algorithms/mcqg")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/mcqg" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/mcqg")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/mcqg"
         OLD_RPATH "/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/src/interfaces/ampl:/home/mustafa.vora/minotaur/build/src/engines:/home/mustafa.vora/minotaur/build/src/engines/Ipopt:/home/mustafa.vora/minotaur/build/src/engines/Cbc:/home/mustafa.vora/minotaur/build/src/engines/OsiLP:/home/mustafa.vora/minotaur/build/src/engines/FilterSQP:/home/mustafa.vora/minotaur/build/src/base:"
         NEW_RPATH ":/home/mustafa.vora/minotaur/build/./lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/mcqg")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/mcbnb" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/mcbnb")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/mcbnb"
         RPATH ":/home/mustafa.vora/minotaur/build/./lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/home/mustafa.vora/minotaur/build/src/algorithms/mcbnb")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/mcbnb" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/mcbnb")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/mcbnb"
         OLD_RPATH "/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/src/interfaces/ampl:/home/mustafa.vora/minotaur/build/src/engines:/home/mustafa.vora/minotaur/build/src/engines/Ipopt:/home/mustafa.vora/minotaur/build/src/engines/Cbc:/home/mustafa.vora/minotaur/build/src/engines/OsiLP:/home/mustafa.vora/minotaur/build/src/engines/FilterSQP:/home/mustafa.vora/minotaur/build/src/base:"
         NEW_RPATH ":/home/mustafa.vora/minotaur/build/./lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/mcbnb")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/qpd" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/qpd")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/qpd"
         RPATH ":/home/mustafa.vora/minotaur/build/./lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/home/mustafa.vora/minotaur/build/src/algorithms/qpd")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/qpd" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/qpd")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/qpd"
         OLD_RPATH "/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/src/interfaces/ampl:/home/mustafa.vora/minotaur/build/src/engines:/home/mustafa.vora/minotaur/build/src/engines/Ipopt:/home/mustafa.vora/minotaur/build/src/engines/Cbc:/home/mustafa.vora/minotaur/build/src/engines/OsiLP:/home/mustafa.vora/minotaur/build/src/engines/FilterSQP:/home/mustafa.vora/minotaur/build/src/base:"
         NEW_RPATH ":/home/mustafa.vora/minotaur/build/./lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/qpd")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/qg" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/qg")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/qg"
         RPATH ":/home/mustafa.vora/minotaur/build/./lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/home/mustafa.vora/minotaur/build/src/algorithms/qg")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/qg" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/qg")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/qg"
         OLD_RPATH "/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/src/interfaces/ampl:/home/mustafa.vora/minotaur/build/src/engines:/home/mustafa.vora/minotaur/build/src/engines/Ipopt:/home/mustafa.vora/minotaur/build/src/engines/Cbc:/home/mustafa.vora/minotaur/build/src/engines/OsiLP:/home/mustafa.vora/minotaur/build/src/engines/FilterSQP:/home/mustafa.vora/minotaur/build/src/base:"
         NEW_RPATH ":/home/mustafa.vora/minotaur/build/./lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/qg")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/qgadv" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/qgadv")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/qgadv"
         RPATH ":/home/mustafa.vora/minotaur/build/./lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/home/mustafa.vora/minotaur/build/src/algorithms/qgadv")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/qgadv" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/qgadv")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/qgadv"
         OLD_RPATH "/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/src/interfaces/ampl:/home/mustafa.vora/minotaur/build/src/engines:/home/mustafa.vora/minotaur/build/src/engines/Ipopt:/home/mustafa.vora/minotaur/build/src/engines/Cbc:/home/mustafa.vora/minotaur/build/src/engines/OsiLP:/home/mustafa.vora/minotaur/build/src/engines/FilterSQP:/home/mustafa.vora/minotaur/build/src/base:"
         NEW_RPATH ":/home/mustafa.vora/minotaur/build/./lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/qgadv")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/glob" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/glob")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/glob"
         RPATH ":/home/mustafa.vora/minotaur/build/./lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/home/mustafa.vora/minotaur/build/src/algorithms/glob")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/glob" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/glob")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/glob"
         OLD_RPATH "/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/src/interfaces/ampl:/home/mustafa.vora/minotaur/build/src/engines:/home/mustafa.vora/minotaur/build/src/engines/Ipopt:/home/mustafa.vora/minotaur/build/src/engines/Cbc:/home/mustafa.vora/minotaur/build/src/engines/OsiLP:/home/mustafa.vora/minotaur/build/src/engines/FilterSQP:/home/mustafa.vora/minotaur/build/src/base:"
         NEW_RPATH ":/home/mustafa.vora/minotaur/build/./lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib:/home/mustafa.vora/minotaur/build/../third-party/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/glob")
    endif()
  endif()
endif()

