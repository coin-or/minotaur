## Minotaur CMake file

project(MINOTAUR CXX C Fortran)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8 FATAL_ERROR) 
cmake_policy(SET CMP0003 NEW)

## Fortran is required for Bqpd, Filter.
## C is required for FortranCInterface.
INCLUDE(CheckIncludeFile)
INCLUDE(CheckIncludeFileCXX)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckLibraryExists)
INCLUDE(FortranCInterface)
INCLUDE(FindSubversion)
INCLUDE(FindPkgConfig OPTIONAL RESULT_VARIABLE MNTR_PKGCON)

set (MINOTAUR_VERSION_MAJOR 0)
set (MINOTAUR_VERSION_MINOR 2)

set (MSG_HEAD "Minotaur config: ")  ## must preceed every message.

## Stop if we are compiling in the minotaur-root directory.
if ((EXISTS "${PROJECT_BINARY_DIR}/src/base/Types.h") OR 
  ("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}"))
  message(FATAL_ERROR
  ${MSG_HEAD} "In-source builds are not allowed. Please create a build directory and compile there.")
endif()

## Enable compiler warnings
OPTION(COMPILER_WARN "Turn compiler warnings on/off " ON)
if (${COMPILER_WARN})
  set (CMAKE_CXX_FLAGS "-Wall -pedantic -Wmissing-include-dirs -Wunused")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wundef -Wshadow")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wredundant-decls")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual")
endif()

###########################################################################
## Don't install in /usr/local
## see http://www.cmake.org/pipermail/cmake/2007-August/015814.html
###########################################################################
set (MNTR_INSTALL_PREFIX "./" CACHE PATH 
    "Dir. where Minotaur will be installed. Defaults to current dir.")
set(CMAKE_INSTALL_PREFIX ${MNTR_INSTALL_PREFIX} CACHE INTERNAL 
    "Install path prefix, prepended onto install directories.")

###########################################################################
## check if all externals available at one place
###########################################################################
set (ALL_EXTERNS_PATH "" CACHE PATH "Path to the directory where all minotaur external libraries and headers are available (/path/to/third-party)")

###########################################################################
## Shared or static libraries.
###########################################################################
OPTION(BUILD_SHARED_LIBS "Build Minotaur with shared libraries." ON)

###########################################################################
## set runtime-path
###########################################################################
# the RPATH to be used when installing, but only if it's not a system 
# directory
if (${BUILD_SHARED_LIBS})
  LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
  if ("${isSystemDir}" STREQUAL "-1")
    if (IS_ABSOLUTE ${CMAKE_INSTALL_PREFIX})
      set(CMAKE_INSTALL_RPATH 
        ${CMAKE_INSTALL_RPATH}:${CMAKE_INSTALL_PREFIX}/lib)
    else()
      set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH}:${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}/lib)
    endif()
  endif()
endif()

###########################################################################
## getrusage
###########################################################################
set (MINOTAUR_RUSAGE) ## NULL
CHECK_FUNCTION_EXISTS(getrusage MINOTAUR_RUSAGE)
message (STATUS ${MSG_HEAD} "Is rusage available = ${MINOTAUR_RUSAGE}")

###########################################################################
## svnversion
###########################################################################
set (MINOTAUR_SVN_VERSION "0.0") 
get_filename_component(MNTR_ABS_PATH "${PROJECT_SOURCE_DIR}/src" ABSOLUTE)
if (EXISTS "${MNTR_ABS_PATH}/.svn")
  FIND_PACKAGE(Subversion)
  if (Subversion_FOUND)
    # extract working copy information for SOURCE_DIR into TMP_SVN_XXX variables
    Subversion_WC_INFO(${MNTR_ABS_PATH} TMP_SVN)
    set (MINOTAUR_SVN_VERSION ${TMP_SVN_WC_REVISION})
  endif()
endif()
message (STATUS ${MSG_HEAD} "svn version = ${MINOTAUR_SVN_VERSION}")


###########################################################################
## pkg-config
###########################################################################
set (MNTR_HAVE_PKGCON 1)
if (("${MNTR_PKGCON}" STREQUAL "NOTFOUND") OR (NOT PKG_CONFIG_FOUND) OR 
    (NOT ${USE_PKGCON_BOOL}))
  set (MNTR_HAVE_PKGCON 0)
  message(STATUS ${MSG_HEAD} "pkg-config not called.")
else()
  message(STATUS ${MSG_HEAD} "pkg-config found.")
endif()


