#include "cpp_vk_lib/vk/methods/constructor.hpp"
#include "cpp_vk_lib/vk/config/config.hpp"

#include "simdjson.h"
#include "asio.hpp"

#include <iostream>

int main() {
  vk::config::load("/home/machen/text/configs/config.json");
  std::string output = vk::method::user_constructor()
    .method("photos.get")
    .param("owner_id", "499047616")
    .param("album_id", "saved")
    .param("count", "1000")
    .perform_request();

  simdjson::dom::parser parser;
  simdjson::dom::object parsed_output = parser.parse(output);

  asio::io_context io_ctx;
  int tasks_scheduled = 0;

  for (auto element : parsed_output["response"]["items"].get_array()) {
    ++tasks_scheduled;
    asio::post(io_ctx, [=] {
      std::string output = vk::method::user_constructor()
        .method("photos.delete")
        .param("owner_id", "499047616")
        .param("photo_id", std::to_string(element["id"].get_int64()))
        .perform_request();

      simdjson::dom::parser parser;
      simdjson::dom::object parsed_output = parser.parse(output);

      if (parsed_output.begin().key() == "response") {
        static int iterator = 0;
        std::cout << "OK " << iterator++ << std::endl;
      } else {
        std::cout << "ERROR: " << parsed_output << std::endl;
      }
    });
    if (tasks_scheduled >= 8) {
      std::vector<std::thread> threads;
      threads.reserve(8);
      for (size_t i = 0; i < 8; ++i) {
        threads.emplace_back([&] {
          io_ctx.run();
        });
      }
      io_ctx.run();
      for (auto& t : threads) {
        t.join();
      }
      io_ctx.restart();
      tasks_scheduled = 0;
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
}