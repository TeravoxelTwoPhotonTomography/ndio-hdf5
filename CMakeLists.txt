cmake_minimum_required(VERSION 2.8)
project(ndio-hdf5-plugin)

macro(show v)
  message("${v} is ${${v}}")
endmacro()

################################################################################
# CONFIG
################################################################################
set(NDIO_HDF5_TEST_DATA_PATH ${PROJECT_SOURCE_DIR}/test/data)
configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_BINARY_DIR}/config.h)
include_directories(${PROJECT_BINARY_DIR})

find_package(GTEST CONFIG PATHS cmake)
find_package(ND    CONFIG PATHS cmake)
find_package(HDF5  CONFIG PATHS cmake CONFIGS HDF5Config.cmake)

include_directories(${ND_INCLUDE_DIRS})
include_directories(${HDF5_INCLUDE_DIRS})

##############################################################################
# SOURCE
##############################################################################

file(GLOB SRCS src/*.c)
file(GLOB HDRS src/*.h)

##############################################################################
# TARGETS
##############################################################################

add_definitions(-fPIC)
add_library(ndio-hdf5 MODULE ${SRCS} ${HDRS})
target_link_libraries(ndio-hdf5 ${HDF5_LIBRARIES} ${ND_LIBRARIES})
add_dependencies(ndio-hdf5 nd)

##############################################################################
#  Testing
##############################################################################

file(GLOB TEST_SOURCES test/*.cc)
if(GTEST_FOUND AND TEST_SOURCES)
  find_package(Threads)
  enable_testing()
  include_directories(${PROJECT_BINARY_DIR})
  include_directories(${GTEST_INCLUDE_DIR})
  add_executable(test-ndio-hdf5 ${TEST_SOURCES} config.h)
  target_link_libraries(test-ndio-hdf5
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${ND_LIBRARIES}
    )
  add_dependencies(test-ndio-hdf5 gtest nd)
  gtest_copy_shared_libraries(test-ndio-hdf5)
  install(TARGETS test-ndio-hdf5 EXPORT ndio-hdf5-targets DESTINATION bin)
  add_test(TestNDIOSeries test-ndio-hdf5)
  gtest_copy_shared_libraries(test-ndio-hdf5)
endif()                

################################################################################
#  Documentation
################################################################################

find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in
                 ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile
                 @ONLY
                 )
  add_custom_target(ndio-hdf5-docs
    ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/doc/Doxyfile
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    COMMENT           "Generating API Docs"
    VERBATIM
    )
endif()

##############################################################################
#  Install
##############################################################################
foreach(_path ${NDIO_PLUGIN_PATH})
  install(TARGETS ndio-hdf5 EXPORT ndio-hdf5-targets LIBRARY DESTINATION ${_path} )
endforeach()
#install(TARGETS ndio-hdf5 EXPORT ndio-hdf5-targets LIBRARY DESTINATION bin/plugins)
install(EXPORT ndio-hdf5-targets DESTINATION cmake)
export(TARGETS ndio-hdf5 FILE ndio-hdf5-config.cmake)
export(PACKAGE ndio-hdf5)

