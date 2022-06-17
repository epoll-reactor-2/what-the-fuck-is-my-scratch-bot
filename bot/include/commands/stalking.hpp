#ifndef EXCUSE_ALL_THE_BLOOD_BOT_STALKING_HPP
#define EXCUSE_ALL_THE_BLOOD_BOT_STALKING_HPP

#include "command.hpp"
#include "method_wrappers.hpp"

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
#include <fstream>

namespace bot {
namespace {

class friends_common_subscriptions {
public:
  friends_common_subscriptions(size_t id) : id_(id) {}

  operator std::string() const {
    return get_common_with_friends_groups();
  }

private:
  size_t id_;

  std::vector<size_t> get_friends() const {
    std::string response = vk::method::user_constructor()
      .method("friends.get")
      .param("user_id", std::to_string(id_))
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

  std::string get_subscriptions(size_t id) const {
    return vk::method::user_constructor()
      .method("users.getSubscriptions")
      .param("user_id", std::to_string(id))
      .perform_request();
  }

  std::vector<size_t> extract_subscriptions(simdjson::dom::array array) const {
    std::vector<size_t> result;
    result.reserve(array.size());
    for (auto object : array)
      result.push_back(object.get_int64().value());
    return result;
  }

  std::string get_common_with_friends_groups() const {
    std::vector<size_t> user_friends = get_friends();
    std::string response = get_subscriptions(id_);

    simdjson::dom::parser parser;
    simdjson::dom::object parsed = parser.parse(response);

    std::vector<size_t> user_subscriptions
      = extract_subscriptions(parsed["response"]["groups"]["items"].get_array());
    std::sort(user_subscriptions.begin(), user_subscriptions.end());

    using id_mapping
      = std::vector<
          std::pair<
            /* friend */ size_t,
            /* his subscriptions */ std::vector<size_t>
          >
        >;

    id_mapping friend_groups_mapping;
    for (size_t friend_ : user_friends) {
      response = get_subscriptions(friend_);
      parsed = parser.parse(response);
      if (parsed.begin().key() == "error")
        continue;
      std::vector<size_t> groups_list
          = extract_subscriptions(parsed["response"]["groups"]["items"]);
      friend_groups_mapping.emplace_back(friend_, std::move(groups_list));
    }

    id_mapping common_groups_mapping;
    for (auto &&[friend_, friend_subscriptions] : friend_groups_mapping) {
      std::sort(friend_subscriptions.begin(), friend_subscriptions.end());
      std::vector<size_t> intersection;
      std::set_intersection(
          user_subscriptions.begin(),   user_subscriptions.end(),
        friend_subscriptions.begin(), friend_subscriptions.end(),
        std::back_inserter(intersection)
      );
      common_groups_mapping.emplace_back(friend_, std::move(intersection));
    }

    std::string output;
    for (auto &&[friend_, friend_subscriptions] : common_groups_mapping) {
      if (friend_subscriptions.empty())
        continue;
      output += "@id" + std::to_string(friend_) + ":\n";
      for (auto subscription : friend_subscriptions)
        output += "https://vk.com/club" + std::to_string(subscription) + '\n';
      output += "\n\n";
    }

    return output;
  }
};

class friends_graph {
public:
  friends_graph(size_t user_id, size_t peer_id, std::string_view filename)
    : user_id_(user_id)
    , peer_id_(peer_id)
    , filename_(string_utils::thread_local_filename(filename, "gv"))
    , graph_(filename_) {}

  operator vk::attachment::attachment_ptr_t() const {
    graph_ << "digraph G {\n";

    try {
      build_friend_links(user_id_);
    } catch (std::exception& error) {
      std::cerr << error.what() << std::endl;
    }

    graph_ << "}";
    graph_.close();

    std::string image_filename
      = bot::string_utils::thread_local_filename("friends_graph", "jpg");
    system(
      fmt::format(
        "dot -Tjpg {} -o {}", filename_, image_filename
      ).c_str()
    );

    std::string server
      = bot::method_wrappers::get_messages_upload_server(peer_id_);
    return bot::method_wrappers::save_messages_photo(image_filename, server);
  }

private:
  struct user {
    size_t user_id;
    std::string avatar_url;

    bool operator==(const user& rhs) const {
      return user_id == rhs.user_id;
    }
    bool operator!=(const user& rhs) const {
      return !(*this == rhs);
    }
    bool operator<(const user& rhs) const {
      return user_id < rhs.user_id;
    }
  };

  size_t user_id_;
  size_t peer_id_;
  std::string filename_;
  mutable std::ofstream graph_;

  std::vector<user> get_friends(size_t user_id) const {
    std::vector<user> friends;

    std::string output = vk::method::user_constructor()
      .method("friends.get")
      .param("user_id", std::to_string(user_id))
      .param("fields", "photo_200_orig")
      .perform_request();

    simdjson::dom::parser parser;
    simdjson::dom::object object = parser.parse(output);

    if (object.begin().key() == "error") {
      auto is_private = [](size_t user_id) { return user_id == 30; };
      auto is_banned  = [](size_t user_id) { return user_id == 18; };
      size_t uid = object["error"]["error_code"].get_int64().take_value();

      if (is_private(uid) || is_banned(uid))
        return {};

      exit(-1);
    }

    simdjson::dom::array friend_list = object["response"]["items"].get_array();

    for (simdjson::dom::element element : friend_list)
      friends.push_back(
        user{
          .user_id    = element["id"].get_uint64().take_value(),
          .avatar_url = std::string(element["photo_200_orig"].get_string().take_value())
        }
      );

    return friends;
  }

  void build_friend_links(std::size_t id) const {
    std::vector<user> users = get_friends(id);
    std::sort(users.begin(), users.end());

    std::vector<
      std::pair<
        /*        user */user,
        /* his friends */std::vector<user>
      >
    > friends;
    for (auto [mate, avatar_url] : users)
      friends.emplace_back(
        user {
          .user_id    = mate,
          .avatar_url = avatar_url
        },
        get_friends(mate)
      );
    for (auto &&[account, user_friends] : friends) {
      std::sort(user_friends.begin(), user_friends.end());
      std::vector<user> intersection;
      std::set_intersection(
          users       .begin(), users       .end(),
          user_friends.begin(), user_friends.end(),
          std::back_inserter(intersection)
      );
      write(account, intersection);
    }
  }

  void write(const user& account, const std::vector<user>& intersection) const {
    namespace net = runtime::network;
    net::download(account.avatar_url, std::to_string(account.user_id) + ".jpg");
    for (const user& mate : intersection) {
      net::download(mate.avatar_url, std::to_string(mate.user_id) + ".jpg");
      graph_ << fmt::format(
        "  label{0}[label=\"\", image=\"{0}.jpg\"];\n"
        "  label{1}[label=\"\", image=\"{1}.jpg\"];\n"
        "  {{\n"
        "    label{0} -> label{1};\n"
        "  }}\n",
        account.user_id, mate.user_id
      );
    }
  }
};

} // namespace

class stalking_command : public command {
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

    vk::attachment::attachment_ptr_t friends
      = friends_graph{id, static_cast<size_t>(event.peer_id()), "friends_graph"};

    std::string output;
    output += friends_common_subscriptions{id};

    vk::method::messages::send(event.peer_id(), output, std::move(friends));
  }
};

} // namespace bot

#endif // EXCUSE_ALL_THE_BLOOD_BOT_STALKING_HPP
