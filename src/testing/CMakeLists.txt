CMAKE_MINIMUM_REQUIRED(VERSION 3.0 FATAL_ERROR) 
include_directories("${PROJECT_BINARY_DIR}/src/base")
include_directories("${PROJECT_SOURCE_DIR}/src/base")
include_directories("${PROJECT_SOURCE_DIR}/src/interfaces")
include_directories("${CPPUNIT_INC_DIR_F}")
set (ALL_EXEC_LIBS) # NULL

set (MINOTAUR_SOURCES
     unittest.cpp 
     CGraphUT.cpp
     EnvironmentUT.cpp
     FunctionUT.cpp
     ProblemUT.cpp
     JacobianUT.cpp
     HessianOfLagUT.cpp
     LapackUT.cpp
     LinearFunctionUT.cpp
     LoggerUT.cpp
     ObjectiveUT.cpp
     OperationsUT.cpp
     PerspRefUT.cpp
     PolyUT.cpp
     QuadraticFunctionUT.cpp
     TimerUT.cpp 
)

## define where to search for external libraries. This path must be defined
## before add_executable is called.
link_directories(${CPPUNIT_LIB_DIR_F})

if (LINK_FILTER_SQP OR LINK_IPOPT OR LINK_OSI OR LINK_ASL)
  include_directories("${PROJECT_SOURCE_DIR}/src/interfaces")
endif()

if (LINK_FILTER_SQP)
  add_definitions(-DUSE_FilterSQP)
  add_definitions(-DUSE_BQPD)
  link_directories(${FILTER_LIB_DIR_F})
endif()

if (LINK_IPOPT)
  add_definitions(-DUSE_IPOPT)
  set (MINOTAUR_SOURCES  ${MINOTAUR_SOURCES} NLPBnbUT.cpp IpoptEngineUT.cpp)
  link_directories(${IPOPT_LIB_DIR_F})
endif()

if (LINK_OSI)
  add_definitions(-DUSE_OSILP)
  if (OSI_INC_DIR_F)
    include_directories("${OSI_INC_DIR_F}")
  endif()
  if (OSI_LIB_DIR_F) 
    link_directories(${OSI_LIB_DIR_F})
  endif()
  if (LINK_CLP)
    link_directories(${CLP_LIB_DIR_F})
  endif()
endif()

if (LINK_ASL)
  add_definitions(-DUSE_MINOTAUR_AMPL_INTERFACE)
  set (MINOTAUR_SOURCES  ${MINOTAUR_SOURCES} AMPLInstanceUT.cpp
	                                     AMPLCGraphUT.cpp
					     PolySolverUT.cpp
				             TransformerUT.cpp)
  include_directories(${ASL_INC_DIR_F})
  if (LINK_FILTER_SQP)
    set (MINOTAUR_SOURCES  ${MINOTAUR_SOURCES} AMPLBqpdUT.cpp AMPLFilterSQPUT.cpp)
  endif()
  if (LINK_IPOPT)
    set (MINOTAUR_SOURCES  ${MINOTAUR_SOURCES} AMPLIpoptUT.cpp)
  endif()
#  if (LINK_CBC)
#    add_definitions(-DUSE_CBC)
#    set (MINOTAUR_SOURCES  ${MINOTAUR_SOURCES} AMPLOsiUT.cpp AMPLCbcUT.cpp )
#    link_directories(${CBC_LIB_DIR_F})
#  elseif (LINK_OSI)
#    set (MINOTAUR_SOURCES  ${MINOTAUR_SOURCES} AMPLOsiUT.cpp)
#  endif()
endif()
  
add_executable(unittest EXCLUDE_FROM_ALL ${MINOTAUR_SOURCES})
list(APPEND ALL_EXEC_LIBS minotaur cppunit)

if (LINK_FILTER_SQP)
  list(APPEND ALL_EXEC_LIBS ${FILTER_SQP_LIBS})
  if (Fortran_COMPILER_NAME STREQUAL "gfortran")
    list(APPEND ALL_EXEC_LIBS gfortran)
  endif()
endif()
if (LINK_IPOPT)
  list(APPEND ALL_EXEC_LIBS ${IPOPT_LIBS})
  if (Fortran_COMPILER_NAME STREQUAL "gfortran")
    list(APPEND ALL_EXEC_LIBS gfortran)
  endif()
  message(STATUS ${MSG_HEAD} "fortran compiler = ${Fortran_COMPILER_NAME}")
endif()
#if (LINK_CBC)
#  list(APPEND ALL_EXEC_LIBS ${CBC_LIBS})
#endif()
if (LINK_OSI)
  list(APPEND ALL_EXEC_LIBS ${OSI_LIBS})
endif()
if (LINK_ASL)
  list(APPEND ALL_EXEC_LIBS ${ASL_LIB_DIR_F}/amplsolver.a)
endif()

#add_custom_target(Name test unittest all)
list(APPEND ALL_EXEC_LIBS  lapack blas)
if (Fortran_COMPILER_NAME STREQUAL "gfortran")
  list(APPEND ALL_EXEC_LIBS gfortran)
endif()
list(APPEND ALL_EXEC_LIBS ${MNTR_EXTRA_LIBS} dl)

list(REVERSE ALL_EXEC_LIBS)
list(REMOVE_DUPLICATES ALL_EXEC_LIBS)
list(REVERSE ALL_EXEC_LIBS)
message(STATUS ${MSG_HEAD} "unittest ALL_EXEC_LIBS after removing duplicates =  ${ALL_EXEC_LIBS}")
target_link_libraries(unittest ${ALL_EXEC_LIBS})

