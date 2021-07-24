##############################################################################
## Do not change this section. It should stay same for all binaries
##############################################################################
add_definitions(-DUSE_MINOTAUR_AMPL_INTERFACE)
include_directories("${PROJECT_BINARY_DIR}/src/base")
include_directories("${PROJECT_SOURCE_DIR}/src/base")
include_directories("${PROJECT_SOURCE_DIR}/src/engines")
include_directories("${PROJECT_SOURCE_DIR}/src/interfaces/ampl")
include_directories("${ASL_INC_DIR_F}")

set (QPOASES_LIBS)
set (GFORTRAN_COMPILER)

## define where to search for external libraries. This path must be defined
## before add_executable is called.
if (LINK_BQPD)
  add_definitions(-DUSE_BQPD)
  include_directories("${PROJECT_SOURCE_DIR}/src/engines/Bqpd")
  link_directories(${BQPD_LIB_DIR_F})
endif()

if (LINK_FILTER_SQP)
  add_definitions(-DUSE_FilterSQP)
  include_directories("${PROJECT_SOURCE_DIR}/src/engines/FilterSQP")
  link_directories(${FILTER_LIB_DIR_F})
endif()

if (LINK_IPOPT)
  add_definitions(-DUSE_IPOPT)
  include_directories("${PROJECT_SOURCE_DIR}/src/engines/Ipopt")
  include_directories("${IPOPT_INC_DIR_F}")
  link_directories(${IPOPT_LIB_DIR_F})
endif()

if (LINK_OSI)
  add_definitions(-DUSE_OSILP)
  include_directories("${PROJECT_SOURCE_DIR}/src/engines/OsiLP")
  if (OSI_INC_DIR_F)
    include_directories("${OSI_INC_DIR_F}")
  endif()
  if (OSI_LIB_DIR_F)
    link_directories(${OSI_LIB_DIR_F})
  endif()
  if (LINK_CLP)
    link_directories(${CLP_LIB_DIR_F})
  endif()
  if (LINK_CBC)
    link_directories(${CBC_LIB_DIR})
  endif()
endif()

if (LINK_CPX)
  add_definitions(-DUSE_CPX -DIL_STD)
  include_directories("${PROJECT_SOURCE_DIR}/src/engines/Cplex")
  include_directories("${CPX_INC_DIR_F}")
  #include_directories("${CPX_INC_DIR_F}/../../concert/include") # does not read from here
  link_directories(${CPX_LIB_DIR_F})
endif()

if (LINK_QPOASES)
  add_definitions(-DUSE_QPOASES)
  include_directories("${PROJECT_SOURCE_DIR}/src/engines/qpOASES")
  include_directories("${QPOASES_INC_DIR}")
  set (QPOASES_LIBS mntrqpoases)
endif()
  
if (Fortran_COMPILER_NAME STREQUAL "gfortran")
  if (LINK_IPOPT OR LINK_FILTER_SQP OR LINK_BQPD)
    set (GFORTRAN_COMPILER gfortran)
  endif()
endif()

set (ALL_EXEC_LIBS 
	"mntrampl" 
	"mntrengfac" 
	"${IPOPT_LIBS}"
	"${CPX_LIBS}"
	"${CBC_LIBS}"
        "${OSI_LIBS}"
        "${FILTER_SQP_LIBS}"
	"${BQPD_LIBS}"
        "${QPOASES_LIBS}"
        "minotaur"
	"${ASL_LIBS}"
        "lapack"
	"blas"
        "${GFORTRAN_COMPILER}"
        "${MNTR_EXTRA_LIBS}"
	)
message(STATUS ${MSG_HEAD} "ALL_EXEC_LIBS =  ${ALL_EXEC_LIBS}")
list(REVERSE ALL_EXEC_LIBS)
list(REMOVE_DUPLICATES ALL_EXEC_LIBS)
list(REVERSE ALL_EXEC_LIBS)
message(STATUS ${MSG_HEAD} "ALL_EXEC_LIBS after removing duplicates =  ${ALL_EXEC_LIBS}")

##############################################################################
## Add lines specific to your binaries in this section.
## Use the lines meant for bnb as a template.
##############################################################################
set (LSTOA_SOURCES
   LSTOA.cpp
)

add_executable(lstoa ${LSTOA_SOURCES})
target_link_libraries(lstoa ${ALL_EXEC_LIBS})

# This will install the binary in bin directory.
install(TARGETS lstoa RUNTIME DESTINATION bin)

##############################################################################
## Add lines specific to your binaries in this section.
## Use the lines meant for bnb as a template.
##############################################################################
set (OA_SOURCES
  OA.cpp
)

add_executable(oa ${OA_SOURCES})
target_link_libraries(oa ${ALL_EXEC_LIBS})

# This will install the binary in bin directory.
install(TARGETS oa RUNTIME DESTINATION bin)

##############################################################################
## Add lines specific to your binaries in this section. 
## Use the lines meant for bnb as a template.
##############################################################################
set (QG2_SOURCES
  Solver.cpp
  QG2.cpp
  QGMain.cpp 
)

add_executable(qg2 ${QG2_SOURCES})
target_link_libraries(qg2 ${ALL_EXEC_LIBS})

# This will install the binary in bin directory.
install(TARGETS qg2 RUNTIME DESTINATION bin)

