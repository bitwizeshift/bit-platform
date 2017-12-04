cmake_minimum_required(VERSION 3.1)

enable_testing()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")

include(AddHeaderSelfContainmentTest)
include(MakeVersionHeader)
include(EnableCCache)

#-----------------------------------------------------------------------------
# Project Setup
#-----------------------------------------------------------------------------

# enable cacche to speed up compilations
enable_ccache()

option(BIT_PLATFORM_COMPILE_HEADER_SELF_CONTAINMENT_TESTS "Include each header independently in a .cpp file to determine header independence" on)
option(BIT_PLATFORM_COMPILE_UNIT_TESTS "Compile and run the unit tests for this library" on)
option(BIT_PLATFORM_GENERATE_DOCUMENTATION "Generates doxygen documentation" off)

project("BitPlatform")

add_subdirectory("external/bit/stl")

if( CMAKE_BUILD_TYPE STREQUAL "" )
  message(STATUS "CMAKE_BUILD_TYPE not set; defaulting to 'Debug'")
  set(CMAKE_BUILD_TYPE DEBUG)
endif()

#-----------------------------------------------------------------------------
# Version Info
#-----------------------------------------------------------------------------

set(BIT_PLATFORM_VERSION_MAJOR 0 CACHE STRING "major version of bit::platform" FORCE)
set(BIT_PLATFORM_VERSION_MINOR 2 CACHE STRING "minor version of bit::platform" FORCE)
set(BIT_PLATFORM_VERSION_PATCH 0 CACHE STRING "patch version of bit::platform" FORCE)

set(BIT_PLATFORM_VERSION "${BIT_PLATFORM_VERSION_MAJOR}.${BIT_PLATFORM_VERSION_MINOR}.${BIT_PLATFORM_VERSION_PATCH}" CACHE STRING "version of bit::platform" FORCE)

message(STATUS "bit::platform ${BIT_PLATFORM_VERSION}")

#-----------------------------------------------------------------------------
# bit::platform
#-----------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)

make_version_header("${CMAKE_CURRENT_BINARY_DIR}/include/bit/platform/version.hpp"
  MAJOR ${BIT_PLATFORM_VERSION_MAJOR}
  MINOR ${BIT_PLATFORM_VERSION_MINOR}
  PATCH ${BIT_PLATFORM_VERSION_PATCH}
  PREFIX BIT_PLATFORM
  SUFFIX "a"
)

set(headers
  # threading
  include/bit/platform/threading/concurrent_queue.hpp
  include/bit/platform/threading/job_dispatcher.hpp
  include/bit/platform/threading/null_mutex.hpp
  include/bit/platform/threading/semaphore.hpp
  include/bit/platform/threading/shared_mutex.hpp
  include/bit/platform/threading/spin_lock.hpp
  include/bit/platform/threading/thread.hpp
  include/bit/platform/threading/thread_pool.hpp
  include/bit/platform/threading/unlock_guard.hpp
  include/bit/platform/threading/waitable_event.hpp

  # filesystem
  include/bit/platform/filesystem/filesystem.hpp
  include/bit/platform/filesystem/null_file_device.hpp
  include/bit/platform/filesystem/disk_file_device.hpp
  include/bit/platform/filesystem/crypto_file_device.hpp
  include/bit/platform/filesystem/random_file_device.hpp

  # timing
  include/bit/platform/timing/clock.hpp
  include/bit/platform/timing/scope_clock.hpp

  # input
  include/bit/platform/system/keyboard.hpp
  include/bit/platform/system/event.hpp
  include/bit/platform/system/processor.hpp
)

if( WIN32 )
  set(platform_source_files
    # threading
    src/bit/platform/threading/win32/thread.cpp
    src/bit/platform/threading/win32/semaphore.cpp

    # fiesystem
    src/bit/platform/filesystem/win32/disk_file_device.cpp
    src/bit/platform/filesystem/win32/disk_file.cpp
    src/bit/platform/filesystem/win32/directory_watcher.cpp

    # input
    src/bit/platform/system/win32/keyboard.cpp
  )
elseif( UNIX )
  set(platform_source_files
    src/bit/platform/threading/posix/thread.cpp
    src/bit/platform/threading/posix/semaphore.cpp
  )
elseif( APPLE )
  set(platform_source_files
    src/bit/platform/threading/mac/thread.cpp
    src/bit/platform/threading/mac/semaphore.cpp
  )
else()
  message(FATAL_ERROR "unknown or unsupported target platform")
endif()

set(source_files
  # threading
  src/bit/platform/threading/job_dispatcher.cpp
  src/bit/platform/threading/spin_lock.cpp

  # filesystem
  src/bit/platform/filesystem/filesystem.cpp
  src/bit/platform/filesystem/null_file.cpp
  src/bit/platform/filesystem/null_file_device.cpp
  src/bit/platform/filesystem/crypto_file.cpp
  src/bit/platform/filesystem/crypto_file_device.cpp
  src/bit/platform/filesystem/random_file.cpp
  src/bit/platform/filesystem/random_file_device.cpp

  # timing
  src/bit/platform/timing/clock.cpp

  # system
  src/bit/platform/system/keyboard.cpp
  src/bit/platform/system/event.cpp
  src/bit/platform/system/processor.cpp
  src/bit/platform/system/cpuid.cpp

  # platform-specific
  ${platform_source_files}
)

add_library(bit_platform ${source_files})
add_library(bit::platform ALIAS bit_platform)

target_link_libraries(bit_platform PUBLIC bit::stl)

target_include_directories(bit_platform PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
)
target_compile_definitions(bit_platform PUBLIC
  $<$<CONFIG:DEBUG>:DEBUG>
  $<$<CONFIG:RELEASE>:NDEBUG RELEASE>
)

#-----------------------------------------------------------------------------
# bit::platform : Independence Tests
#-----------------------------------------------------------------------------

if( BIT_PLATFORM_COMPILE_HEADER_SELF_CONTAINMENT_TESTS )

  add_header_self_containment_test(bit_platform_header_self_containment_test ${headers})
  target_compile_definitions(bit_platform_header_self_containment_test PRIVATE
    $<$<CONFIG:DEBUG>:DEBUG>
    $<$<CONFIG:RELEASE>:NDEBUG RELEASE>
  )
  target_include_directories(bit_platform_header_self_containment_test PRIVATE
    $<TARGET_PROPERTY:bit_platform,INCLUDE_DIRECTORIES>
  )

  add_library(bit::platform::header_self_containment_test ALIAS bit_platform_header_self_containment_test)

  if( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(bit_platform_header_self_containment_test PRIVATE -Wall -Werror -pedantic)
  elseif( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
    target_compile_options(bit_platform_header_self_containment_test PRIVATE -Wall -Werror -pedantic)
  elseif( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" )
    # TODO: Determine MSVC necessary compiler flags
  endif()

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

export_library( TARGETS bit_platform
                PACKAGE Platform
                VERSION ${BIT_PLATFORM_VERSION}
                MAJOR_VERSION ${BIT_PLATFORM_VERSION_MAJOR}
                MINOR_VERSION ${BIT_PLATFORM_VERSION_MINOR}
                INCLUDE_PATHS "${CMAKE_CURRENT_LIST_DIR}/include"
                              "${CMAKE_CURRENT_BINARY_DIR}/include"
                NAMESPACE "bit::"
                PREFIX_PACKAGE )
