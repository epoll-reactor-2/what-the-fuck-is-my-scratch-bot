#ifndef WHAT_THE_FUCK_IS_MY_SCRATCH_BOT_ID_COLLECTOR_HPP
#define WHAT_THE_FUCK_IS_MY_SCRATCH_BOT_ID_COLLECTOR_HPP

#include "command.hpp"

#include "cpp_vk_lib/vk/events/message_new.hpp"
#include "cpp_vk_lib/vk/methods/constructor.hpp"
#include "cpp_vk_lib/vk/methods/basic.hpp"

#include "simdjson.h"
#include "asio.hpp"

#include <vector>
#include <iostream>

namespace bot {

class id_collector_command : public command {
  std::vector<size_t> get_friends(size_t user_id, asio::error_code& errc) {
    std::string response = vk::method::user_constructor()
      .method("friends.get")
      .param("user_id", std::to_string(user_id))
      .param("order", "hints")
      .perform_request();
    simdjson::dom::parser parser;
    simdjson::dom::object parsed = parser.parse(std::move(response));
    if (parsed.begin().key() == "error") {
      if (parsed["error"]["error_code"].get_int64() == 29) { // Rate limit
        errc = asio::error::connection_refused;
      }
      return {};
    }
    std::vector<size_t> friends;
    friends.reserve(parsed["response"]["count"].get_int64());
    auto array = parsed["response"]["items"].get_array();
    friends.reserve(array.size());
    for (auto element : parsed["response"]["items"].get_array()) {
      friends.push_back(element.get_uint64().take_value());
    }
    total_ids_collected += friends.size();
    return friends;
  }
  std::string_view trigger() const override {
    return "/friends_graph";
  }
  void run(
    const vk::event::message_new &event,
    const std::vector<std::string_view> &args
  ) override {
    static_cast<void>(args);
    asio::io_context io_context(2);

    auto friends_of_user = [&](auto begin, auto end) mutable -> std::vector<size_t> {
      std::vector<size_t> all_users;
      all_users.reserve(std::distance(begin, end));
      asio::error_code errc;
      for (; begin != end; ++begin) {
        asio::post(io_context, std::bind([&](auto it) mutable {
          auto users = get_friends(*it, errc);
            if (errc == asio::error::connection_refused) {
              vk::method::messages::send(
                event.peer_id(),
                "Ошибка: " + errc.message()
              );
            return;
          }
          std::mutex mutex;
          [[maybe_unused]]
          std::lock_guard<std::mutex> lock(mutex);
          std::move(users.begin(), users.end(), std::back_inserter(all_users));
        }, begin));
      }
      std::vector<std::thread> threads;
      threads.reserve(8);
      for (size_t i = 0; i < 8; ++i) {
        threads.emplace_back([&] {
          io_context.run();
        });
      }
      io_context.run();
      for (auto& t : threads) {
        t.join();
      }
      io_context.restart();
      return all_users;
    };

    asio::error_code errc;
    std::vector<size_t> friend_list = get_friends(std::stoull(args.front().data()), errc);
    if (errc) {
      vk::method::messages::send(event.peer_id(), "Ошибка: " + errc.message());
      return;
    }
    std::vector<size_t> buffer = friends_of_user(friend_list.begin(), friend_list.end());
    std::vector<size_t> total_friends;
    total_friends.reserve(1'000'000);
    std::move(buffer.begin(), buffer.end(), std::back_inserter(total_friends));

    for (size_t friend_lookup_depth = 0; friend_lookup_depth < 1; ++friend_lookup_depth) {
      static constexpr long step = 32;
      for (auto it = total_friends.begin(); std::distance(it, total_friends.end()) > step; std::advance(it, step)) {
        for (size_t id : friends_of_user(it, it + step)) {
          total_friends.push_back(id);
        }
      }
    }

    total_friends.erase(std::unique(total_friends.begin(), total_friends.end()), total_friends.end());

    vk::method::messages::send(event.peer_id(), "Собрано: " + std::to_string(total_friends.size()) + " айдишников.");
  }

  static inline size_t total_ids_collected = 0;
};

} // namespace bot

#endif // WHAT_THE_FUCK_IS_MY_SCRATCH_BOT_ID_COLLECTOR_HPP
