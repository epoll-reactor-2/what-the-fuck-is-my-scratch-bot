#include "spdlog/spdlog.h"
#include "message_handler.hpp"
#include "commands/cmus_status.hpp"
#include "commands/cpp_shell.hpp"
#include "commands/hello_world.hpp"
#include "commands/id_collector.hpp"
#include "commands/kick_all.hpp"
#include "commands/ping_all.hpp"
#include "commands/smtp_mail_send.hpp"
#include "commands/stalking.hpp"
#include "commands/zettelkasten.hpp"

#include "cpp_vk_lib/vk/config/config.hpp"
#include "cpp_vk_lib/vk/long_poll/long_poll.hpp"
#include "cpp_vk_lib/vk/methods/basic.hpp"
#include "cpp_vk_lib/runtime/setup_logger.hpp"

#include <iostream>
#include <thread>

class bot_builder {
public:
  bot_builder(int argc, char* argv[])
      : argc_(argc)
      , argv_(argv) {
    if (argc_ != 2) {
      std::cerr << "usage: ./what_the_fuck_is_my_scratch_bot <config>" << std::endl;
      exit(1);
    }
  }

  void configure() {
    load_config_files();
    setup_logger();
    install_background_commands();
    install_commands();
  }

  void event_loop() {
    asio::io_context io_ctx;
    vk::long_poll api(io_ctx);
    api.on_event(vk::event::type::message_new, [&](const vk::event::common& event) {
      handle_message_new(message_handler_, event);
    });
    api.run();
  }

private:
  void load_config_files() {
    vk::config::load(argv_[1]);
  }

  void setup_logger() {
    runtime::setup_logger(spdlog::level::level_enum::trace);
    spdlog::info("{} threads available to use", vk::config::num_workers());
  }

  void install_commands() {
    message_handler_.install_command(new bot::cpp_shell_command{});
    message_handler_.install_command(new bot::hello_world_command{});
    message_handler_.install_command(new bot::id_collector_command{});
    message_handler_.install_command(new bot::kick_all_command{});
    message_handler_.install_command(new bot::ping_all_command{});
    message_handler_.install_command(new bot::smtp_mail_send_command{});
    message_handler_.install_command(new bot::stalking_command{});
    message_handler_.install_command(new bot::zettelkasten_command{});
  }

  void install_background_commands() {
    for (bot::background_command *cmd : {
      new bot::cmus_status_command{}
    })
      std::thread(&bot::background_command::run, cmd).detach();
  }

  void handle_message_new(const bot::message_handler& handler, const vk::event::common& event) {
    vk::event::message_new message_event = event.get_message_new();
    try {
      handler.process(message_event);
    } catch (std::exception& error) {
      spdlog::error("exception caught from message_new handler: {}", error.what());
      vk::method::messages::send(
        message_event.peer_id(),
        fmt::format("Внутренняя ошибка: {}", error.what())
      );
    }
  }

  int argc_;
  char** argv_;
  bot::message_handler message_handler_;
};

int main(int argc, char* argv[]) {
  bot_builder builder(argc, argv);
  builder.configure();
  builder.event_loop();
}