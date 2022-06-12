#ifndef EXCUSE_ALL_THE_BLOOD_BOT_ZETTELKASTEN_HPP
#define EXCUSE_ALL_THE_BLOOD_BOT_ZETTELKASTEN_HPP

#include "command.hpp"

#include "string_utils.hpp"

#include "cpp_vk_lib/runtime/setup_logger.hpp"
#include "cpp_vk_lib/runtime/string_utils/implementation/split.hpp"
#include "cpp_vk_lib/runtime/string_utils/implementation/join.hpp"
#include "cpp_vk_lib/vk/config/config.hpp"
#include "cpp_vk_lib/vk/events/message_new.hpp"
#include "cpp_vk_lib/vk/long_poll/long_poll.hpp"
#include "cpp_vk_lib/vk/methods/basic.hpp"

namespace bot {

class zettelkasten_command : public command {
  virtual std::string trigger() const override {
    return "/zettelkasten";
  }

  void run(
    const vk::event::message_new &event,
    const std::vector<std::string_view> &args
  ) override {
    if (args.empty()) {
      vk::method::messages::send(event.peer_id(), "Ожидается название темы.");
      return;
    }
    if (!event.has_reply() && !event.has_fwd_messages()) {
      vk::method::messages::send(event.peer_id(), "Ожидается реплай.");
      return;
    }
    auto handle_reply = [&]() -> std::string {
      if (event.has_reply())
        return event.reply()->text();

      if (event.has_fwd_messages())
        return event.fwd_messages().front()->text();

      return "";
    };
    using namespace runtime::string_utils;
    auto dir = join(args, ' ');
    std::replace(dir.begin(), dir.end(), ' ', '-');
    auto *fd = popen(
      (
        "#!/bin/sh\n"
        ""
        "pushd $HOME/git/zettelkasten\n"
        "mkdir `date -I`-" + dir + "\n"
        "pushd `date -I`-" + dir + "\n"
        "echo '" + handle_reply() + "' > Note.txt\n"
        "popd\n"
        "echo $PWD\n"
        "git add *\n"
        "git commit -m 'update'\n"
        "git push -f\n"
        "popd\n"
      ).c_str(),
      "r"
    );
    std::string output;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fd) != nullptr)
      output += buffer;
    pclose(fd);
    vk::method::messages::send(event.peer_id(), output);
  }
};

} // namespace bot

#endif // EXCUSE_ALL_THE_BLOOD_BOT_ZETTELKASTEN_HPP
