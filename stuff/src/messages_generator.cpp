#include "cpp_vk_lib/runtime/setup_logger.hpp"
#include "cpp_vk_lib/runtime/net/network.hpp"
#include "cpp_vk_lib/runtime/string_utils/implementation/split.hpp"
#include "cpp_vk_lib/vk/methods/constructor.hpp"
#include "cpp_vk_lib/vk/config/config.hpp"

#include <iostream>
#include <thread>
#include <random>
#include <list>
#include <fstream>

int main(int argc, char* argv[]) {
  if (argc != 5) {
    std::cerr << "Usage ./anekdoti <config.json> <chat-id> <anekdoti.txt>\n";
    return -1;
  }
  vk::config::load(argv[1]);
  runtime::setup_logger(spdlog::level::level_enum::trace);

  const std::string chat_id = std::to_string(2000000000 + std::stoi(argv[2]));

  std::ifstream file(argv[4]);
  std::vector<std::string> payload;
  std::string line;
  while (std::getline(file, line)) {
    payload.push_back(std::move(line));
  }

  std::vector<std::string_view> wait_args = runtime::string_utils::split(argv[3], "-");
  if (wait_args.size() != 2) {
  	std::cerr << "time range {}-{} expected\n";
  	return -1;
  }
  int32_t wait_from = std::stoi(wait_args[0].data());
  int32_t wait_to   = std::stoi(wait_args[1].data());

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> wait_generator(wait_from, wait_to);

  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(wait_generator(gen)));
    std::uniform_int_distribution<> anekdoti_generator(0, payload.size() - 1);
    size_t index = anekdoti_generator(gen);
    std::cout << vk::method::user_constructor()
      .method("messages.send")
      .param("peer_id", chat_id)
      .param("message", payload[index])
      .param("random_id", "0")
      .perform_request() << std::endl;
    payload[index] = "";
  }
}
