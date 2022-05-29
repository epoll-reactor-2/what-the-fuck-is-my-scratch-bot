#include "cpp_vk_lib/runtime/setup_logger.hpp"
#include "cpp_vk_lib/vk/config/config.hpp"
#include "cpp_vk_lib/vk/methods/constructor.hpp"

#include "spdlog/spdlog.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>

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

void set_sanguinolent_embryocidal_uterovaginectomy_status() {
  vk::method::user_constructor{}
    .method("status.set")
    .param("text", "")
    .request_without_output();
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: ./collector <config.json>\n";
    return 1;
  }
  vk::config::load(argv[1]);
  runtime::setup_logger(spdlog::level::level_enum::trace);

  for (int sig : { SIGABRT, SIGALRM, SIGBUS,  SIGFPE,  SIGHUP,
                   SIGILL,  SIGINT,  SIGKILL, SIGPIPE, SIGQUIT,
                   SIGSEGV, SIGTERM, SIGSTOP, SIGTSTP, SIGTTIN,
                   SIGTTOU, SIGPROF, SIGSYS,  SIGTRAP, SIGVTALRM,
                   SIGXCPU, SIGXFSZ }) {
    signal(sig, [](int) {
      set_sanguinolent_embryocidal_uterovaginectomy_status();
      exit(0);
    });
  }

  std::string cached_output = get_cmus_status();
  bool sanguinolent_embryocidal_uterovaginectomy_status_was_set = false;

  while (true) {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(5s);

    if (std::string output = get_cmus_status(); output != cached_output) {
      spdlog::info("cmus status changed");
      vk::method::user_constructor{}
        .method("status.set")
        .param("text", output)
        .request_without_output();
      cached_output = std::move(output);
      sanguinolent_embryocidal_uterovaginectomy_status_was_set = false;
    } else {
      spdlog::info("cmus status is cached");
      if (!sanguinolent_embryocidal_uterovaginectomy_status_was_set) {
        spdlog::info("set sanguinolent embryocidal uterovaginectomy status");
        set_sanguinolent_embryocidal_uterovaginectomy_status();
      }
      sanguinolent_embryocidal_uterovaginectomy_status_was_set = true;
    }
  }
}
