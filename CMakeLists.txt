## Minotaur CMake file

project(MINOTAUR CXX C Fortran)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0 FATAL_ERROR) 
cmake_policy(SET CMP0003 NEW)
set(CMAKE_CXX_STANDARD 17)
## Fortran is required for Bqpd, Filter.
## C is required for FortranCInterface.
INCLUDE(CheckIncludeFile)
INCLUDE(CheckIncludeFileCXX)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckLibraryExists)
INCLUDE(FortranCInterface)
INCLUDE(FindSubversion)
INCLUDE(FindPkgConfig OPTIONAL RESULT_VARIABLE MNTR_PKGCON)

#set flags
set (MSG_HEAD "Minotaur config: ")  ## must preceed every message.

enable_language(CXX C Fortran)
include(FortranCInterface)
FortranCInterface_VERIFY(CXX)

if("${CMAKE_HOST_SYSTEM}" MATCHES ".*Windows.*")
  set(SEP "\\;")
else() # e.g. Linux
  set(SEP ":")
endif()
message(STATUS MSG_HEAD "Path separator set to " ${SEP})

## Stop if we are compiling in the minotaur-root directory.
if ((EXISTS "${PROJECT_BINARY_DIR}/src/base/Types.h") OR 
  ("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}"))
  message(FATAL_ERROR
  ${MSG_HEAD} "In-source builds are not allowed. Please create a build directory and compile there.")
endif()

## Enable compiler warnings
OPTION(COMPILER_WARN "Turn compiler warnings on/off " ON)
if (${COMPILER_WARN})
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic \
  -Wmissing-include-dirs -Wunused -Wextra -Wundef -Wshadow -Wredundant-decls \
  -Woverloaded-virtual -Wnon-virtual-dtor -Wmaybe-uninitialized")
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
set (THIRD_PARTY_PATH "" CACHE PATH "Path to the directory where all minotaur external libraries and headers are available (/path/to/third-party)")

# if THIRD_PARTY_PATH is entered, convert it to absolute path
set(TP_ABS_PATH)   ## NULL
if (NOT ("${THIRD_PARTY_PATH}" STREQUAL ""))
  if(NOT IS_ABSOLUTE ${THIRD_PARTY_PATH})
    set(TP_ABS_PATH "${CMAKE_CURRENT_BINARY_DIR}/${THIRD_PARTY_PATH}")
  else()
    set(TP_ABS_PATH ${THIRD_PARTY_PATH})
  endif()
endif()

message (STATUS ${MSG_HEAD} "Path to third-party = ${TP_ABS_PATH}")

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
    if (IS_ABSOLUTE ${CMAKE_INSTALL_PREFIX})
      set(MNTR_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
    else()
      set(MNTR_INSTALL_RPATH ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}/lib)
    endif()
  endif()

###########################################################################
## OpenMP is a requirement 
###########################################################################
FIND_PACKAGE(OpenMP)
if(OPENMP_FOUND)
  message(STATUS ${MSG_HEAD} "OpenMP support detected.")
  add_definitions(-DUSE_OPENMP=1)	
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
else()
  message(FATAL_ERROR
  ${MSG_HEAD} "OpenMP is required, but was not found")
endif()

###########################################################################
## getrusage
###########################################################################
set (MINOTAUR_RUSAGE) ## NULL
CHECK_FUNCTION_EXISTS(getrusage MINOTAUR_RUSAGE)
message (STATUS ${MSG_HEAD} "Is rusage available = ${MINOTAUR_RUSAGE}")

###########################################################################
## git revision number as returned by git describe
###########################################################################
set (MINOTAUR_GIT_VERSION "") 
get_filename_component(MNTR_ABS_PATH "${PROJECT_SOURCE_DIR}" ABSOLUTE)
message (STATUS ${MSG_HEAD} "checking git in = ${MNTR_ABS_PATH}")
if (EXISTS "${MNTR_ABS_PATH}/.git")
  message (STATUS ${MSG_HEAD} "found .git in ${MNTR_ABS_PATH}")
  FIND_PACKAGE(Git QUIET)
  if (GIT_FOUND)
    execute_process( COMMAND ${GIT_EXECUTABLE} describe 
	    OUTPUT_VARIABLE MINOTAUR_GIT_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE)
  else()
    message (STATUS ${MSG_HEAD} "CMake git package not found.")
    message (STATUS ${MSG_HEAD} "Not finding git revision. Setting manually.")
    set (MINOTAUR_GIT_VERSION "v0.2-182-gc80398c") 
    message (STATUS ${MSG_HEAD} "Setting it manually to v0.2-182-gc80398c")
  endif()
