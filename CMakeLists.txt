cmake_minimum_required(VERSION 3.16)
project(gauge15 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# --- library: header-only, so this is an INTERFACE target -----------------
add_library(sfv INTERFACE)
target_include_directories(sfv INTERFACE include)
target_compile_features(sfv INTERFACE cxx_std_20)

# --- warning flags ----------------------------------------------------------
# -Wall/-Wextra/-Wpedantic are GCC/Clang spellings; MSVC's cl.exe does not
# understand dash-prefixed flags at all and fails to configure every target
# that receives them (this is what broke the MSVC leg of the CI matrix).
# /W4 is the closest MSVC equivalent.
if(MSVC)
  set(SFV_WARNING_FLAGS /W4)
else()
  set(SFV_WARNING_FLAGS -Wall -Wextra -Wpedantic)
endif()

# --- tests ------------------------------------------------------------------
enable_testing()

# Every test target is: compile tests/<name>.cpp, link against the
# header-only sfv target, apply the compiler-appropriate warning flags, and
# register it with ctest. test-api.cpp is the one exception that doesn't
# need the tests/ include dir (it doesn't pull in the generated conformance
# .inc files), so that's the one optional knob.
function(sfv_add_test name)
  cmake_parse_arguments(ARG "NO_TESTS_INCLUDE" "" "" ${ARGN})

  add_executable(sfv-${name} tests/${name}.cpp)
  target_link_libraries(sfv-${name} PRIVATE sfv)
  if(NOT ARG_NO_TESTS_INCLUDE)
    target_include_directories(sfv-${name} PRIVATE tests)
  endif()
  target_compile_options(sfv-${name} PRIVATE ${SFV_WARNING_FLAGS})
  add_test(NAME sfv-${name} COMMAND sfv-${name})
endfunction()

sfv_add_test(test-number-decimal)
sfv_add_test(test-boolean)
sfv_add_test(test-string)
sfv_add_test(test-token)
sfv_add_test(test-binary)
sfv_add_test(test-date)
sfv_add_test(test-display-string)
sfv_add_test(test-list)
sfv_add_test(test-param-list)
sfv_add_test(test-param-listlist)
sfv_add_test(test-dictionary)
sfv_add_test(test-param-dict)
sfv_add_test(test-key-generated)
sfv_add_test(test-api NO_TESTS_INCLUDE)
