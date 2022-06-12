#ifndef EXCUSE_ALL_THE_BLOOD_BOT_BACKGROUND_COMMAND_HPP
#define EXCUSE_ALL_THE_BLOOD_BOT_BACKGROUND_COMMAND_HPP

namespace bot {

class background_command {
public:
  virtual ~background_command() = default;

  virtual void run() = 0;
};

} // namespace bot

#endif // EXCUSE_ALL_THE_BLOOD_BOT_BACKGROUND_COMMAND_HPP