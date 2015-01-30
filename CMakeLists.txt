cmake_minimum_required(VERSION 2.8.12)
project(net)

# Configures the compiler, regardless of the build type.
if("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
  add_definitions(-std=c++11
                  -stdlib=libc++
                  -Weverything
                  -Wno-c++98-compat
                  -Wno-missing-prototypes
                  )
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  add_definitions(-std=c++11
                  -Wall
                  -Wextra
                  -Wpedantic
                  )
endif()

#
# Use `make doc` to generate API documentation.
#
find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM)
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/html
          DESTINATION doc)
endif(DOXYGEN_FOUND)


# Adds the include directory to the compiler's search path.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Builds the test drivers.
enable_testing()

add_executable(net_client_test_pos01 ${CMAKE_CURRENT_SOURCE_DIR}/test/net_client_test_pos01.cpp)
add_test(net_client_test_pos01 net_client_test_pos01)

add_executable(net_server_test_pos01 ${CMAKE_CURRENT_SOURCE_DIR}/test/net_server_test_pos01.cpp)
add_test(net_server_test_pos01 net_server_test_pos01)

#add_executable(TEST ${CMAKE_CURRENT_SOURCE_DIR}/test/TEST.cpp)
#add_test(TEST TEST)