###########################################################################
## F77_FUNC
###########################################################################
# save name of fortran compiler in CMAKE_Fortran_COMPILER.
get_filename_component (Fortran_COMPILER_NAME ${CMAKE_Fortran_COMPILER} NAME)
FortranCInterface_HEADER(src/base/MinotaurCFortran.h
                         MACRO_NAMESPACE "F77_"
                         SYMBOL_NAMESPACE "F77_")

###########################################################################
## High verbosity option
###########################################################################
OPTION (SPEW_FLAG "Turn verbosity on/off." OFF) 
if (${SPEW_FLAG})
  add_definitions(-DSPEW=1)
else()
  add_definitions(-DSPEW=0)
endif()
message(STATUS ${MSG_HEAD} "SPEW flag is " ${SPEW_FLAG})


###########################################################################
## Debug options
###########################################################################
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  add_definitions(-DDEBUG=1)
else()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
  set (CMAKE_FORTRAN_FLAGS "${CMAKE_FORTRAN_FLAGS} -O3")
  add_definitions(-DDEBUG=0 -DNDEBUG)
endif()


###########################################################################
## boost
###########################################################################
message(STATUS ${MSG_HEAD} "Searching for boost ...")
set (BOOST_INC_DIR "" CACHE PATH 
"Path to boost headers. The path must contain 'boost/shared_ptr.hpp' file.") 

set (BOOST_INC_DIR_F ${BOOST_INC_DIR}) 
if ("${BOOST_INC_DIR_F}" STREQUAL "" AND 
  (NOT ("${ALL_EXTERNS_PATH}" STREQUAL "")))
  set (BOOST_INC_DIR_F ${ALL_EXTERNS_PATH}/include)
endif()

if (NOT BOOST_INC_DIR_F)
  message(STATUS ${MSG_HEAD} "User did not specify any boost path."
    " Searching system.")

  set (BOOST_DIR_BOOL) ## false
  CHECK_INCLUDE_FILE_CXX(boost/shared_ptr.hpp BOOST_DIR_BOOL
                         ${CMAKE_REQUIRED_FLAGS})
  message(STATUS ${MSG_HEAD} "boost dir. found in system = ${BOOST_DIR_BOOL}") 

  if (NOT BOOST_DIR_BOOL)
    message(FATAL_ERROR " ${MSG_HEAD}" 
      "Cannot find Boost headers in the system. "
      "Please specify a location of boost directory.")
  endif()

elseif (EXISTS ${BOOST_INC_DIR_F}/boost/shared_ptr.hpp)
  message(STATUS ${MSG_HEAD} "Using boost headers from ${BOOST_INC_DIR_F}")
  include_directories(${BOOST_INC_DIR_F})

else()
  message(FATAL_ERROR " ${MSG_HEAD}"
    "${BOOST_INC_DIR_F}/boost/shared_ptr.hpp not found in BOOST_INC_DIR_F")

endif()

###########################################################################
## base
###########################################################################
add_subdirectory(src/base)

###########################################################################
## Bqpd
###########################################################################
set (BQPD_LIB_DIR)          ## NULL
set (LINK_BQPD)         ## NULL
message(STATUS ${MSG_HEAD} "Searching for bqpd ...")

set (BQPD_LIB_DIR "" CACHE PATH 
  "Full path to libbqpd.a library directory, e.g. /home/path/to/third-party/lib")
set (BQPD_LIB_DIR_F ${BQPD_LIB_DIR})

if (("${BQPD_LIB_DIR_F}" STREQUAL "") AND 
  (NOT ("${ALL_EXTERNS_PATH}" STREQUAL "")))
  set (BQPD_LIB_DIR_F ${ALL_EXTERNS_PATH}/lib)
endif()
message(STATUS ${MSG_HEAD} "BQPD_LIB_DIR_F is set to = ${BQPD_LIB_DIR_F}")
if (BQPD_LIB_DIR_F)
  if (EXISTS "${BQPD_LIB_DIR_F}")
    message(STATUS ${MSG_HEAD} "BQPD library directory exists.")
    add_subdirectory(src/engines/Bqpd)
    set (LINK_BQPD 1)
    set (BQPD_LIBS "mntrbqpd" "bqpd")
  else()
    message(FATAL_ERROR " ${MSG_HEAD} BQPD_LIB_DIR_F ${BQPD_LIB_DIR_F} not found.")
  endif()
endif()

if (LINK_BQPD)
  message(STATUS ${MSG_HEAD} "Link BQPD? Yes.")
else()
  set (BQPD_LIB_DIR_F)
  message(STATUS ${MSG_HEAD} "Link BQPD? No.")
