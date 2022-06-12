#include "message_handler.hpp"

#include "string_utils.hpp"

#include "spdlog/spdlog.h"

#include "cpp_vk_lib/runtime/string_utils/implementation/split.hpp"

bot::message_handler::~message_handler() {
  for (const auto& [_, cmd] : commands_)
    delete cmd;
}

void bot::message_handler::process(const vk::event::message_new& event) const {
  std::string text = event.text();
  spdlog::info("incoming message event: {} from {}", text, event.peer_id());

  if (auto iterator = commands_.find(string_utils::get_first(text)); iterator != commands_.end()) {
    const auto&[_, command_pointer] = *iterator;
    command_pointer->run(
        event,
        runtime::string_utils::whitespace_split(string_utils::cut_first(text))
    );
  }
}

void bot::message_handler::install_command(command* command) {
  commands_.emplace(command->trigger(), command);
}