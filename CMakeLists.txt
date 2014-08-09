cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
project(relacy)

# enable testing
enable_testing()
include(Dart)

include(CheckCXXSourceCompiles)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

check_cxx_source_compiles("
  #include <atomic>
  int main() { 
    std::atomic<unsigned int> x;
    x.fetch_add(1);
    x.fetch_sub(1);
    return 0; 
  }
" HAVE_STD_ATOMIC)

check_cxx_source_compiles("
int main()
{
	char const *func = __func__;
	return 0;
}
"
HAVE__FUNC__)
check_cxx_source_compiles("
int main()
{
	char const *func = __FUNCTION__;
	return 0;
}
"
HAVE__FUNCTION__)

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

foreach(example
	cli_ws_deque
	condvar
	#eao_blocking               # all of the implementing code is commented out in this example /rlc (2014-08-07)
	#eventcount                 # this example appears to be broken: it deletes a thread-local static. As Relacy tries to do this twice expecting the same results, the second time it gets a stale pointer, and crashes /rlc (2014-08-07)
	examples
	java_ws_deque
	mpmc
	mutex_business_logic
	peterson
	proxy_collector
	ref_counting
	smr
	spsc_queue
	stack
	tbb_eventcount
	ws_deque
	ws_deque2
	)
	add_executable(example_${example}
		example/${example}/${example}.cpp
		example/${example}/stdafx.cpp
		example/${example}/stdafx.h
		)
	add_test(example_${example} ${EXECUTABLE_OUTPUT_PATH}/example_${example})
endforeach()

configure_file(relacy_config.hpp.in relacy_config.hpp)