endif()


###########################################################################
## Filter-SQP 
###########################################################################
set (FILTER_LIB_DIR)          ## NULL
set (LINK_FILTER_SQP)         ## NULL

message(STATUS ${MSG_HEAD} "Searching for Filter-SQP ...")
set (FILTER_LIB_DIR "" CACHE PATH 
  "Path to Filter-SQP library. This directory must have a libfilter file.") 
set (FILTER_LIB_DIR_F ${FILTER_LIB_DIR})

if (("${FILTER_LIB_DIR_F}" STREQUAL "") AND 
    (NOT ("${ALL_EXTERNS_PATH}" STREQUAL "")))
    set (FILTER_LIB_DIR_F ${ALL_EXTERNS_PATH}/lib)
endif()

message(STATUS ${MSG_HEAD} "FILTER_LIB_DIR_F is set to = ${FILTER_LIB_DIR_F}")
if (FILTER_LIB_DIR_F)
  if (EXISTS "${FILTER_LIB_DIR_F}")
    message(STATUS ${MSG_HEAD} "Filter-SQP library directory exists.")
    add_subdirectory(src/engines/FilterSQP)
    set (LINK_FILTER_SQP 1)
    set (FILTER_SQP_LIBS "mntrfiltersqp" "filtersqp")
  else()
    message(FATAL_ERROR " ${MSG_HEAD} FILTER_LIB_DIR_F "
    "${FILTER_LIB_DIR_F} not found.")
  endif()
endif()

if (LINK_FILTER_SQP)
  message(STATUS ${MSG_HEAD} "Link Filter-SQP? Yes.")
else()
  set (FILTER_LIB_DIR_F)
  message(STATUS ${MSG_HEAD} "Link Filter-SQP? No.")
endif()

###########################################################################

set (USE_PKGCON_BOOL TRUE CACHE BOOL 
"If true, try to use pkg-config to find library flags.") 

###########################################################################
## Ipopt
###########################################################################
set (IPOPT_INC_DIR)          ## NULL
set (IPOPT_LIB_DIR)          ## NULL
set (LINK_IPOPT)             ## NULL
set (IPOPT_LIBS)             ## NULL

message(STATUS ${MSG_HEAD} "Searching for Ipopt ...")
set (IPOPT_INC_DIR "" CACHE PATH 
  "Path to Ipopt headers. It must have coin/IpIpoptApplication.hpp file.") 
set (IPOPT_INC_DIR_F ${IPOPT_INC_DIR})
if (("${IPOPT_INC_DIR_F}" STREQUAL "") AND
    (NOT ("${ALL_EXTERNS_PATH}" STREQUAL "")))
  set (IPOPT_INC_DIR_F "${ALL_EXTERNS_PATH}/include")
endif()


set (IPOPT_LIB_DIR "" CACHE PATH 
  "Path to Ipopt library. It must have a libipopt file.") 
set (IPOPT_LIB_DIR_F ${IPOPT_LIB_DIR})
if (("${IPOPT_LIB_DIR_F}" STREQUAL "") AND
       (NOT ("${ALL_EXTERNS_PATH}" STREQUAL "")))
  set (IPOPT_LIB_DIR_F "${ALL_EXTERNS_PATH}/lib")
endif()

message(STATUS ${MSG_HEAD} "IPOPT_INC_DIR_F is set to ${IPOPT_INC_DIR_F}")
message(STATUS ${MSG_HEAD} "IPOPT_LIB_DIR_F is set to ${IPOPT_LIB_DIR_F}")