else()
    message (STATUS ${MSG_HEAD} "Not finding .git directory. Setting manually.")
    set (MINOTAUR_GIT_VERSION "v0.2-182-gc80398c") 
endif()
if ("${MINOTAUR_GIT_VERSION}" STREQUAL "")
  set (MINOTAUR_GIT_VERSION "v0.0-unknown")
endif()
message (STATUS ${MSG_HEAD} "git version = ${MINOTAUR_GIT_VERSION}")


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
## MDBUG options
###########################################################################
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  add_definitions(-DMDBUG=1) #Minotaur Debug Flag MDBUG
else()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
  set (CMAKE_FORTRAN_FLAGS "${CMAKE_FORTRAN_FLAGS} -O3")
  add_definitions(-DMDBUG=0 -DNDEBUG)
endif()

###########################################################################
# Choose between FILTERSQP and UNO
###########################################################################
# Define options
OPTION(USE_UNO "Enable UNO solver" OFF)
OPTION(USE_FILTERSQP "Enable FilterSQP solver" ON)
message(STATUS ${MSG_HEAD} "Choosing between FILTERSQP and UNO...")
if (NOT ${USE_FILTERSQP} AND NOT ${USE_UNO})
  message(STATUS ${MSG_HEAD} "Both USE_FILTERSQP and USE_UNO are not set.")
  set(USE_FILTERSQP FALSE) # Enable FilterSQP
  set(USE_UNO FALSE) # Disable UNO if FilterSQP is enabled
elseif (${USE_FILTERSQP} AND ${USE_UNO})
  message(STATUS ${MSG_HEAD} "Both USE_FILTERSQP and USE_UNO are set. Prioritizing FilterSQP.")
  set(USE_UNO FALSE) # Disable UNO if FilterSQP is enabled
elseif (${USE_FILTERSQP})
  message(STATUS ${MSG_HEAD} "USE_FILTERSQP is set and USE_UNO is not set. Solver in use: FilterSQP")
  set(USE_UNO FALSE) # Disable UNO if FilterSQP is enabled
elseif (${USE_UNO})
  message(STATUS ${MSG_HEAD} "USE_FILTERSQP is not set and USE_UNO is set. Solver in use: UNO")
  set(USE_FILTERSQP FALSE) # Disable FilterSQP if UNO is enabled
endif()
###########################################################################

###########################################################################
## Filter-SQP and BQPD
###########################################################################
set (USE_FILTERSQP TRUE CACHE BOOL 
  "If true, link to FilterSQP and BQPD solvers")
set (FILTER_LIB_DIR)          ## NULL
set (LINK_FILTER_SQP)         ## NULL
set (FILTER_SQP_LIBS)         ## NULL
#set (USE_FILTERSQP 0)
if (${USE_FILTERSQP})
  message(STATUS ${MSG_HEAD} "Searching for Filter-SQP and BQPD ...")
  set (FILTER_LIB_DIR "" CACHE PATH 
    "Path to Filter-SQP library. This directory must have a libfilter and libbqpd file.") 
  set (FILTER_LIB_DIR_F ${FILTER_LIB_DIR})
  
  if (("${FILTER_LIB_DIR_F}" STREQUAL "") AND 
      (NOT ("${TP_ABS_PATH}" STREQUAL "")))
      set (FILTER_LIB_DIR_F ${TP_ABS_PATH}/lib)
  endif()
  
  message(STATUS ${MSG_HEAD} "FILTER_LIB_DIR_F is set to ${FILTER_LIB_DIR_F}")
  if (FILTER_LIB_DIR_F)
    if (EXISTS "${FILTER_LIB_DIR_F}")
      message(STATUS ${MSG_HEAD} "Filter-SQP library directory exists.")
      set (LINK_FILTER_SQP 1)
      set (FILTER_SQP_LIBS "bqpd" "filtersqp")
    else()
      message(FATAL_ERROR " ${MSG_HEAD} FILTER_LIB_DIR_F "
      "${FILTER_LIB_DIR_F} not found.")
    endif()
  endif()
endif()
  
