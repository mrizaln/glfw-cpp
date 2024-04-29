# use mold if it exist in path
find_program(MOLD mold)
if(MOLD)
  set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=mold")
  set(CMAKE_SHARED_LINKER_FLAGS "-fuse-ld=mold")
  message(STATUS "mold executable found: ${MOLD}")
endif()
