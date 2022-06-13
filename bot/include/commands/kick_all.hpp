#ifndef EXCUSE_ALL_THE_BLOOD_BOT_KICK_ALL_HPP
#define EXCUSE_ALL_THE_BLOOD_BOT_KICK_ALL_HPP

#include "command.hpp"

#include "cpp_vk_lib/runtime/setup_logger.hpp"
#include "cpp_vk_lib/vk/config/config.hpp"
#include "cpp_vk_lib/vk/events/message_new.hpp"
#include "cpp_vk_lib/vk/methods/constructor.hpp"
#include "cpp_vk_lib/vk/methods/basic.hpp"

#include "simdjson.h"

namespace bot {

class kick_all_command : public command {
  virtual std::string_view trigger() const override {
    return "/пидорнуть_быдло";
  }
  virtual void run(
    const vk::event::message_new &event,
    const std::vector<std::string_view> &args
  ) override {
    auto chat_id = args.front();

    const std::string get_chat_response = vk::method::user_constructor()
      .method("messages.getChat")
      .param("chat_id", chat_id)
      .param("fields", "uid")
      .perform_request();
    simdjson::dom::parser parser;
    simdjson::dom::object get_chat_object = parser.parse(get_chat_response);
    if (get_chat_object.begin().key() == "error") {
      vk::method::messages::send(
        event.peer_id(),
        "Error: " + to_string(get_chat_object["error"]["error_msg"])
      );
      return;
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
};

} // namespace bot

#endif // EXCUSE_ALL_THE_BLOOD_BOT_KICK_ALL_HPP