if (LINK_FILTER_SQP)
  message(STATUS ${MSG_HEAD} "Link Filter-SQP and BQPD? Yes.")
else()
  set (FILTER_LIB_DIR_F)
  message(STATUS ${MSG_HEAD} "Link Filter-SQP and BQPD? No.")
endif()

###########################################################################

set (USE_PKGCON_BOOL TRUE CACHE BOOL 
"If true, try to use pkg-config to find library flags.") 

###########################################################################
## Ipopt
###########################################################################
set (USE_IPOPT TRUE CACHE BOOL "If true, link to IPOPT solver")
set (IPOPT_INC_DIR)          ## NULL
set (IPOPT_LIB_DIR)          ## NULL
set (LINK_IPOPT)             ## NULL
set (IPOPT_LIBS)             ## NULL

if (${USE_IPOPT})
  message(STATUS ${MSG_HEAD} "Searching for Ipopt ...")
  set (IPOPT_INC_DIR "" CACHE PATH 
    "Path to Ipopt headers. It must have coin-or/IpIpoptApplication.hpp file.") 
  set (IPOPT_INC_DIR_F ${IPOPT_INC_DIR})
  if (("${IPOPT_INC_DIR_F}" STREQUAL "") AND
      (NOT ("${TP_ABS_PATH}" STREQUAL "")))
    set (IPOPT_INC_DIR_F "${TP_ABS_PATH}/include")
  endif()
  
  
  set (IPOPT_LIB_DIR "" CACHE PATH 
    "Path to Ipopt library. It must have a libipopt file.") 
  set (IPOPT_LIB_DIR_F ${IPOPT_LIB_DIR})
  if (("${IPOPT_LIB_DIR_F}" STREQUAL "") AND
         (NOT ("${TP_ABS_PATH}" STREQUAL "")))
    set (IPOPT_LIB_DIR_F "${TP_ABS_PATH}/lib")
  endif()
  
  message(STATUS ${MSG_HEAD} "IPOPT_INC_DIR_F is set to ${IPOPT_INC_DIR_F}")
  message(STATUS ${MSG_HEAD} "IPOPT_LIB_DIR_F is set to ${IPOPT_LIB_DIR_F}")
  
  if (IPOPT_INC_DIR_F)
    ## complain if header not found
    if (NOT EXISTS "${IPOPT_INC_DIR_F}/coin-or/IpIpoptApplication.hpp")
      message(FATAL_ERROR " ${MSG_HEAD}" 
              "${IPOPT_INC_DIR_F}/coin-or/IpIpoptApplication.hpp not found.")
    endif()
  
    ## complain if library not specified
    if (NOT IPOPT_LIB_DIR_F)
      message(FATAL_ERROR " ${MSG_HEAD}" 
              "IPOPT_LIB_DIR must be set along with IPOPT_INC_DIR")
    endif()
  
    ## check if library found
    if (EXISTS ${IPOPT_LIB_DIR_F})
      message(STATUS ${MSG_HEAD} "Ipopt library directory exists.")
      set (LINK_IPOPT 1)
      if (${BUILD_SHARED_LIBS})
        set (MNTR_INSTALL_RPATH ${MNTR_INSTALL_RPATH} ${IPOPT_LIB_DIR_F})
      endif()
  
      set (MNTR_IPOPT_PC_LIBRARIES)
      if (MNTR_HAVE_PKGCON)
         set (ENV{PKG_CONFIG_PATH}
             "${IPOPT_LIB_DIR_F}/pkgconfig${SEP}$ENV{PKG_CONFIG_PATH}")
         pkg_check_modules(MNTR_IPOPT_PC ipopt)
         message(STATUS ${MSG_HEAD} "Ipopt libraries: "
  	       "${MNTR_IPOPT_PC_LIBRARIES}")
         message(STATUS ${MSG_HEAD} "pkgconfig path = "
  	       "${IPOPT_LIB_DIR_F}/pkgconfig")
         set (IPOPT_LIBS ${MNTR_IPOPT_PC_LIBRARIES})
      endif()
  
      if ((NOT (MNTR_HAVE_PKGCON)) OR
  	("${MNTR_IPOPT_PC_LIBRARIES}" STREQUAL ""))
         set (IPOPT_LIBS "ipopt")
      endif()
  
    else()
      message(FATAL_ERROR " ${MSG_HEAD} "
              "IPOPT_LIB_DIR_F ${IPOPT_LIB_DIR_F} not found.")
    endif()
  elseif (IPOPT_LIB_DIR_F)
      message(FATAL_ERROR " ${MSG_HEAD}" 
              "IPOPT_INC_DIR must be set along with IPOPT_LIB_DIR")
  endif()
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
    (NOT ("${TP_ABS_PATH}" STREQUAL "")))
  set (OSI_INC_DIR_F "${TP_ABS_PATH}/include")
