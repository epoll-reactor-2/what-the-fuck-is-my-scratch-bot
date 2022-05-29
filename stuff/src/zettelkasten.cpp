#include "cpp_vk_lib/runtime/setup_logger.hpp"
#include "cpp_vk_lib/runtime/string_utils/implementation/split.hpp"
#include "cpp_vk_lib/vk/config/config.hpp"
#include "cpp_vk_lib/vk/events/message_new.hpp"
#include "cpp_vk_lib/vk/long_poll/long_poll.hpp"
#include "cpp_vk_lib/vk/methods/basic.hpp"

#include <iostream>

std::string cut_first(std::string_view str) {
  const size_t curr = str.find_first_of(" \f\n\r\t\v");
  return (curr == std::string::npos) ? "" : std::string(str.substr(curr + 1));
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: ./zettelkasten <config.json>\n";
    return 1;
  }
  vk::config::load(argv[1]);
  runtime::setup_logger(spdlog::level::level_enum::debug);

  asio::io_context io_context;
  vk::long_poll long_poll(io_context);

  long_poll.on_event(vk::event::type::message_new, [](const vk::event::common& event) {
    auto message = event.get_message_new();
    auto text = message.text();
    if (text.size() <= 5 || text.find_first_of(":push") != 0)
      return;
    if (!message.has_reply() && !message.has_fwd_messages()) {
      vk::method::messages::send(message.peer_id(), "Ожидается реплай.");
      return;
    }
    auto handle_reply = [&] -> std::string {
      if (message.has_reply())
        return message.reply()->text();

      if (message.has_fwd_messages())
        return message.fwd_messages().front()->text();

      return "";
    };
    auto *fd = popen(
      (
        "#!/bin/sh\n"
        ""
        "pushd $HOME/git/zettelkasten\n"
        "mkdir `date -I`-" + cut_first(text) + "\n"
        "pushd `date -I`-" + cut_first(text) + "\n"
        "echo '" + handle_reply() + "' > Note.txt\n"
        "popd\n"
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
    vk::method::messages::send(message.peer_id(), output);
  });
  long_poll.run();
}