if (IPOPT_INC_DIR_F)
  ## complain if header not found
  if (NOT EXISTS "${IPOPT_INC_DIR_F}/coin/IpIpoptApplication.hpp")
    message(FATAL_ERROR " ${MSG_HEAD}" 
            "${IPOPT_INC_DIR_F}/coin/IpIpoptApplication.hpp not found.")
  endif()

  ## complain if library not specified
  if (NOT IPOPT_LIB_DIR_F)
    message(FATAL_ERROR " ${MSG_HEAD}" 
            "IPOPT_LIB_DIR must be set along with IPOPT_INC_DIR")
  endif()

  ## check if library found
  if (EXISTS ${IPOPT_LIB_DIR_F})
    message(STATUS ${MSG_HEAD} "Ipopt library directory exists.")
    add_subdirectory(src/engines/Ipopt)
    set (LINK_IPOPT 1)
    if (${BUILD_SHARED_LIBS})
      set (CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH}:${IPOPT_LIB_DIR_F})
    endif()

    set (MNTR_IPOPT_PC_LIBRARIES)
    if (MNTR_HAVE_PKGCON)
       set (ENV{PKG_CONFIG_PATH}
	   "${IPOPT_LIB_DIR_F}/pkgconfig:$ENV{PKG_CONFIG_PATH}")
       pkg_check_modules(MNTR_IPOPT_PC ipopt)
       message(STATUS ${MSG_HEAD} "Ipopt libraries: "
	       "${MNTR_IPOPT_PC_LIBRARIES}")
       set (IPOPT_LIBS "mntripopt" ${MNTR_IPOPT_PC_LIBRARIES})
    endif()

    if ((NOT (MNTR_HAVE_PKGCON)) OR
	("${MNTR_IPOPT_PC_LIBRARIES}" STREQUAL ""))
       set (IPOPT_LIBS "mntripopt" "ipopt")
    endif()

  else()
    message(FATAL_ERROR " ${MSG_HEAD} "
            "IPOPT_LIB_DIR_F ${IPOPT_LIB_DIR_F} not found.")
  endif()
elseif (IPOPT_LIB_DIR_F)
    message(FATAL_ERROR " ${MSG_HEAD}" 
            "IPOPT_INC_DIR must be set along with IPOPT_LIB_DIR")
endif()

if (LINK_IPOPT)
  message(STATUS ${MSG_HEAD} "Link IPOPT? Yes.")
else()
  set (IPOPT_INC_DIR_F)
  set (IPOPT_LIB_DIR_F)
  message(STATUS ${MSG_HEAD} "Link IPOPT? No.")
endif()

###########################################################################
## OSI-LP 
###########################################################################
set (OSI_INC_DIR)          ## NULL
set (OSI_LIB_DIR)          ## NULL
set (CLP_INC_DIR)          ## NULL
set (CLP_LIB_DIR)          ## NULL
set (CPX_INC_DIR)          ## NULL
set (GRB_INC_DIR)          ## NULL
set (OSI_LIBS)             ## NULL
set (LINK_OSI)             ## NULL


message(STATUS ${MSG_HEAD} "Searching for OsiLP ...")
set (OSI_INC_DIR "" CACHE PATH 
  "Path to OsiSolver headers. It must have coin/OsiSolverInterface.hpp file.") 

set (OSI_INC_DIR_F ${OSI_INC_DIR})
if (("${OSI_INC_DIR_F}" STREQUAL "") AND
    (NOT ("${ALL_EXTERNS_PATH}" STREQUAL "")))
  set (OSI_INC_DIR_F "${ALL_EXTERNS_PATH}/include")
endif()


set (OSI_LIB_DIR "" CACHE PATH 
  "Path to OsiSolver library. It must have libOsi.") 

set (OSI_LIB_DIR_F ${OSI_LIB_DIR})
if (("${OSI_LIB_DIR_F}" STREQUAL "") AND
    (NOT ("${ALL_EXTERNS_PATH}" STREQUAL "")))
  set (OSI_LIB_DIR_F "${ALL_EXTERNS_PATH}/lib")
endif()

message(STATUS ${MSG_HEAD} "OSI_LIB_DIR_F is set to ${OSI_LIB_DIR_F}")
message(STATUS ${MSG_HEAD} "OSI_INC_DIR_F is set to ${OSI_INC_DIR_F}")

if (OSI_INC_DIR_F)
  ## complain if header not found
  if (NOT EXISTS "${OSI_INC_DIR_F}/coin/OsiSolverInterface.hpp")
    message(FATAL_ERROR " ${MSG_HEAD}" 
            "${OSI_INC_DIR_F}/coin/OsiSolverInterface.hpp not found.")
  endif()

  ## complain if library not specified
  if (NOT OSI_LIB_DIR_F)
    message(FATAL_ERROR " ${MSG_HEAD}" 
            "OSI_LIB_DIR must be set along with OSI_INC_DIR")
  endif()

  ## complain if library not specified
  if (NOT (EXISTS ${OSI_LIB_DIR_F}))
    message(FATAL_ERROR " ${MSG_HEAD} "
            "OSI_LIB_DIR_F ${OSI_LIB_DIR_F} not found.")
  endif()

  message(STATUS ${MSG_HEAD} "OSI library directory exists.")
  set (LINK_OSI 1)

  if (${BUILD_SHARED_LIBS})
    set (CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH}:${OSI_LIB_DIR_F})
  endif()

  set (OSI_LIBS mntrosilp)

