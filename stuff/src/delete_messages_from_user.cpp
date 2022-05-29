#include "cpp_vk_lib/vk/methods/constructor.hpp"
#include "cpp_vk_lib/vk/config/config.hpp"
#include "cpp_vk_lib/runtime/setup_logger.hpp"

#include "simdjson.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/fmt.h"

#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage ./delete <config.json> <user-ids.txt>\n";
    return 1;
  }
  const char* log_path = argv[1];
  const char* user_ids_file = argv[2];
  vk::config::load(log_path);
  runtime::setup_logger(spdlog::level::level_enum::info);
  std::ifstream file(user_ids_file);
  std::vector<int64_t> user_ids;
  std::copy(
    std::istream_iterator<int64_t>(file),
    std::istream_iterator<int64_t>(),
    std::back_inserter(user_ids));
  std::sort(user_ids.begin(), user_ids.end());
  get_server_again:
  simdjson::dom::parser global_parser;
  const std::string server = vk::method::user_constructor()
    .method("messages.getLongPollServer")
    .param("need_pts", "0")
    .perform_request();
  int64_t ts = 0;
  const simdjson::dom::object parsed = global_parser.parse(server);
  while (true) {
    simdjson::dom::parser parser;
    if (ts == 0) { ts = parsed["response"]["ts"].get_int64(); }
    const std::string response = vk::method::raw_constructor()
      .method(std::string(parsed["response"]["server"]) + "?")
      .param("act", "a_check")
      .param("key", parsed["response"]["key"])
      .param("ts", std::to_string(ts))
      .param("wait", "60")
      .param("mode", "2")
      .param("version", "3")
      .perform_request();
    const simdjson::dom::object response_object = parser.parse(response);
    if (response_object.begin().key() == "failed") {
      spdlog::info("Get new server...");
      goto get_server_again;
    }
    ts = response_object["ts"].get_int64();
    for (const simdjson::dom::array update : response_object["updates"]) {
      if (update.at(0).get_int64() != 4) {
        continue;
      }
      const int64_t message_id { update.at(1) };
      const int64_t peer_id { update.at(3) };
      const int64_t from_id { std::stoi(std::string_view(update.at(6).get_object()["from"]).data()) };
      if (!std::binary_search(user_ids.begin(), user_ids.end(), from_id)) {
        continue;
      }
      spdlog::info("Чмырло {} что-то там хрюкнуло", [](int64_t from_id){
        const std::string response = vk::method::user_constructor()
          .method("users.get")
          .param("user_ids", std::to_string(from_id))
          .param("name_case", "Nom")
          .perform_request();
        simdjson::dom::parser parser;
        const simdjson::dom::object user { parser.parse(response)["response"].get_array().at(0) };
        return fmt::format("{} {}", std::string_view(user["first_name"]), std::string_view(user["last_name"]));
      }(from_id));
      vk::method::user_constructor()
        .method("messages.delete")
        .param("message_ids", std::to_string(message_id))
        .param("peer_id", std::to_string(peer_id))
        .request_without_output();
    }
  }
}