endif()


set (OSI_LIB_DIR "" CACHE PATH 
  "Path to OsiSolver library. It must have libOsi.") 

set (OSI_LIB_DIR_F ${OSI_LIB_DIR})
if (("${OSI_LIB_DIR_F}" STREQUAL "") AND
    (NOT ("${TP_ABS_PATH}" STREQUAL "")))
  set (OSI_LIB_DIR_F "${TP_ABS_PATH}/lib")
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
    set (MNTR_INSTALL_RPATH ${MNTR_INSTALL_RPATH} ${OSI_LIB_DIR_F})
  endif()

  set (OSI_LIBS)

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
if (NOT ("${TP_ABS_PATH}" STREQUAL ""))
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
    set (MNTR_INSTALL_RPATH ${MNTR_INSTALL_RPATH} ${CLP_LIB_DIR_F})
  endif()

  set (MNTR_OSI_PC_LIBRARIES)
  if (MNTR_HAVE_PKGCON)
    set (ENV{PKG_CONFIG_PATH}
      "${CLP_LIB_DIR_F}/pkgconfig${SEP}$ENV{PKG_CONFIG_PATH}")
    pkg_check_modules(MNTR_OSI_PC osi-clp)
    message(STATUS ${MSG_HEAD} "Osi CLP libraries: "
      "${MNTR_OSI_PC_LIBRARIES}")
    set (OSI_LIBS ${OSI_LIBS} ${MNTR_OSI_PC_LIBRARIES})
  endif()

  if ((NOT (MNTR_HAVE_PKGCON)) OR ("${MNTR_OSI_PC_LIBRARIES}" STREQUAL ""))
    set (OSI_LIBS ${OSI_LIBS} OsiClp Osi Clp CoinUtils)
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
## Uno
###########################################################################
#set (USE_UNO TRUE CACHE BOOL "If true, link to UNO solver")

set (UNO_INC_DIR)          ## NULL
set (UNO_LIB_DIR)          ## NULL
set (LINK_UNO)             ## NULL
set (UNO_LIBS)             ## NULL
#set (CMAKE_CXX_FLAGS1  "${CMAKE_CXX_FLAGS}")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")


if (${USE_UNO})
  message(STATUS ${MSG_HEAD} "Searching for Uno ...")
  set (UNO_INC_DIR "" CACHE PATH 
    "Path to Uno headers. It must have uno/Uno.hpp file.") 
  set (UNO_INC_DIR_F ${UNO_INC_DIR})
  if (("${UNO_INC_DIR_F}" STREQUAL "") AND
      (NOT ("${TP_ABS_PATH}" STREQUAL "")))
    set (UNO_INC_DIR_F "${TP_ABS_PATH}/include/uno")
  endif()
  
  
  set (UNO_LIB_DIR "" CACHE PATH 
    "Path to Uno library. It must have a libuno file.") 
  set (UNO_LIB_DIR_F ${UNO_LIB_DIR})
  if (("${UNO_LIB_DIR_F}" STREQUAL "") AND
        (NOT ("${TP_ABS_PATH}" STREQUAL "")))
    set (UNO_LIB_DIR_F "${TP_ABS_PATH}/lib")
  endif()
  
  message(STATUS ${MSG_HEAD} "UNO_INC_DIR_F is set to ${UNO_INC_DIR_F}")
  message(STATUS ${MSG_HEAD} "UNO_LIB_DIR_F is set to ${UNO_LIB_DIR_F}")
endif()
#   if (UNO_INC_DIR_F)
#     ## complain if header not found
#     if (NOT EXISTS "${UNO_INC_DIR_F}/uno/Uno.hpp")
#       message(FATAL_ERROR " ${MSG_HEAD}" 
#               "${UNO_INC_DIR_F}/uno/Uno.hpp not found.")
#     endif()
  
