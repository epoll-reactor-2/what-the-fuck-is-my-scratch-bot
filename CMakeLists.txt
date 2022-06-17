cmake_minimum_required(VERSION 3.20)

project(
  excuse_all_the_blood_bot
  DESCRIPTION "Personal bot with wierd things for https://vk.com/excuse_all_the_blood"
  LANGUAGES CXX
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../build)
set(EXECUTABLE_OUTPUT_PATH ../build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../build)

set(CMAKE_C_COMPILER /usr/bin/gcc CACHE PATH "" FORCE)
set(CMAKE_CXX_COMPILER /usr/bin/g++ CACHE PATH "" FORCE)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MACOSX_RPATH OFF)
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)

add_subdirectory(modules)
add_subdirectory(bot)
