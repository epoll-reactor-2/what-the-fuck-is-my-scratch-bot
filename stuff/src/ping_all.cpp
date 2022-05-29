#include "cpp_vk_lib/runtime/setup_logger.hpp"
#include "cpp_vk_lib/runtime/string_utils/implementation/join.hpp"
#include "cpp_vk_lib/vk/config/config.hpp"
#include "cpp_vk_lib/vk/methods/constructor.hpp"

#include "simdjson.h"
#include "spdlog/fmt/fmt.h"

#include <iostream>

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage ./ping_all <config.json> <chat-id>\n";
    return -1;
  }
  const char* config_path = argv[1];
  const char* chat_id = argv[2];
  vk::config::load(config_path);
  runtime::setup_logger(spdlog::level::level_enum::trace);

  const std::string get_chat_response = vk::method::user_constructor()
    .method("messages.getChat")
    .param("chat_id", chat_id)
    .param("name_case", "nom")
    .param("fields", "uid,screen_name")
    .perform_request();
  simdjson::dom::parser parser;
  simdjson::dom::object get_chat_object = parser.parse(get_chat_response);
  if (get_chat_object.begin().key() == "error") {
    std::cerr << "Error: " << get_chat_object["error"]["error_msg"] << std::endl;
    return -1;
  }
  simdjson::dom::array users = get_chat_object["response"]["users"].get_array();
  std::vector<std::string> users_to_ping;
  users_to_ping.reserve(users.size());
  namespace su = runtime::string_utils;
  for (simdjson::dom::object user : users) {
    std::cout << user << std::endl;
    if (user.begin().key() != "id") continue;
    std::string_view screen_name;
    if (auto error = user["screen_name"].get(screen_name); !error) {
      users_to_ping.push_back(fmt::format("@{}", screen_name));
    }
  }
  std::string chat_id_num = std::to_string(std::stoul(chat_id) + 2000000000);
  vk::method::user_constructor()
    .method("messages.send")
    .param("peer_id", chat_id_num)
    .param("message", su::join(users_to_ping, ' '))
    .param("random_id", "0")
    .request_without_output();
}