#     ## complain if library not specified
#     if (NOT UNO_LIB_DIR_F)
#       message(FATAL_ERROR " ${MSG_HEAD}" 
#               "UNO_LIB_DIR must be set along with UNO_INC_DIR")
#     endif()
  
#     ## check if library found
    if (EXISTS ${UNO_LIB_DIR_F})
      message(STATUS ${MSG_HEAD} "Uno library directory exists.")
      set (LINK_UNO 1)
      set (UNO_LIBS "uno" "ma57" "metis" "GKlib" "bqpd" "coinmumps")
      if (${BUILD_SHARED_LIBS})
        set (MNTR_INSTALL_RPATH ${MNTR_INSTALL_RPATH} ${UNO_LIB_DIR_F})
      endif()
  
#       set (MNTR_UNO_PC_LIBRARIES)
#       if (MNTR_HAVE_PKGCON)
#          set (ENV{PKG_CONFIG_PATH}
#              "${UNO_LIB_DIR_F}/pkgconfig${SEP}$ENV{PKG_CONFIG_PATH}")
#          pkg_check_modules(MNTR_UNO_PC UNO)
#          message(STATUS ${MSG_HEAD} "Uno libraries: "
#   	       "${MNTR_UNO_PC_LIBRARIES}")
#          message(STATUS ${MSG_HEAD} "pkgconfig path = "
#   	       "${UNO_LIB_DIR_F}/pkgconfig")
#          set (UNO_LIBS ${MNTR_UNO_PC_LIBRARIES})
#       endif()
  
#       if ((NOT (MNTR_HAVE_PKGCON)) OR
#   	("${MNTR_UNO_PC_LIBRARIES}" STREQUAL ""))
#          set (UNO_LIBS "uno")
#       endif()
  
#     else()
#       message(FATAL_ERROR " ${MSG_HEAD} "
#               "UNO_LIB_DIR_F ${UNO_LIB_DIR_F} not found.")
#     endif()
#   elseif (UNO_LIB_DIR_F)
#       message(FATAL_ERROR " ${MSG_HEAD}" 
#               "UNO_INC_DIR must be set along with UNO_LIB_DIR")
#   endif()
endif()

if (LINK_UNO)
  message(STATUS ${MSG_HEAD} "Link UNO? Yes.")
else()
  set (UNO_INC_DIR_F)
  set (UNO_LIB_DIR_F)
  message(STATUS ${MSG_HEAD} "Link UNO? No.")
endif()

#set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS1})

###########################################################################
## CPLEX
###########################################################################
message(STATUS ${MSG_HEAD} "Searching for Cplex ...")

set (CPX_LIB_DIR)          ## NULL
set (CPX_INC_DIR)          ## NULL
set (LINK_CPX 0)
set (CPX_INC_DIR "" CACHE PATH 
     "Path to Cplex headers. It must have ilcplex/cplex.h file.") 
set (CPX_LIB_DIR "" CACHE PATH 
     "Path to Cplex static library. It must have libcplex.a file.") 
set (CPX_LIB_DIR_F ${CPX_LIB_DIR})

if (CPX_INC_DIR)
  ## complain if header not found
  #if (NOT EXISTS "${CPX_INC_DIR}/cplex.h")
  if (NOT EXISTS "${CPX_INC_DIR}/ilcplex/cplex.h")
    message(FATAL_ERROR " ${MSG_HEAD} "
            "ilcplex/cplex.h not found at ${CPX_INC_DIR}.")
  endif()
  set (LINK_CPX 1)
  set (CPX_INC_DIR_F ${CPX_INC_DIR})

  ## complain if library not specified
  if (NOT CPX_LIB_DIR_F)
    message(FATAL_ERROR " ${MSG_HEAD}" 
	    "CPX_LIB_DIR must be set along with CPX_INC_DIR")
  endif()

  ## check if library found
  if (EXISTS ${CPX_LIB_DIR_F})
    message(STATUS ${MSG_HEAD} "Cplex library directory exists.")
    #add_subdirectory(src/engines/Cplex)
    set (LINK_CPX 1)
    if (${BUILD_SHARED_LIBS})
      set (MNTR_INSTALL_RPATH ${MNTR_INSTALL_RPATH} ${CPX_LIB_DIR_F})
    endif()

    set (CPX_LIBS "${CPX_LIB_DIR_F}/libcplex.a")
    set (MNTR_CPX_PC_LIBRARIES)

  else()
    message(FATAL_ERROR " ${MSG_HEAD} "
	    "CPX_LIB_DIR_F ${CPX_LIB_DIR_F} not found.")
  endif()
