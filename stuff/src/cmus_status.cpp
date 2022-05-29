#include "cpp_vk_lib/runtime/setup_logger.hpp"
#include "cpp_vk_lib/vk/config/config.hpp"
#include "cpp_vk_lib/vk/methods/constructor.hpp"

#include "spdlog/spdlog.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>

std::string get_cmus_info() {
  static const char cmus_command[] = "bash -c /home/machen/code/Bash/cmus_status.sh";
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

  std::string cached_output = get_cmus_info();
  bool sanguinolent_embryocidal_uterovaginectomy_status_was_set = false;

  while (true) {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(5s);

    if (std::string output = get_cmus_info(); output != cached_output) {
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
