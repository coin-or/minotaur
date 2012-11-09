include_directories("${PROJECT_BINARY_DIR}/src/base")
include_directories("${PROJECT_SOURCE_DIR}/src/base")

set (ENGFAC_SOURCES
  EngineFactory.cpp
)
set (ENGFAC_HEADERS
  EngineFactory.h
)


link_directories(${IPOPT_LIB_DIR})
link_directories(${OSI_LIB_DIR})
add_library(mntrengfac ${ENGFAC_SOURCES})

if (LINK_IPOPT)
  add_definitions(-DUSE_IPOPT)
  include_directories("${PROJECT_SOURCE_DIR}/src/engines/Ipopt")
endif (LINK_IPOPT)

if (LINK_FILTER_SQP)
  add_definitions(-DUSE_BQPD)
  add_definitions(-DUSE_FILTERSQP)
  include_directories("${PROJECT_SOURCE_DIR}/src/engines/FilterSQP")
  include_directories("${PROJECT_SOURCE_DIR}/src/engines/Bqpd")
endif (LINK_FILTER_SQP)

if (LINK_QPOASES)
  add_definitions(-DUSE_QPOASES)
  include_directories("${PROJECT_SOURCE_DIR}/src/engines/qpOASES")
  include_directories("${QPOASES_INC_DIR}")
  link_directories(${QPOASES_LIB_DIR})
endif (LINK_QPOASES)

if (LINK_OSI)
  add_definitions(-DUSE_OSILP)
  include_directories("${PROJECT_SOURCE_DIR}/src/engines/OsiLP")
endif (LINK_OSI)

if (BUILD_SHARED_LIBS)
  install(TARGETS mntrengfac LIBRARY DESTINATION lib)
else (BUILD_SHARED_LIBS)
  install(TARGETS mntrengfac ARCHIVE DESTINATION lib)
endif (BUILD_SHARED_LIBS)

install(FILES ${ENGFAC_HEADERS} DESTINATION include/minotaur)
