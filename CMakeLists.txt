cmake_minimum_required(VERSION 3.1)

enable_testing()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")

include(AddIndependenceCheck)
include(GitBuildInformation)
include(MakeVersionHeader)

#-----------------------------------------------------------------------------
# Project Setup
#-----------------------------------------------------------------------------

option(BIT_PLATFORM_COMPILE_INDEPENDENCE_TESTS "Include each header independently in a .cpp file to determine header independence" on)
option(BIT_PLATFORM_COMPILE_UNIT_TESTS "Compile and run the unit tests for this library" on)

project("BitPlatform")

find_package(Bit REQUIRED COMPONENTS Stl)

if( CMAKE_BUILD_TYPE STREQUAL "" )
  message(STATUS "CMAKE_BUILD_TYPE not set; defaulting to 'Debug'")
  set(CMAKE_BUILD_TYPE DEBUG)
endif()

#-----------------------------------------------------------------------------
# Version Info
#-----------------------------------------------------------------------------

git_build_information( PATCH_VARIABLE BIT_PLATFORM_VERSION_PATCH
                       BUILD_VARIABLE BIT_PLATFORM_BUILD_NUMBER
                       BRANCH_VARIABLE BIT_PLATFORM_BRANCH
                       REF v0.2
                       PATHSPECS ${CMAKE_CURRENT_LIST_DIR}/include
                                 ${CMAKE_CURRENT_LIST_DIR}/cmake
                                 ${CMAKE_CURRENT_LIST_FILE}
)

set(BIT_PLATFORM_VERSION_MAJOR 0 CACHE STRING "major version of bit::platform" FORCE)
set(BIT_PLATFORM_VERSION_MINOR 2 CACHE STRING "minor version of bit::platform" FORCE)
set(BIT_PLATFORM_VERSION_PATCH ${BIT_PLATFORM_VERSION_PATCH} CACHE STRING "patch version of bit::platform" FORCE)
set(BIT_PLATFORM_VERSION "${BIT_PLATFORM_VERSION_MAJOR}.${BIT_PLATFORM_VERSION_MINOR}.${BIT_PLATFORM_VERSION_PATCH}" CACHE STRING "version of bit::platform" FORCE)
set(BIT_PLATFORM_BUILD_NUMBER "${BIT_PLATFORM_BUILD_NUMBER}" CACHE STRING "build number of bit::platform" FORCE)

message(STATUS "bit::platform ${BIT_PLATFORM_VERSION} (${BIT_PLATFORM_BRANCH} ${BIT_PLATFORM_BUILD_NUMBER})")

#-----------------------------------------------------------------------------
# bit::platform
#-----------------------------------------------------------------------------

set(CMAKE_CXX_VERSION 14)

make_version_header("${CMAKE_CURRENT_BINARY_DIR}/include/bit/platform/version.hpp"
  MAJOR ${BIT_PLATFORM_VERSION_MAJOR}
  MINOR ${BIT_PLATFORM_VERSION_MINOR}
  PATCH ${BIT_PLATFORM_VERSION_PATCH}
  BUILD ${BIT_PLATFORM_BUILD_NUMBER}
  PREFIX BIT_PLATFORM
  SUFFIX "a"
  TAG ${BIT_PLATFORM_BRANCH}
)

set(headers
  # threading
  include/bit/platform/threading/null_mutex.hpp
  include/bit/platform/threading/shared_mutex.hpp
  include/bit/platform/threading/spin_lock.hpp
  include/bit/platform/threading/thread.hpp
  include/bit/platform/threading/unlock_guard.hpp

  # filesystem
  include/bit/platform/filesystem/filesystem.hpp
  include/bit/platform/filesystem/null_file_device.hpp
  include/bit/platform/filesystem/disk_file_device.hpp
  include/bit/platform/filesystem/crypto_file_device.hpp
  include/bit/platform/filesystem/random_file_device.hpp
)

if( WIN32 )
  set(platform_source_files
    src/bit/platform/threading/win32/thread.cpp
    src/bit/platform/filesystem/win32/disk_file_device.cpp
    src/bit/platform/filesystem/win32/disk_file.cpp
  )
elseif( UNIX )
  set(platform_source_files
    src/bit/platform/threading/posix/thread.cpp
  )
elseif( APPLE )
  set(platform_source_files
    src/bit/platform/threading/mac/thread.cpp
  )
else()
  message(FATAL_ERROR "unknown or unsupported target platform")
endif()

set(source_files
  src/bit/platform/threading/spin_lock.cpp
  src/bit/platform/filesystem/filesystem.cpp
  src/bit/platform/filesystem/null_file.cpp
  src/bit/platform/filesystem/null_file_device.cpp
  src/bit/platform/filesystem/crypto_file.cpp
  src/bit/platform/filesystem/crypto_file_device.cpp
  src/bit/platform/filesystem/random_file.cpp
  src/bit/platform/filesystem/random_file_device.cpp
  ${platform_source_files}
)


add_library(platform ${source_files})
add_library(bit::platform ALIAS platform)

target_link_libraries(platform PUBLIC bit::stl)

set_target_properties(platform PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED true
)

target_include_directories(platform PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
)
target_compile_options(platform PUBLIC -std=c++14
  $<$<CONFIG:DEBUG>:-DDEBUG>
  $<$<CONFIG:RELEASE>:-DNDEBUG -DRELEASE>
)

#-----------------------------------------------------------------------------
# bit::platform : Independence Tests
#-----------------------------------------------------------------------------

if( BIT_PLATFORM_COMPILE_INDEPENDENCE_TESTS )

  add_independence_check(platform_independence ${headers})
  target_compile_options(platform_independence PRIVATE -std=c++14
    $<$<CONFIG:DEBUG>:-DDEBUG>
    $<$<CONFIG:RELEASE>:-DNDEBUG -DRELEASE>
  )
  target_include_directories(platform_independence PRIVATE $<TARGET_PROPERTY:platform,INCLUDE_DIRECTORIES>)

  add_library(bit::platform::independence ALIAS platform_independence)

endif()

#-----------------------------------------------------------------------------
# bit::platform : Unit Tests
#-----------------------------------------------------------------------------

if( BIT_PLATFORM_COMPILE_UNIT_TESTS )

  add_subdirectory(test)

endif()

#-----------------------------------------------------------------------------
# bit::platform : Export
#-----------------------------------------------------------------------------

include(ExportLibrary)

if( EXISTS "$ENV{BIT_HOME}" )
  set(CMAKE_INSTALL_PREFIX "$ENV{BIT_HOME}")
endif()

export_library( TARGETS platform
                PACKAGE Platform
                VERSION ${BIT_PLATFORM_VERSION}
                MAJOR_VERSION ${BIT_PLATFORM_VERSION_MAJOR}
                MINOR_VERSION ${BIT_PLATFORM_VERSION_MINOR}
                INCLUDE_PATHS "${CMAKE_CURRENT_LIST_DIR}/include"
                              "${CMAKE_CURRENT_BINARY_DIR}/include"
                NAMESPACE "bit::"
                PREFIX_PACKAGE )
