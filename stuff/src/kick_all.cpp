#include "cpp_vk_lib/runtime/setup_logger.hpp"
#include "cpp_vk_lib/vk/config/config.hpp"
#include "cpp_vk_lib/vk/methods/constructor.hpp"

#include "simdjson.h"

#include <iostream>

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage ./kick_all <config.json> <chat-id>\n";
    return -1;
  }
  vk::config::load(argv[1]);
  runtime::setup_logger(spdlog::level::level_enum::trace);

  const char* chat_id = argv[2];
  const std::string get_chat_response = vk::method::user_constructor()
    .method("messages.getChat")
    .param("chat_id", chat_id)
    .param("fields", "uid")
    .perform_request();
  simdjson::dom::parser parser;
  simdjson::dom::object get_chat_object = parser.parse(get_chat_response);
  if (get_chat_object.begin().key() == "error") {
    std::cerr << "Error: " << get_chat_object["error"]["error_msg"] << std::endl;
    return -1;
  }
  for (auto user : get_chat_object["response"]["users"].get_array()) {
    const int64_t user_id { user["id"] };
    std::cout << vk::method::group_constructor()
      .method("messages.removeChatUser")
      .param("chat_id", chat_id)
      .param("member_id", std::to_string(user_id))
      .perform_request() << std::endl;
  }
}