elseif (OSI_LIB_DIR_F)
    message(FATAL_ERROR " ${MSG_HEAD}" 
            "OSI_INC_DIR must be set along with OSI_LIB_DIR")
endif()

if (LINK_OSI)
  message(STATUS ${MSG_HEAD} "Link OSI? Yes.")
else()
  set (OSI_INC_DIR_F)
  set (OSI_LIB_DIR_F)
  message(STATUS ${MSG_HEAD} "Link OSI? No.")
endif()


###########################################################################
## CLP
###########################################################################

message(STATUS ${MSG_HEAD} "Searching for CLP ...")
OPTION(OSICLP "Use Clp with OSI" OFF)
set (LINK_CLP 0)
set (OSICLP_F)
set (CLP_INC_DIR_F)

set (OSICLP_F ${OSICLP})
if (NOT ("${ALL_EXTERNS_PATH}" STREQUAL ""))
  set (OSICLP_F ON)
endif()

set (CLP_INC_DIR "" CACHE PATH 
     "Path to CLP headers. It must have coin/ClpModel.hpp file. Defaults to OSI_INC_DIR") 

if (OSICLP_F)
  set (CLP_INC_DIR_F ${OSI_INC_DIR_F})
endif()
if ((NOT ("${CLP_INC_DIR}" STREQUAL "")))
  set (CLP_INC_DIR_F ${CLP_INC_DIR})
endif()


set (CLP_LIB_DIR "" CACHE PATH 
     "Path to CLP libraries. It must have libClp file. Defaults to OSI_LIB_DIR") 
set (CLP_LIB_DIR_F)
if (OSICLP_F)
  set (CLP_LIB_DIR_F ${OSI_LIB_DIR_F})
endif()
if ((NOT ("${CLP_LIB_DIR}" STREQUAL "")))
  set (CLP_LIB_DIR_F ${CLP_LIB_DIR})
endif()

message(STATUS ${MSG_HEAD} "CLP_LIB_DIR_F is set to ${CLP_LIB_DIR_F}")
message(STATUS ${MSG_HEAD} "CLP_INC_DIR_F is set to ${CLP_INC_DIR_F}")