elseif (CPX_LIB_DIR_F)
  message(FATAL_ERROR " ${MSG_HEAD}" 
	  "CPX_INC_DIR must be set along with CPX_LIB_DIR")
endif()

message(STATUS ${MSG_HEAD} "CPX_INC_DIR is set to ${CPX_INC_DIR}")

if (LINK_CPX)
  message(STATUS ${MSG_HEAD} "Link Cplex? Yes.")
  #add_definitions("-DIL_STD") # added to Cplex's CMakeList.txt
else()
  set (CPX_INC_DIR_F)
  set (CPX_LIB_DIR_F)
  message(STATUS ${MSG_HEAD} "Link Cplex? No.")
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
      "${OSI_LIB_DIR_F}/pkgconfig${SEP}$ENV{PKG_CONFIG_PATH}")
    set (MNTR_OSI_PC3_LIBRARIES)
    pkg_check_modules(MNTR_OSI_PC osi-gurobi)
    message(STATUS ${MSG_HEAD} "Osi Gurobi libraries: "
      "${MNTR_OSI_PC3_LIBRARIES}")
    set (OSI_LIBS ${OSI_LIBS} ${MNTR_OSI_PC3_LIBRARIES})
  endif()

  if ((NOT (MNTR_HAVE_PKGCON)) OR ("${MNTR_OSI_PC3_LIBRARIES}" STREQUAL ""))
    set (OSI_LIBS ${OSI_LIBS} Osi CoinUtils)
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
#if (LINK_OSI)
#  add_subdirectory(src/engines/OsiLP)
#endif()



###########################################################################
## Cbc
###########################################################################
#set (CBC_INC_DIR)          ## NULL
#set (CBC_LIB_DIR)          ## NULL
#set (LINK_CBC)             ## NULL
#set (CBC_LIBS)             ## NULL
#
#message(STATUS ${MSG_HEAD} "Searching for Cbc ...")
#set (CBC_INC_DIR "" CACHE PATH 
#  "Path to Cbc headers. It must have coin/CbcModel.hpp file.") 
#set (CBC_INC_DIR_F ${CBC_INC_DIR})
#
#if (("${CBC_INC_DIR_F}" STREQUAL "") AND
#    (NOT ("${TP_ABS_PATH}" STREQUAL "")))
#  set (CBC_INC_DIR_F "${TP_ABS_PATH}/include")
#endif()
#
#
#set (CBC_LIB_DIR "" CACHE PATH 
#  "Path to Cbc library. It must have a Cbc library file.") 
#set (CBC_LIB_DIR_F ${CBC_LIB_DIR})
#
#if (("${CBC_LIB_DIR_F}" STREQUAL "") AND
#       (NOT ("${TP_ABS_PATH}" STREQUAL "")))
#  set (CBC_LIB_DIR_F "${TP_ABS_PATH}/lib")
#endif()
#
#message(STATUS ${MSG_HEAD} "CBC_INC_DIR_F is set to ${CBC_INC_DIR_F}")
#message(STATUS ${MSG_HEAD} "CBC_LIB_DIR_F is set to ${CBC_LIB_DIR_F}")
#
#if (CBC_INC_DIR_F)
#  ## complain if header not found
#  if (NOT EXISTS "${CBC_INC_DIR_F}/coin/CbcModel.hpp")
#    message(FATAL_ERROR " ${MSG_HEAD}" 
#	    "${CBC_INC_DIR_F}/coin/CbcModel.hpp not found.")
#  endif()
#
#  ## complain if library not specified
#  if (NOT CBC_LIB_DIR_F)
#    message(FATAL_ERROR " ${MSG_HEAD}" 
#	    "CBC_LIB_DIR must be set along with CBC_INC_DIR")
#  endif()
#
#  ## check if library found
#  if (EXISTS ${CBC_LIB_DIR_F})
#    message(STATUS ${MSG_HEAD} "Cbc library directory exists.")
#    set (LINK_CBC 1)
#    if (${BUILD_SHARED_LIBS})
#      set (MNTR_INSTALL_RPATH ${MNTR_INSTALL_RPATH} ${CBC_LIB_DIR_F})
#    endif()
#
#    set (MNTR_CBC_PC_LIBRARIES)
#    if (MNTR_HAVE_PKGCON)
#       set (ENV{PKG_CONFIG_PATH}
#	       "${CBC_LIB_DIR_F}/pkgconfig${SEP}$ENV{PKG_CONFIG_PATH}")
#       pkg_check_modules(MNTR_CBC_PC cbc)
#       message(STATUS ${MSG_HEAD} "Cbc libraries: "
#	       "${MNTR_CBC_PC_LIBRARIES}")
#       set (CBC_LIBS ${MNTR_CBC_PC_LIBRARIES})
#    endif()
#
#    if ((NOT (MNTR_HAVE_PKGCON)) OR
#	    ("${MNTR_CBC_PC_LIBRARIES}" STREQUAL ""))
#      set (CBC_LIBS "cbc")
#    endif()
#
#  else()
#    message(FATAL_ERROR " ${MSG_HEAD} "
#	    "CBC_LIB_DIR_F ${CBC_LIB_DIR_F} not found.")
#  endif()
#elseif (CBC_LIB_DIR_F)
#  message(FATAL_ERROR " ${MSG_HEAD}" 
#	  "CBC_INC_DIR must be set along with CBC_LIB_DIR")
#endif()
#
#if (LINK_CBC)
#  message(STATUS ${MSG_HEAD} "Link Cbc? Yes.")
#else()
#  set (CBC_INC_DIR_F)
#  set (CBC_LIB_DIR_F)
#  message(STATUS ${MSG_HEAD} "Link Cbc? No.")
#endif()

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
    set(MNTR_INSTALL_RPATH ${MNTR_INSTALL_RPATH} ${QPOASES_LIB_DIR})
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
## ASL
###########################################################################
set(ASL_INC_DIR)        ## NULL
set(ASL_LIB_DIR)        ## NULL
set(LINK_ASL)       ## NULL
set(ASL_LIBS)       ## NULL

