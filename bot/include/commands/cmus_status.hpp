#ifndef NOBODY_CAN_FUCK_WITH_ME_BOT_CMUS_STATUS_HPP
#define NOBODY_CAN_FUCK_WITH_ME_BOT_CMUS_STATUS_HPP

#include "../background_command.hpp"

#include "cpp_vk_lib/vk/events/message_new.hpp"
#include "cpp_vk_lib/vk/methods/basic.hpp"
#include "cpp_vk_lib/vk/methods/constructor.hpp"

#include "spdlog/spdlog.h"

#include <csignal>
#include <thread>
#include <chrono>
#include <array>

namespace bot {

class cmus_status_command : public background_command {
  std::string get_cmus_status() {
    static const char cmus_command[] =
        "#!/bin/sh\n"
        ""
        "if info=$(cmus-remote -Q 2> /dev/null); then\n"
        "  status=$(echo \"$info\" | grep -v \"set \" | grep -v \"tag \" | grep \"status \" | cut -d ' ' -f 2)\n"
        ""
        "  if [ \"$status\" = \"playing\" ] || [ \"$status\" = \"paused\" ] || [ \"$status\" = \"stopped\" ]; then\n"
        "    title=$(echo \"$info\" | grep -v 'set ' | grep \" title \" | cut -d ' ' -f 3-)\n"
        "    artist=$(echo \"$info\" | grep -v 'set ' | grep \" artist \" | cut -d ' ' -f 3-)\n"
        "    position=$(echo \"$info\" | grep -v \"set \" | grep -v \"tag \" | grep \"position \" | cut -d ' ' -f 2)\n"
        "    duration=$(echo \"$info\" | grep -v \"set \" | grep -v \"tag \" | grep \"duration \" | cut -d ' ' -f 2)\n"
        ""
        "    if [[ \"$duration\" -ge 0 ]]; then\n"
        "      pos_minutes=$(printf \"%02d\" $((position / 60)))\n"
        "      pos_seconds=$(printf \"%02d\" $((position % 60)))\n"
        ""
        "      dur_minutes=$(printf \"%02d\" $((duration / 60)))\n"
        "      dur_seconds=$(printf \"%02d\" $((duration % 60)))\n"
        ""
        "      info_string=\"| $pos_minutes:$pos_seconds - $dur_minutes:$dur_seconds    \" \n"
        "    fi\n"
        ""
        "    info_string=\"$artist - $title $info_string\"\n"
        "                            \n"
        "    if [ \"$status\" = \"playing\" ]; then\n"
        "      echo \"▶️ $info_string\"\n"
        "    elif [ \"$status\" = \"paused\" ]; then\n"
        "      echo \"⏸️ $info_string\"\n"
        "    elif [ \"$status\" = \"stopped\" ]; then \n"
        "      echo \"⏹ $info_string\"\n"
        "    fi\n"
        "  fi\n"
        "fi";
    std::string result;
    std::array<char, 128> buffer;
    std::unique_ptr<FILE, decltype (&pclose)> pipe(popen(cmus_command, "r"), pclose);
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
      result += buffer.data();
    }
    return result;
  }

  virtual void run() override {
    std::string cached_output = get_cmus_status();
    bool status_was_set = false;

    while (true) {
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(5s);

      if (std::string output = get_cmus_status(); output != cached_output) {
        spdlog::info("cmus_status_command: cmus status changed");
        vk::method::user_constructor{}
            .method("status.set")
            .param("text", output)
            .request_without_output();
        cached_output = std::move(output);
        status_was_set = false;
      } else {
        spdlog::info("cmus_status_command: cmus status is cached");
        if (!status_was_set) {
          spdlog::info("cmus_status_command: set status");
          vk::method::user_constructor{}
              .method("status.set")
              .param("text", "")
              .request_without_output();
        }
        status_was_set = true;
      }
    }
  }
};

} // namespace bot

#endif // NOBODY_CAN_FUCK_WITH_ME_BOT_CMUS_STATUS_HPP