if (NOT ("${CLP_INC_DIR_F}" STREQUAL ""))
  ## complain if header not found
  if (NOT EXISTS "${CLP_INC_DIR_F}/coin/ClpModel.hpp")
    message(FATAL_ERROR " ${MSG_HEAD}" 
            "${CLP_INC_DIR_F}/coin/ClpModel.hpp not found.")
  endif()

  ## complain if library not specified
  if (NOT CLP_LIB_DIR_F)
    message(FATAL_ERROR " ${MSG_HEAD}" 
            "CLP_LIB_DIR must be set along with CLP_INC_DIR")
  endif()

  ## complain if library not specified
  if (NOT (EXISTS ${CLP_LIB_DIR_F}))
    message(FATAL_ERROR " ${MSG_HEAD} "
            "CLP_LIB_DIR_F ${CLP_LIB_DIR_F} not found.")
  endif()

  message(STATUS ${MSG_HEAD} "CLP library directory exists.")
  set (LINK_CLP 1)
  set (LINK_OSI 1)

  if (${BUILD_SHARED_LIBS})
    set (CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH}:${CLP_LIB_DIR_F})
  endif()

  set (MNTR_OSI_PC_LIBRARIES)
  if (MNTR_HAVE_PKGCON)
    set (ENV{PKG_CONFIG_PATH}
      "${CLP_LIB_DIR_F}/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    pkg_check_modules(MNTR_OSI_PC osi-clp)
    message(STATUS ${MSG_HEAD} "Osi CLP libraries: "
      "${MNTR_OSI_PC_LIBRARIES}")
    set (OSI_LIBS mntrosilp ${OSI_LIBS} ${MNTR_OSI_PC_LIBRARIES})
  endif()

  if ((NOT (MNTR_HAVE_PKGCON)) OR ("${MNTR_OSI_PC_LIBRARIES}" STREQUAL ""))
    set (OSI_LIBS mntrosilp ${OSI_LIBS} OsiClp Osi Clp CoinUtils)
  endif()

elseif (CLP_LIB_DIR_F)
    message(FATAL_ERROR " ${MSG_HEAD}" 
            "CLP_INC_DIR must be set along with CLP_LIB_DIR")
endif()

if (LINK_CLP)
  message(STATUS ${MSG_HEAD} "Link CLP? Yes.")
  add_definitions("-DMNTROSICLP=1")
else()
  message(STATUS ${MSG_HEAD} "Link CLP? No.")
  add_definitions("-DMNTROSICLP=0")
endif()

###########################################################################
## CPLEX
###########################################################################
message(STATUS ${MSG_HEAD} "Searching for Cplex ...")
set (LINK_CPX 0)
set (CPX_INC_DIR "" CACHE PATH 
     "Path to Cplex headers. It must have cplex.h file.") 

if (CPX_INC_DIR)
  ## complain if header not found
  if (NOT EXISTS "${CPX_INC_DIR}/cplex.h")
    message(FATAL_ERROR " ${MSG_HEAD} "
            "cplex.h not found at ${CPX_INC_DIR} not found.")
  endif()
  set (LINK_CPX 1)
  set (MNTR_OSI_PC2_LIBRARIES)
  if (MNTR_HAVE_PKGCON)
    set (ENV{PKG_CONFIG_PATH}
      "${OSI_LIB_DIR_F}/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    pkg_check_modules(MNTR_OSI_PC2 osi-cplex)
    message(STATUS ${MSG_HEAD} "Osi Cplex libraries: "
      "${MNTR_OSI_PC2_LIBRARIES}")
    set (OSI_LIBS ${OSI_LIBS} ${MNTR_OSI_PC2_LIBRARIES})
  endif()

  if ((NOT (MNTR_HAVE_PKGCON)) OR ("${MNTR_OSI_PC2_LIBRARIES}" STREQUAL ""))
    set (OSI_LIBS ${OSI_LIBS} OsiCpx Osi CoinUtils)
  endif()
endif()

message(STATUS ${MSG_HEAD} "CPX_INC_DIR is set to ${CPX_INC_DIR}")


if (LINK_CPX)
  message(STATUS ${MSG_HEAD} "Link Cplex? Yes.")
  add_definitions("-DMNTROSICPX=1")
else()
  message(STATUS ${MSG_HEAD} "Link Cplex? No.")
  add_definitions("-DMNTROSICPX=0")
endif()


###########################################################################
## Gurobi
###########################################################################
message(STATUS ${MSG_HEAD} "Searching for Gurobi ...")
set (LINK_GRB 0)
set (GRB_INC_DIR "" CACHE PATH 
     "Path to Gurobi headers. It must have gurobi_c.h file.") 

if (GRB_INC_DIR)
  ## complain if header not found
  if (NOT EXISTS "${GRB_INC_DIR}/gurobi_c.h")
    message(FATAL_ERROR " ${MSG_HEAD} "
            "gurobi_c.h not found at ${GRB_INC_DIR} not found.")
  endif()
  set (LINK_GRB 1)
  set (MNTR_OSI_PC3_LIBRARIES)
  if (MNTR_HAVE_PKGCON)
    set (ENV{PKG_CONFIG_PATH}
      "${OSI_LIB_DIR_F}/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    set (MNTR_OSI_PC3_LIBRARIES)
    pkg_check_modules(MNTR_OSI_PC osi-gurobi)
    message(STATUS ${MSG_HEAD} "Osi Gurobi libraries: "
      "${MNTR_OSI_PC3_LIBRARIES}")
    set (OSI_LIBS ${OSI_LIBS} ${MNTR_OSI_PC3_LIBRARIES})
  endif()

  if ((NOT (MNTR_HAVE_PKGCON)) OR ("${MNTR_OSI_PC3_LIBRARIES}" STREQUAL ""))
    set (OSI_LIBS ${OSI_LIBS} OsiGrb Osi CoinUtils)
  endif()
endif()

message(STATUS ${MSG_HEAD} "GRB_INC_DIR is set to ${GRB_INC_DIR}")


if (LINK_GRB)
  message(STATUS ${MSG_HEAD} "Link Gurobi? Yes.")
  add_definitions("-DMNTROSIGRB=1")
else()
  message(STATUS ${MSG_HEAD} "Link Gurobi? No.")
  add_definitions("-DMNTROSIGRB=0")
endif()


###########################################################################
## Osi revisited
###########################################################################
if (LINK_OSI)
  add_subdirectory(src/engines/OsiLP)
endif()



###########################################################################
## qpOASES
###########################################################################
set (QPOASES_LIB)          ## NULL
set (LINK_QPOASES)         ## NULL

message(STATUS ${MSG_HEAD} "Searching for qpOASES ...")
set (QPOASES_INC_DIR "" CACHE PATH
  "Path to qpOASES headers. It must have subdirectory qpOASES with file QProblem.hpp.")
set (QPOASES_LIB_DIR "" CACHE PATH
  "Path to qpOASES library. It must have a libqpOASES.a file.")
message(STATUS ${MSG_HEAD} "QPOASES_INC_DIR is set to ${QPOASES_INC_DIR}")
message(STATUS ${MSG_HEAD} "QPOASES_LIB_DIR is set to ${QPOASES_LIB_DIR}")

if (QPOASES_INC_DIR)
  if (NOT EXISTS "${QPOASES_INC_DIR}/qpOASES/QProblem.hpp")
    message(FATAL_ERROR " ${MSG_HEAD}"
            "${QPOASES_INC_DIR}/qpOASES/QProblem.hpp not found.")
  endif()
  if (NOT QPOASES_LIB_DIR)
    message(FATAL_ERROR " ${MSG_HEAD}"
            "QPOASES_LIB_DIR must be set along with QPOASES_INC_DIR")
  endif()
  if (EXISTS ${QPOASES_LIB_DIR})
    message(STATUS ${MSG_HEAD} "qpOASES library directory exists.")
    add_subdirectory(src/engines/qpOASES)
    set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH}:${QPOASES_LIB_DIR})
    set (LINK_QPOASES 1)
  else()
    message(FATAL_ERROR " ${MSG_HEAD} "
            "QPOASES_LIB_DIR ${QPOASES_LIB_DIR} not found.")
  endif()
elseif (QPOASES_INC_DIR)
  if (QPOASES_LIB_DIR)
    message(FATAL_ERROR " ${MSG_HEAD}"
            "QPOASES_INC_DIR must be set along with QPOASES_LIB_DIR")
  endif()
endif()

if (LINK_QPOASES)
  message(STATUS ${MSG_HEAD} "Link qpOASES? Yes.")
else()
  set (QPOASES_INC_DIR)
  set (QPOASES_LIB_DIR)
  message(STATUS ${MSG_HEAD} "Link qpOASES? No.")
endif()

###########################################################################
## Engine Factory
###########################################################################
add_subdirectory(src/engines)


###########################################################################
## ASL
###########################################################################
set(ASL_INC_DIR)        ## NULL
set(ASL_LIB_DIR)        ## NULL
set(LINK_ASL)       ## NULL
set(ASL_LIBS)       ## NULL

message(STATUS ${MSG_HEAD} "Searching for ASL ...")
set(ASL_INC_DIR "" CACHE PATH 
  "Path to ASL headers. It must have asl.h") 
set(ASL_LIB_DIR "" CACHE PATH 
  "Path to ASL library. It must have amplsolver.a") 

set (ASL_INC_DIR_F ${ASL_INC_DIR})
set (ASL_LIB_DIR_F ${ASL_LIB_DIR})

if (("${ASL_INC_DIR_F}" STREQUAL "") AND
    (NOT ("${ALL_EXTERNS_PATH}" STREQUAL "")))
  set (ASL_INC_DIR_F "${ALL_EXTERNS_PATH}/include")
  set (ASL_LIB_DIR_F "${ALL_EXTERNS_PATH}/lib")
endif()

if (ASL_INC_DIR_F)
  set(LINK_ASL 1)
  if (NOT EXISTS "${ASL_INC_DIR_F}/asl.h")
    message(FATAL_ERROR " ${MSG_HEAD} ${ASL_INC_DIR_F}/asl.h not found.")
    set(LINK_ASL 0)
  endif()
  if (NOT EXISTS "${ASL_LIB_DIR_F}/amplsolver.a")
    message(FATAL_ERROR " ${MSG_HEAD} ${ASL_LIB_DIR_F}/amplsolver.a not found.")
    set(LINK_ASL 0)
  endif()
endif()

if (LINK_ASL)
  set(ASL_LIBS ${ASL_LIBS} "${ASL_LIB_DIR_F}/amplsolver.a" "dl")
  message(STATUS ${MSG_HEAD} "Using AMPL solver interface at"
    ${ASL_INC_DIR_F})
  message(STATUS ${MSG_HEAD} "ASL_LIBS =  ${ASL_LIBS}")
  message(STATUS ${MSG_HEAD} "Link AMPL-ASL? Yes.")
  add_subdirectory(src/interfaces/ampl)
  add_subdirectory(src/algorithms)
else()
  message(STATUS ${MSG_HEAD} "Link AMPL-ASL? No.")
endif()

###########################################################################
## Testing
###########################################################################
set(CPPUNIT_INC_DIR)        ## NULL
set(CPPUNIT_LIB_DIR)        ## NULL
set(LINK_CPPUNIT)           ## NULL

set(CPPUNIT_INC_DIR "" CACHE PATH 
  "Path to CppUnit headers. It must have cppunit/TestCase.h") 
set (CPPUNIT_INC_DIR_F ${CPPUNIT_INC_DIR})
if (("${CPPUNIT_INC_DIR_F}" STREQUAL "") AND
    (NOT ("${ALL_EXTERNS_PATH}" STREQUAL "")))
  set (CPPUNIT_INC_DIR_F "${ALL_EXTERNS_PATH}/include")
endif()


set(CPPUNIT_LIB_DIR "" CACHE PATH 
  "Path to CppUnit library. It must have cppunit library file.") 
set (CPPUNIT_LIB_DIR_F ${CPPUNIT_LIB_DIR})
if (("${CPPUNIT_LIB_DIR_F}" STREQUAL "") AND
    (NOT ("${ALL_EXTERNS_PATH}" STREQUAL "")))
  set (CPPUNIT_LIB_DIR_F "${ALL_EXTERNS_PATH}/lib")
endif()


message(STATUS ${MSG_HEAD} "Searching for cppunit ...")
if (CPPUNIT_INC_DIR_F)
  if (NOT EXISTS "${CPPUNIT_INC_DIR_F}/cppunit/TestCase.h")
    message(FATAL_ERROR 
            " ${MSG_HEAD} ${CPPUNIT_INC_DIR_F}/cppunit/TestCase.h not found.")
  endif()
  if (NOT CPPUNIT_LIB_DIR_F)
    message(FATAL_ERROR " ${MSG_HEAD} "
            "CPPUNIT_LIB_DIR must be set along with CPPUNIT_INC_DIR")
  endif()
  if (EXISTS ${CPPUNIT_LIB_DIR_F})
    add_definitions("-DCPPUNIT_STD_NEED_ALLOCATOR=0")
    message(STATUS ${MSG_HEAD} "found CPPUNIT library directory.")
    add_subdirectory(src/testing)
    set (LINK_CPPUNIT 1)
  else()
    message(FATAL_ERROR " ${MSG_HEAD} "
            "CPPUNIT_LIB_DIR ${CPPUNIT_LIB_DIR_F} not found.")
  endif()
elseif (CPPUNIT_LIB_DIR_F)
    message(FATAL_ERROR " ${MSG_HEAD}" 
            "CPPUNIT_INC_DIR must be set along with CPPUNIT_LIB_DIR")
endif()


if (LINK_CPPUNIT)
  message(STATUS ${MSG_HEAD} "Link CppUnit? Yes.")
  # I don't know what this flag does, but if not set, compiling CPPUNIT gets
  # many warnings. -Ashu
else()
  set (CPPUNIT_INC_DIR_F)
  set (CPPUNIT_LIB_DIR_F)
  message(STATUS ${MSG_HEAD} "Link CppUnit? No.")
endif()


## for make test, copy instances, cd to src testing, and run unittest
add_custom_target(utest 
  ${CMAKE_COMMAND} -E remove_directory ./src/testing/instances
  COMMAND ${CMAKE_COMMAND} -E copy_directory 
  ${PROJECT_SOURCE_DIR}/src/testing/instances ./instances 
  COMMAND unittest all
  WORKING_DIRECTORY src/testing)


###########################################################################
## Any other extra libs that user may need to link to
###########################################################################
set(MNTR_EXTRA_LIBS "" CACHE STRING 
  "Additional libraries that you may want to link to. e.g. -lbz2 -llapack") 

###########################################################################
## Documentation using doxygen will be installed in ./doxygen
###########################################################################
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory doxygen)
add_custom_target(doc 
  ${CMAKE_COMMAND} -E remove_directory html
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/doc/doxyfile .
  COMMAND echo "INPUT = ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/doc" >> doxyfile
  COMMAND echo "EXCLUDE = ${PROJECT_SOURCE_DIR}/src/testing" >> doxyfile 
  COMMAND echo "HTML_FOOTER = ${PROJECT_SOURCE_DIR}/doc/footer.html" >> doxyfile
  COMMAND doxygen doxyfile 
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/doc/logo/small.jpg html
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/doc/logo/tiny.jpg html
  WORKING_DIRECTORY doxygen)

message (STATUS ${MSG_HEAD} 
  "rpath for installation is set to ${CMAKE_INSTALL_RPATH}")

###########################################################################
## hide unused options
###########################################################################
set(EXECUTABLE_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}" CACHE INTERNAL "")
set(LIBRARY_OUTPUT_PATH "${LIBRARY_OUTPUT_PATH}" CACHE INTERNAL "")

