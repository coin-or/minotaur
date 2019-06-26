add_definitions(-DUSE_CPX -DIL_STD)
include_directories("${PROJECT_BINARY_DIR}/src/base")
include_directories("${PROJECT_SOURCE_DIR}/src/base")
include_directories("${CPX_INC_DIR_F}")
include_directories("${CPX_INC_DIR_F}/../../concert/include")

set (CPX_SOURCES
  CplexLPEngine.cpp
  CplexMILPEngine.cpp
)
set (CPX_HEADERS
  CplexLPEngine.h
  CplexMILPEngine.h
)

add_library(mntrcpx ${CPX_SOURCES})

if (BUILD_SHARED_LIBS)
  install(TARGETS mntrcpx LIBRARY DESTINATION lib)
else (BUILD_SHARED_LIBS)
  install(TARGETS mntrcpx ARCHIVE DESTINATION lib)
endif (BUILD_SHARED_LIBS)

install(FILES ${CPX_HEADERS} DESTINATION include/minotaur)

