#ifndef WHAT_THE_FUCK_IS_MY_SCRATCH_BOT_BACKGROUND_COMMAND_HPP
#define WHAT_THE_FUCK_IS_MY_SCRATCH_BOT_BACKGROUND_COMMAND_HPP

namespace bot {

class background_command {
public:
  virtual ~background_command() = default;

  virtual void run() = 0;
};

} // namespace bot

#endif // WHAT_THE_FUCK_IS_MY_SCRATCH_BOT_BACKGROUND_COMMAND_HPP