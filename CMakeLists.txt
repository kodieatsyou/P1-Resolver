cmake_minimum_required(VERSION 3.20)
project(ability_resolver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(resolver
  src/Db.cpp
  src/World.cpp
  src/Trace.cpp
  src/Resolver.cpp
)

target_include_directories(resolver PUBLIC include)

add_executable(resolver_demo src/main.cpp)
target_link_libraries(resolver_demo PRIVATE resolver)

add_executable(resolver_tests tests/ResolverTests.cpp)
target_link_libraries(resolver_tests PRIVATE resolver)
