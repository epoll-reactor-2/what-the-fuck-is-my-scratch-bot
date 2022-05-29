// I DO NOT GIVE A FUCK.
#include "cpp_vk_lib/vk/methods/constructor.hpp"
#include "cpp_vk_lib/vk/config/config.hpp"
#include "cpp_vk_lib/vk/attachment/attachment.hpp"
// #include "cpp_vk_lib/runtime/setup_logger.hpp"
// #include "cpp_vk_lib/runtime/signal_handlers.hpp"
#include "cpp_vk_lib/runtime/string_utils/implementation/split.hpp"

#include "simdjson.h"
#include "spdlog/spdlog.h"

#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
if (argc != 3) {
std::cerr << "Usage ./delete <config.json> <peer-id>\n";
return 1;
}
const char* log_path = argv[1];
const char* peer_id = argv[2];
vk::config::load(log_path);
// runtime::setup_signal_handlers();
// runtime::setup_logger(spdlog::level::level_enum::trace);
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
spdlog::info("response: {}", simdjson::to_string(response_object));
if (response_object.begin().key() == "failed") {
spdlog::info("Get new server...");
goto get_server_again;
}
ts = response_object["ts"].get_int64();
spdlog::info("ts: {}", ts);
for (const simdjson::dom::array update : response_object["updates"]) {
if (update.at(0).get_int64() != 4) {
continue;
}
const std::string text { update.at(5).get_string().take_value() };
const int64_t chat_id { update.at(3) };
const int64_t from_id { std::stoi(std::string_view(update.at(6).get_object()["from"]).data()) };
if (from_id == 499047616) {
continue;
}
if (chat_id != (2000000000 + atoi(peer_id))) {
continue;
}
const auto words = runtime::string_utils::split(text, ' ');
const auto longest = std::max_element(words.begin(), words.end(), [](auto const& l, auto const& r) {
return l.size() < r.size();
});
if (longest->size() < 12) {
continue;
}
spdlog::info("text: {}, length: {}", *longest, longest->length());
const std::string raw_photo = vk::method::user_constructor()
.method("photos.search")
.param("q", *longest)
.param("count", "100")
.perform_request();
simdjson::dom::parser photo_parser;
const simdjson::dom::object photo_object = photo_parser.parse(raw_photo);
if (simdjson::dom::array photos = photo_object["response"]["items"].get_array(); photos.size() != 0) {
srand(time(nullptr));
auto photo_tape = photos.at(rand() % (photos.size()));
std::unique_ptr<vk::attachment::photo> photo = std::make_unique<vk::attachment::photo>(
photo_tape.at(0)["owner_id"].get_int64(),
photo_tape.at(0)[      "id"].get_int64()
);
vk::method::user_constructor()
.method("messages.send")
.param("peer_id", std::to_string(2000000000 + atoi(peer_id)))
.param("message", "")
.param("random_id", "0")
.param("attachment", photo->value())
.perform_request();
}
}
}
}
