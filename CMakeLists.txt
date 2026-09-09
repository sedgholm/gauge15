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

# --- tests ------------------------------------------------------------------
enable_testing()

add_executable(sfv-test-number-decimal tests/test-number-decimal.cpp)
target_link_libraries(sfv-test-number-decimal PRIVATE sfv)
target_include_directories(sfv-test-number-decimal PRIVATE tests)
target_compile_options(sfv-test-number-decimal PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME sfv-test-number-decimal COMMAND sfv-test-number-decimal)

add_executable(sfv-test-boolean tests/test-boolean.cpp)
target_link_libraries(sfv-test-boolean PRIVATE sfv)
target_include_directories(sfv-test-boolean PRIVATE tests)
target_compile_options(sfv-test-boolean PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME sfv-test-boolean COMMAND sfv-test-boolean)

add_executable(sfv-test-string tests/test-string.cpp)
target_link_libraries(sfv-test-string PRIVATE sfv)
target_include_directories(sfv-test-string PRIVATE tests)
target_compile_options(sfv-test-string PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME sfv-test-string COMMAND sfv-test-string)

add_executable(sfv-test-token tests/test-token.cpp)
target_link_libraries(sfv-test-token PRIVATE sfv)
target_include_directories(sfv-test-token PRIVATE tests)
target_compile_options(sfv-test-token PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME sfv-test-token COMMAND sfv-test-token)

add_executable(sfv-test-binary tests/test-binary.cpp)
target_link_libraries(sfv-test-binary PRIVATE sfv)
target_include_directories(sfv-test-binary PRIVATE tests)
target_compile_options(sfv-test-binary PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME sfv-test-binary COMMAND sfv-test-binary)

add_executable(sfv-test-date tests/test-date.cpp)
target_link_libraries(sfv-test-date PRIVATE sfv)
target_include_directories(sfv-test-date PRIVATE tests)
target_compile_options(sfv-test-date PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME sfv-test-date COMMAND sfv-test-date)

add_executable(sfv-test-display-string tests/test-display-string.cpp)
target_link_libraries(sfv-test-display-string PRIVATE sfv)
target_include_directories(sfv-test-display-string PRIVATE tests)
target_compile_options(sfv-test-display-string PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME sfv-test-display-string COMMAND sfv-test-display-string)

add_executable(sfv-test-list tests/test-list.cpp)
target_link_libraries(sfv-test-list PRIVATE sfv)
target_include_directories(sfv-test-list PRIVATE tests)
target_compile_options(sfv-test-list PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME sfv-test-list COMMAND sfv-test-list)

add_executable(sfv-test-param-list tests/test-param-list.cpp)
target_link_libraries(sfv-test-param-list PRIVATE sfv)
target_include_directories(sfv-test-param-list PRIVATE tests)
target_compile_options(sfv-test-param-list PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME sfv-test-param-list COMMAND sfv-test-param-list)

add_executable(sfv-test-param-listlist tests/test-param-listlist.cpp)
target_link_libraries(sfv-test-param-listlist PRIVATE sfv)
target_include_directories(sfv-test-param-listlist PRIVATE tests)
target_compile_options(sfv-test-param-listlist PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME sfv-test-param-listlist COMMAND sfv-test-param-listlist)

add_executable(sfv-test-dictionary tests/test-dictionary.cpp)
target_link_libraries(sfv-test-dictionary PRIVATE sfv)
target_include_directories(sfv-test-dictionary PRIVATE tests)
target_compile_options(sfv-test-dictionary PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME sfv-test-dictionary COMMAND sfv-test-dictionary)

add_executable(sfv-test-param-dict tests/test-param-dict.cpp)
target_link_libraries(sfv-test-param-dict PRIVATE sfv)
target_include_directories(sfv-test-param-dict PRIVATE tests)
target_compile_options(sfv-test-param-dict PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME sfv-test-param-dict COMMAND sfv-test-param-dict)

add_executable(sfv-test-key-generated tests/test-key-generated.cpp)
target_link_libraries(sfv-test-key-generated PRIVATE sfv)
target_include_directories(sfv-test-key-generated PRIVATE tests)
target_compile_options(sfv-test-key-generated PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME sfv-test-key-generated COMMAND sfv-test-key-generated)

add_executable(sfv-test-api tests/test-api.cpp)
target_link_libraries(sfv-test-api PRIVATE sfv)
target_compile_options(sfv-test-api PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME sfv-test-api COMMAND sfv-test-api)
