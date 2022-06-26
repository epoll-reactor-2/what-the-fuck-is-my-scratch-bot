#ifndef NOBODY_CAN_FUCK_WITH_ME_BOT_BACKGROUND_COMMAND_HPP
#define NOBODY_CAN_FUCK_WITH_ME_BOT_BACKGROUND_COMMAND_HPP

namespace bot {

class background_command {
public:
  virtual ~background_command() = default;

  virtual void run() = 0;
};

} // namespace bot

#endif // NOBODY_CAN_FUCK_WITH_ME_BOT_BACKGROUND_COMMAND_HPP