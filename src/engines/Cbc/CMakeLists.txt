add_definitions(-DUSE_CBC)
include_directories("${PROJECT_BINARY_DIR}/src/base")
include_directories("${PROJECT_SOURCE_DIR}/src/base")
include_directories("${CBC_INC_DIR_F}")

set (CBC_SOURCES
  CbcEngine.cpp 
)
set (CBC_HEADERS
  CbcEngine.h 
)

add_library(mntrcbc ${CBC_SOURCES})

if (BUILD_SHARED_LIBS)
install(TARGETS mntrcbc LIBRARY DESTINATION lib)
else (BUILD_SHARED_LIBS)
install(TARGETS mntrcbc ARCHIVE DESTINATION lib)
endif (BUILD_SHARED_LIBS)

install(FILES ${CBC_HEADERS} DESTINATION include/minotaur)