message(STATUS ${MSG_HEAD} "Searching for ASL ...")
set(ASL_INC_DIR "" CACHE PATH 
  "Path to ASL headers. It must have asl/asl.h") 
set(ASL_LIB_DIR "" CACHE PATH 
  "Path to ASL library. It must have amplsolver.a") 

set (ASL_INC_DIR_F ${ASL_INC_DIR})
set (ASL_LIB_DIR_F ${ASL_LIB_DIR})

if (("${ASL_INC_DIR_F}" STREQUAL "") AND
    (NOT ("${TP_ABS_PATH}" STREQUAL "")))
  set (ASL_INC_DIR_F "${TP_ABS_PATH}/include/asl")
  set (ASL_LIB_DIR_F "${TP_ABS_PATH}/lib")
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
  set(ASL_LIBS ${ASL_LIBS} "${ASL_LIB_DIR_F}/amplsolver.a" ${CMAKE_DL_LIBS})
  message(STATUS ${MSG_HEAD} "Using AMPL solver interface at"
    ${ASL_INC_DIR_F})
  message(STATUS ${MSG_HEAD} "ASL_LIBS =  ${ASL_LIBS}")
  message(STATUS ${MSG_HEAD} "Link AMPL-ASL? Yes.")
else()
  message(STATUS ${MSG_HEAD} "Link AMPL-ASL? No.")
endif()

if (MNTR_INSTALL_RPATH)
  list(REMOVE_DUPLICATES MNTR_INSTALL_RPATH)
endif()
###########################################################################
## Compile the source and interfaces to various libraries
###########################################################################
add_subdirectory(src)


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
    (NOT ("${TP_ABS_PATH}" STREQUAL "")))
  set (CPPUNIT_INC_DIR_F "${TP_ABS_PATH}/include")
endif()


set(CPPUNIT_LIB_DIR "" CACHE PATH 
  "Path to CppUnit library. It must have cppunit library file.") 
set (CPPUNIT_LIB_DIR_F ${CPPUNIT_LIB_DIR})
if (("${CPPUNIT_LIB_DIR_F}" STREQUAL "") AND
    (NOT ("${TP_ABS_PATH}" STREQUAL "")))
  set (CPPUNIT_LIB_DIR_F "${TP_ABS_PATH}/lib")
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


## for make utest, copy instances, cd to src testing, and run unittest
if (LINK_CPPUNIT)
  add_custom_target(utest 
    ${CMAKE_COMMAND} -E remove_directory ./src/testing/instances
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${PROJECT_SOURCE_DIR}/src/testing/instances ./instances 
    COMMAND unittest all
    WORKING_DIRECTORY src/testing)