##############################################################################
## Add lines specific to your binaries in this section. 
## Use the lines meant for bnb as a template.
##############################################################################
set (BNB_SOURCES
  Solver.cpp
  Bnb.cpp 
  BnbMain.cpp
)

add_executable(bnb ${BNB_SOURCES})
target_link_libraries(bnb ${ALL_EXEC_LIBS})

# This will install the binary in bin directory.
install(TARGETS bnb RUNTIME DESTINATION bin)

##############################################################################
## Add lines specific to your binaries in this section.
## Use the lines meant for bnb as a template.
##############################################################################
#set (MSBNB_SOURCES
 #MsBnb.cpp
#)

#add_executable(msbnb ${MSBNB_SOURCES})
#target_link_libraries(msbnb ${ALL_EXEC_LIBS})

## This will install the binary in bin directory.
#install(TARGETS msbnb RUNTIME DESTINATION bin)

##############################################################################
## Add lines specific to your binaries in this section.
## Use the lines meant for bnb as a template.
##############################################################################
set (MCQG_SOURCES
McQG.cpp
)

add_executable(mcqg ${MCQG_SOURCES})
target_link_libraries(mcqg ${ALL_EXEC_LIBS})

# This will install the binary in bin directory.
install(TARGETS mcqg RUNTIME DESTINATION bin)

##############################################################################
## Add lines specific to your binaries in this section.
## Use the lines meant for bnb as a template.
##############################################################################
set (MCQGADV_SOURCES
McQGAdv.cpp
)

add_executable(mcqgadv ${MCQGADV_SOURCES})
target_link_libraries(mcqgadv ${ALL_EXEC_LIBS})

# This will install the binary in bin directory.
install(TARGETS mcqgadv RUNTIME DESTINATION bin)


##############################################################################
## Add lines specific to your binaries in this section.
## Use the lines meant for bnb as a template.
##############################################################################
set (MCBNB_SOURCES
 McBnb.cpp
)

add_executable(mcbnb ${MCBNB_SOURCES})
target_link_libraries(mcbnb ${ALL_EXEC_LIBS})

# This will install the binary in bin directory.
install(TARGETS mcbnb RUNTIME DESTINATION bin)

##############################################################################
## Add lines specific to your binaries in this section.
## Use the lines meant for bnb as a template.
##############################################################################
#set (MIDFO_SOURCES
# MiDfo.cpp
#)
#
#add_executable(midfo ${MIDFO_SOURCES})
#target_link_libraries(midfo ${ALL_EXEC_LIBS})
#
## This will install the binary in bin directory.
#install(TARGETS midfo RUNTIME DESTINATION bin)

##############################################################################
## Add lines specific to your binaries in this section. 
## Use the lines meant for bnb as a template.
##############################################################################
set (QPD_SOURCES
   QPDive.cpp 
)

add_executable(qpd ${QPD_SOURCES})
target_link_libraries(qpd ${ALL_EXEC_LIBS})

# This will install the binary in bin directory.
install(TARGETS qpd RUNTIME DESTINATION bin)

##############################################################################
## Add lines specific to your binaries in this section. 
## Use the lines meant for bnb as a template.
##############################################################################
set (QG_SOURCES
  QG.cpp 
)

add_executable(qg ${QG_SOURCES})
target_link_libraries(qg ${ALL_EXEC_LIBS})

# This will install the binary in bin directory.
install(TARGETS qg RUNTIME DESTINATION bin)

##############################################################################
## Add lines specific to your binaries in this section. 
## Use the lines meant for bnb as a template.
##############################################################################
#set (QGADV_SOURCES
#QGAdv.cpp 
#)

#add_executable(qgadv ${QGADV_SOURCES})
#target_link_libraries(qgadv ${ALL_EXEC_LIBS})

## This will install the binary in bin directory.
#install(TARGETS qgadv RUNTIME DESTINATION bin)

##############################################################################
## Add lines specific to your binaries in this section. 
## Use the lines meant for bnb as a template.
##############################################################################
#set (POLYNOMIAL_SOURCES
#  mipdecoqg.cpp 
#)
#
#add_executable(mipdecoqg ${POLYNOMIAL_SOURCES})
#target_link_libraries(mipdecoqg ${ALL_EXEC_LIBS})
#
## This will install the binary in bin directory.
#install(TARGETS mipdecoqg RUNTIME DESTINATION bin)


##############################################################################
## Add lines specific to your binaries in this section. 
## Use the lines meant for bnb as a template.
##############################################################################
set (GLOB_SOURCES
 Glob.cpp 
 QG2.cpp
 Solver.cpp
 GlobMain.cpp
)

add_executable(glob ${GLOB_SOURCES})
target_link_libraries(glob ${ALL_EXEC_LIBS})

# This will install the binary in bin directory.
install(TARGETS glob RUNTIME DESTINATION bin)

# Serdar added for branch and cut method. 
##############################################################################
## Add lines specific to your binaries in this section. 
## Use the lines meant for bnb as a template.
##############################################################################
#set (BNC_SOURCES
#  Bnc.cpp 
#)
#
#add_executable(bnc ${BNC_SOURCES})
#target_link_libraries(bnc ${ALL_EXEC_LIBS})
#
## This will install the binary in bin directory.
#install(TARGETS bnc RUNTIME DESTINATION bin)
