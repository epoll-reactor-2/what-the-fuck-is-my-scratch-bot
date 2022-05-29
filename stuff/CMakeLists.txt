include_directories(../modules/cpp_vk_lib/cpp_vk_lib/include)
include_directories(../modules/cpp_vk_lib/modules/spdlog/include)
include_directories(../modules/cpp_vk_lib/modules/asio/asio/include)
include_directories(include)

find_library(cpp_vk_lib HINTS "../cpp_vk_lib")
link_libraries(cpp_vk_lib simdjson)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -O3")

file(GLOB_RECURSE srcs "*/*.cpp")
foreach(file ${srcs})
  get_filename_component(executable_name ${file} NAME_WE)
  add_executable(${executable_name} ${file})
endforeach()