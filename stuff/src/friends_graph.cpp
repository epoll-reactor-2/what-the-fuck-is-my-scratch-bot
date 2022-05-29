#include "cpp_vk_lib/runtime/setup_logger.hpp"
#include "cpp_vk_lib/runtime/net/network.hpp"
#include "cpp_vk_lib/vk/config/config.hpp"
#include "cpp_vk_lib/vk/methods/constructor.hpp"

#include "simdjson.h"
#include "spdlog/fmt/fmt.h"

#include <iostream>
#include <fstream>

class graph_builder {
public:
  graph_builder(std::size_t user_id, const std::string &pwd)
    : user_id_(user_id), pwd_(pwd), graph_(pwd + "/graph.gv") {}

  void build() {
    graph_ << "digraph G {\n";

    try {
      build_friend_links(user_id_);
    } catch (std::exception &error) {
      std::cerr << error.what() << std::endl;
    }

    graph_ << "}";
  }

private:
  struct user {
    std::size_t user_id;
    std::string avatar_url;

    bool operator==(const user &rhs) const { return user_id == rhs.user_id; }
    bool operator!=(const user &rhs) const { return !(*this == rhs);        }
    bool operator< (const user &rhs) const { return user_id < rhs.user_id;  }
  };

  std::size_t user_id_;
  std::string pwd_;
  std::ofstream graph_;

  std::vector<user> get_friends(std::size_t user_id) {
    std::vector<user> friends;

    std::string output = vk::method::user_constructor()
      .method("friends.get")
      .param("user_id", std::to_string(user_id))
      .param("fields", "photo_200_orig")
      .perform_request();

    simdjson::dom::parser parser;
    simdjson::dom::object object = parser.parse(output);

    if (object.begin().key() == "error") {
      std::cerr << object << std::endl;
      auto is_private = [](std::size_t user_id) { return user_id == 30; };
      auto is_banned = [](std::size_t user_id) { return user_id == 18; };
      std::size_t uid = object["error"]["error_code"].get_int64().take_value();

      if (is_private(uid) || is_banned(uid))
        return {};

      exit(-1);
    }

    simdjson::dom::array friend_list = object["response"]["items"].get_array();

    for (simdjson::dom::element element: friend_list)
      friends.push_back(user{
        .user_id = element["id"].get_uint64().take_value(),
        .avatar_url = std::string(element["photo_200_orig"].get_string().take_value())
      });

    return friends;
  }

  void build_friend_links(std::size_t id) {
    std::vector<user> users = get_friends(id);
    std::sort(users.begin(), users.end());

    std::vector<
      std::pair<
        /*        user */user,
        /* his friends */std::vector<user>
      >
    > friends;
    for (const auto &[mate, avatar_url] : users)
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

  void write(const user &account, const std::vector<user> &intersection) {
    namespace net = runtime::network;
    net::download(account.avatar_url, pwd_ + "/" + std::to_string(account.user_id) + ".jpg");
    for (auto mate : intersection) {
      net::download(mate.avatar_url, pwd_ + "/" + std::to_string(mate.user_id) + ".jpg");
      graph_ << fmt::format(
        "  label{0}[label=\"\", image=\"{2}/{0}.jpg\"];\n"
        "  label{1}[label=\"\", image=\"{2}/{1}.jpg\"];\n"
        "  {{\n"
        "    label{0} -> label{1};\n"
        "  }}\n",
        account.user_id, mate.user_id, pwd_
      );
    }
  }
};

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: ./friends_graph <config.json> <pwd> <user_id>\n";
    return 1;
  }
  vk::config::load(argv[1]);
  runtime::setup_logger(spdlog::level::level_enum::debug);

  graph_builder(std::stoi(argv[3]), argv[2]).build();
}