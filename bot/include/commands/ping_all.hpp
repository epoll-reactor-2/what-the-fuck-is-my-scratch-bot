#ifndef EXCUSE_ALL_THE_BLOOD_BOT_PING_ALL_HPP
#define EXCUSE_ALL_THE_BLOOD_BOT_PING_ALL_HPP

#include "command.hpp"

#include "cpp_vk_lib/runtime/string_utils/implementation/join.hpp"
#include "cpp_vk_lib/vk/events/message_new.hpp"
#include "cpp_vk_lib/vk/methods/constructor.hpp"
#include "cpp_vk_lib/vk/methods/basic.hpp"

#include "simdjson.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/spdlog.h"

namespace bot {

class ping_all_command : public command {
  virtual std::string trigger() const override {
    return "/пингануть_стадо";
  }
  virtual void run(
    const vk::event::message_new &event,
    const std::vector<std::string_view> &args
  ) override {
    auto chat_id = args.front();
    const std::string get_chat_response = vk::method::user_constructor()
      .method("messages.getChat")
      .param("chat_id", chat_id)
      .param("name_case", "nom")
      .param("fields", "uid,screen_name")
      .perform_request();
    simdjson::dom::parser parser;
    simdjson::dom::object get_chat_object = parser.parse(get_chat_response);
    if (get_chat_object.begin().key() == "error") {
      vk::method::messages::send(
        event.peer_id(),
        fmt::format("Ошибка: {}", get_chat_object["error"]["error_msg"])
      );
      return;
    }
    simdjson::dom::array users = get_chat_object["response"]["users"].get_array();
    std::vector<std::string> users_to_ping;
    users_to_ping.reserve(users.size());
    namespace su = runtime::string_utils;
    for (simdjson::dom::object user : users) {
      if (user.begin().key() != "id")
        continue;
      std::string_view screen_name;
      if (auto error = user["screen_name"].get(screen_name); !error) {
        auto reference = fmt::format("@{}", screen_name);
        users_to_ping.push_back(reference);
        spdlog::info("ping_all_command: ебаная сволочь {}", reference);
      }
    }
    std::string chat_id_num = std::to_string(std::stoul(chat_id.data()) + 2000000000);
    vk::method::user_constructor()
        .method("messages.send")
        .param("peer_id", chat_id_num)
        .param("message", su::join(users_to_ping, ' '))
        .param("random_id", "0")
        .request_without_output();
  }
};

} // namespace bot

#endif // EXCUSE_ALL_THE_BLOOD_BOT_PING_ALL_HPP
