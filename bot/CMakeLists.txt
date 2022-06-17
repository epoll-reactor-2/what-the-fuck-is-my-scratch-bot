include_directories(../modules/cpp_vk_lib/cpp_vk_lib/include)
include_directories(../modules/cpp_vk_lib/modules/spdlog/include)
include_directories(../modules/cpp_vk_lib/modules/asio/asio/include)
include_directories(include)

add_compile_options(-Wall -Wextra -Werror -Wunused -Wwrite-strings -Wpedantic)
add_compile_options(-fPIC -O3 -march=native -ftree-vectorize -flto -ffast-math)

file(GLOB_RECURSE SRC "src/*.cpp")
add_executable(excuse_all_the_blood_bot ${SRC})

find_library(cpp_vk_lib HINTS "../cpp_vk_lib")
target_link_libraries(excuse_all_the_blood_bot curl cpp_vk_lib simdjson)