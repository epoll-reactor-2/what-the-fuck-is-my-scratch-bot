#ifndef NOBODY_CAN_FUCK_WITH_ME_BOT_COMMAND_HPP
#define NOBODY_CAN_FUCK_WITH_ME_BOT_COMMAND_HPP

#include <vector>
#include <string>

namespace vk::event {
class message_new;
} // namespace vk::event

namespace bot {

class command {
public:
  virtual ~command() = default;

  virtual std::string_view trigger() const = 0;
  virtual void run(
    const vk::event::message_new& event,
    const std::vector<std::string_view>& args
  ) = 0;
};

} // namespace bot

#endif // NOBODY_CAN_FUCK_WITH_ME_BOT_COMMAND_HPP