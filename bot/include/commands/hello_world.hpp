#ifndef WHAT_THE_FUCK_IS_MY_SCRATCH_BOT_HELLO_WORLD_HPP
#define WHAT_THE_FUCK_IS_MY_SCRATCH_BOT_HELLO_WORLD_HPP

#include "../command.hpp"

#include "cpp_vk_lib/vk/events/message_new.hpp"
#include "cpp_vk_lib/vk/methods/basic.hpp"

namespace bot {

class hello_world_command : public command {
  std::string_view trigger() const override {
    return "/hello";
  }
  void run(
    const vk::event::message_new& event,
    const std::vector<std::string_view>&
  ) override {
    vk::method::messages::send(event.peer_id(), "Hello, World!");
  }
};

} // namespace bot

#endif // WHAT_THE_FUCK_IS_MY_SCRATCH_BOT_HELLO_WORLD_HPP