endif()


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

###########################################################################
## hide unused options
###########################################################################
set(EXECUTABLE_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}" CACHE INTERNAL "")
set(LIBRARY_OUTPUT_PATH "${LIBRARY_OUTPUT_PATH}" CACHE INTERNAL "")


###########################################################################
## make examples
###########################################################################
# Check if TP_ABS_PATH is set, otherwise exit with an error

if(NOT DEFINED TP_ABS_PATH OR "${TP_ABS_PATH}" STREQUAL "")
    message(WARNING "THIRD_PARTY_PATH is not set. Examples will not be compiled. Continuing with rest of the build process...")
else()
    # Set variables
    set(CXX_FLAGS "-ggdb")
    set(LD_ADD "-lgfortran -llapack -ldl")
    set(MINCDIR "${CMAKE_CURRENT_BINARY_DIR}/include/minotaur")
    set(MLIBDIR "${CMAKE_CURRENT_BINARY_DIR}/lib")
    set(TARGET_DIR "${CMAKE_SOURCE_DIR}/examples")
    set(ASL_LIB "${TP_ABS_PATH}/lib/amplsolver.a")
    set(FILTERSQP_LIB "${TP_ABS_PATH}/lib/libfiltersqp.a")
    set(BPQD_LIB "${TP_ABS_PATH}/lib/libbqpd.a")
    set(RPATH_FLAGS "-Wl,-rpath=${MLIBDIR}:${TP_ABS_PATH}/lib")

    # Set include directories
    include_directories(${MINCDIR} ${TP_ABS_PATH}/include/asl)

    # Set link directories
    link_directories(${MLIBDIR} ${TP_ABS_PATH}/lib)

    # Get all .cpp files in the examples directory
    file(GLOB EXAMPLE_CPP_FILES "${TARGET_DIR}/*.cpp")

    # List of executables to be built
    set(EXAMPLE_EXECUTABLES)
    # List of executables to be removed
    set(RM_EXAMPLE_EXECUTABLES)

    # Create an executable for each example and set output directory to be the examples folder
    foreach(EXAMPLE_FILE ${EXAMPLE_CPP_FILES})
        get_filename_component(EXAMPLE_NAME ${EXAMPLE_FILE} NAME_WE)

        # Add the executable target for each example
        add_executable(${EXAMPLE_NAME} ${EXAMPLE_FILE})

        # Link required libraries
        target_link_libraries(${EXAMPLE_NAME}
            -lminotaur
            -lipopt
            -lOsiClp
            -lOsi
            -lClp
            -lCoinUtils
            -lcoinmumps
            ${FILTERSQP_LIB}
            ${BPQD_LIB}
            ${ASL_LIB}
            ${LD_ADD}
        )

        # Make sure examples do not build by default
        set_target_properties(${EXAMPLE_NAME} PROPERTIES
            EXCLUDE_FROM_ALL TRUE)

        # Set the RPATH flags
        set_target_properties(${EXAMPLE_NAME} PROPERTIES
            LINK_FLAGS "${RPATH_FLAGS}"
        )

        # Set the output directory to be the same as the examples directory
        set_target_properties(${EXAMPLE_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${TARGET_DIR}"
        )

        # Add the executable to the list
        list(APPEND EXAMPLE_EXECUTABLES ${EXAMPLE_NAME})
        list(APPEND RM_EXAMPLE_EXECUTABLES ${TARGET_DIR}/${EXAMPLE_NAME})
    endforeach()

    # Add a custom target to build all examples at once
    add_custom_target(examples
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${EXAMPLE_EXECUTABLES}
        DEPENDS ${EXAMPLE_EXECUTABLES}
        COMMENT "Building all examples in the examples directory..."
    )
endif()
###########################################################################
## make clean_examples
###########################################################################
# Add a custom target to clean all example binaries
add_custom_target(clean_examples
    COMMAND echo "Removing the following Example Binaries:"
    COMMAND echo ${RM_EXAMPLE_EXECUTABLES}
    COMMAND ${CMAKE_COMMAND} -E rm -f ${RM_EXAMPLE_EXECUTABLES}
    COMMENT "All example binaries cleaned..."
)


###########################################################################
## make uninstall
###########################################################################
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

