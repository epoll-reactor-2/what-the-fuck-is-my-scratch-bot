#ifndef EXCUSE_ALL_THE_BLOOD_BOT_MESSAGE_HANDLER_HPP
#define EXCUSE_ALL_THE_BLOOD_BOT_MESSAGE_HANDLER_HPP

#include "command.hpp"

#include "cpp_vk_lib/vk/events/message_new.hpp"

#include <map>
#include <list>
#include <functional>


namespace bot {

class message_handler {
public:
  ~message_handler();

  void process(const vk::event::message_new&) const;

  void install_command(command* command);

private:
  std::map<std::string, command*> commands_;
};

}// namespace bot

#endif // EXCUSE_ALL_THE_BLOOD_BOT_MESSAGE_HANDLER_HPP