#include "cpp_vk_lib/runtime/string_utils/string_utils.hpp"
#include "cpp_vk_lib/runtime/setup_logger.hpp"
#include "cpp_vk_lib/vk/long_poll/long_poll.hpp"
#include "cpp_vk_lib/vk/config/config.hpp"
#include "cpp_vk_lib/vk/events/message_new.hpp"
#include "cpp_vk_lib/vk/methods/basic.hpp"

#include <iostream>

inline bool cpp_vk_lib_curl_verbose = false;


static std::string os_exec(std::string_view cmd) {
  std::string result;
  std::array<char, 128> buffer;
  std::unique_ptr<FILE, decltype (&pclose)> pipe(popen(cmd.data(), "r"), pclose);
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: ./long_poll <config.json>" << std::endl;
    return 1;
  }

  vk::config::load(argv[1]);
  runtime::setup_logger(spdlog::level::level_enum::trace);

  asio::io_context io_context;
  vk::long_poll api(io_context);

  api.on_event(vk::event::type::message_new, [](const vk::event::common& event) {
    vk::event::message_new message = event.get_message_new();
    vk::method::messages::send(message.peer_id(), "response");
  });
  api.run();
  return 0;
}
