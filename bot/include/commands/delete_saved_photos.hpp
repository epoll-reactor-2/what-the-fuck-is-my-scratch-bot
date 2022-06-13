#ifndef EXCUSE_ALL_THE_BLOOD_BOT_DELETE_SAVED_PHOTOS_HPP
#define EXCUSE_ALL_THE_BLOOD_BOT_DELETE_SAVED_PHOTOS_HPP

#include "command.hpp"

#include "cpp_vk_lib/vk/events/message_new.hpp"
#include "cpp_vk_lib/vk/methods/constructor.hpp"
#include "cpp_vk_lib/vk/methods/basic.hpp"

#include "simdjson.h"
#include "asio.hpp"

namespace bot {

class delete_saved_photos_command : public command {
  virtual std::string_view trigger() const override {
    return "/__удалить_сохранёнки";
  }
  virtual void run(
    const vk::event::message_new &event,
    const std::vector<std::string_view>&
  ) override {
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
      asio::post(io_ctx,
        [
          element = std::move(element),
          parsed_output = std::ref(parsed_output),
          event = std::ref(event)
        ] {
        std::string output = vk::method::user_constructor()
          .method("photos.delete")
          .param("owner_id", "499047616")
          .param("photo_id", std::to_string(element["id"].get_int64()))
          .perform_request();

        if (parsed_output.get().begin().key() != "response")
          vk::method::messages::send(event.get().peer_id(), output);
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
};

} // namespace bot

#endif // EXCUSE_ALL_THE_BLOOD_BOT_DELETE_SAVED_PHOTOS_HPP
