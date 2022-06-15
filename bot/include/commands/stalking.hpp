#ifndef EXCUSE_ALL_THE_BLOOD_BOT_STALKING_HPP
#define EXCUSE_ALL_THE_BLOOD_BOT_STALKING_HPP

#include "command.hpp"

#include "cpp_vk_lib/runtime/setup_logger.hpp"
#include "cpp_vk_lib/vk/config/config.hpp"
#include "cpp_vk_lib/vk/events/message_new.hpp"
#include "cpp_vk_lib/vk/methods/constructor.hpp"
#include "cpp_vk_lib/vk/methods/basic.hpp"

#include "simdjson.h"
#include "asio.hpp"

#include <future>
#include <vector>
#include <deque>
#include <iostream>

namespace bot {

class stalking_command : public command {
  std::vector<size_t> get_friends(size_t user_id) {
    std::string response = vk::method::user_constructor()
      .method("friends.get")
      .param("user_id", std::to_string(user_id))
      .param("order", "hints")
      .perform_request();
    simdjson::dom::parser parser;
    simdjson::dom::object parsed = parser.parse(response);
    if (parsed.begin().key() == "error") {
      return {};
    }
    std::vector<size_t> friends;
    friends.reserve(parsed["response"]["count"].get_int64());
    auto array = parsed["response"]["items"].get_array();
    friends.reserve(array.size());
    for (auto element : parsed["response"]["items"].get_array()) {
      friends.push_back(element.get_uint64());
    }
    return friends;
  }

  std::string get_subscriptions(size_t id) {
    return vk::method::user_constructor()
      .method("users.getSubscriptions")
      .param("user_id", std::to_string(id))
      .perform_request();
  }

  std::vector<size_t> extract_subscriptions(
    simdjson::dom::array array
  ) {
      std::vector<size_t> result;
      for (auto object : array)
        result.push_back(object.get_int64().value());
      return result;
  }

  std::string get_common_with_friends_groups(size_t user) {
    std::vector<size_t> user_friends = get_friends(user);
    std::string response = get_subscriptions(user);

    simdjson::dom::parser parser;
    simdjson::dom::object parsed = parser.parse(response);

    std::vector<size_t> user_subscriptions
      = extract_subscriptions(parsed["response"]["groups"]["items"].get_array());
    std::sort(user_subscriptions.begin(), user_subscriptions.end());

    std::vector<
      std::pair<
        /* friend */ size_t,
        /* his subscriptions */ std::vector<size_t>
      >
    > friend_groups_mapping;
    for (size_t friend_ : user_friends) {
      response = get_subscriptions(friend_);
      parsed = parser.parse(response);
      if (parsed.begin().key() == "error")
        continue;
      std::vector<size_t> groups_list
        = extract_subscriptions(parsed["response"]["groups"]["items"]);
      friend_groups_mapping.emplace_back(friend_, std::move(groups_list));
    }

    std::vector<
      std::pair<
        /* friend */ size_t,
        /* his subscriptions */ std::vector<size_t>
      >
    > common_groups_mapping;
    for (auto [friend_, friend_subscriptions] : friend_groups_mapping) {
      std::sort(friend_subscriptions.begin(), friend_subscriptions.end());
      std::vector<size_t> intersection;
      std::set_intersection(
          user_subscriptions.begin(),   user_subscriptions.end(),
        friend_subscriptions.begin(), friend_subscriptions.end(),
        std::back_inserter(intersection)
      );
      common_groups_mapping.emplace_back(
        friend_, std::move(intersection)
      );
    }

    std::string output;
    for (auto [friend_, friend_subscriptions] : common_groups_mapping) {
      if (friend_subscriptions.empty())
        continue;
      output += "@id" + std::to_string(friend_) + ":\n";
      for (auto subscription : friend_subscriptions)
        output += "https://vk.com/club" + std::to_string(subscription) + '\n';
      output += "\n\n";
    }

    return output;
  }

  virtual std::string_view trigger() const override {
    return "/сталкеринг";
  }
  virtual void run(
    const vk::event::message_new &event,
    const std::vector<std::string_view> &args
  ) override {
    if (args.size() != 1) {
      vk::method::messages::send(event.peer_id(), "Ожидается айди страницы.");
      return;
    }

    size_t id = std::stoull(args.front().data());

    std::string output;
    output += get_common_with_friends_groups(id);

    vk::method::messages::send(event.peer_id(), output);
  }
};

} // namespace bot

#endif // EXCUSE_ALL_THE_BLOOD_BOT_STALKING_HPP
