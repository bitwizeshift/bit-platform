cmake_minimum_required(VERSION 3.1)

find_package(Catch REQUIRED)

set(sources
      main.test.cpp
      bit/platform/concurrency/utilities/concurrent_queue.test.cpp
)

add_executable(platform_test ${sources})

target_link_libraries(platform_test PRIVATE "bit::platform" "philsquared::Catch")

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------

add_test( NAME "platform_test_default"
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          COMMAND "$<TARGET_FILE:platform_test>" )

#-----------------------------------------------------------------------------

add_test( NAME "platform_test_all"
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          COMMAND "$<TARGET_FILE:platform_test>" "*" )
