add_definitions(-DUSE_BQPD)
include_directories("${PROJECT_BINARY_DIR}/src/base")
include_directories("${PROJECT_SOURCE_DIR}/src/base")

set (BQPD_SOURCES
  BqpdEngine.cpp 
  BqpdAux.f 
  BqpdAux2.f 
)
set (BQPD_HEADERS
  BqpdEngine.h 
)


add_library(mntrbqpd ${BQPD_SOURCES})

# Build only static library as BqpdAux*.f files have common blocks that access
# data in bqpd's static library. We see alignment warnings if shared library
# is used.
install(TARGETS mntrbqpd ARCHIVE DESTINATION lib)

install(FILES ${BQPD_HEADERS} DESTINATION include/minotaur)

