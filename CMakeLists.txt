cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(relacy)

include(CheckCXXSourceCompiles)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

check_cxx_source_compiles("
  #include <atomic>
  int main() { 
    std::atomic<unsigned int> x;
    x.fetch_add(1);
    x.fetch_sub(1);
    return 0; 
  }
" HAVE_STD_ATOMIC)

#add_library(relacy INTERFACE)
#target_include_directories(relacy INTERFACE
#    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/relacy>
#    )
file(GLOB_RECURSE RELACY_SOURCES relacy/*.h relacy/*.hpp)
add_custom_target(relacy SOURCES ${RELACY_SOURCES})

add_executable(test_relacy
    test/compare_swap.hpp
    test/condvar.hpp
    test/data_race.hpp
    test/dyn_thread.hpp
    test/event.hpp
    test/fence.hpp
    test/main.cpp
    test/memory.hpp
    test/memory_order.hpp
    test/mutex.hpp
    test/pthread.hpp
    test/scheduler.hpp
    test/semaphore.hpp
    test/stdafx.cpp
    test/stdafx.h
    test/thread_local.hpp
    test/wfmo.hpp
    test/windows.hpp
    )
configure_file(relacy_config.hpp.in relacy_config.hpp)