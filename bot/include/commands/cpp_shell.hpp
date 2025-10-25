#ifndef WHAT_THE_FUCK_IS_MY_SCRATCH_BOT_CPP_SHELL_HPP
#define WHAT_THE_FUCK_IS_MY_SCRATCH_BOT_CPP_SHELL_HPP

#include "../command.hpp"

#include "cpp_vk_lib/runtime/string_utils/implementation/join.hpp"
#include "cpp_vk_lib/vk/events/common.hpp"
#include "cpp_vk_lib/vk/events/message_new.hpp"
#include "cpp_vk_lib/vk/methods/basic.hpp"

#include <vector>
#include <array>
#include <memory>
#include <fstream>

namespace bot {

class cpp_shell_command : public command {
  std::string os_exec(std::string_view cmd) {
    std::string result;
    std::array<char, 128> buffer;

    struct pclose_operator {
        void operator()(FILE *file) {
            pclose(file);
        }
    };

    std::unique_ptr<FILE, pclose_operator> pipe(popen(cmd.data(), "r"), pclose_operator{});
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
      result += buffer.data();
    }
    return result;
  }
  std::string_view trigger() const override {
    return "/cpp_shell";
  }
  void run(
    const vk::event::message_new& event,
    const std::vector<std::string_view>& args
  ) override {
    if (event.from_id() != 499047616) {
      vk::method::messages::send(event.peer_id(), "Отказано в доступе.");
      return;
    }
    using namespace runtime::string_utils;
    vk::method::messages::send(
        event.peer_id(),
        [&]() -> std::string {
          std::ofstream("/tmp/script.sh")
              << "echo -e \""
                 "#include <bits/stdc++.h>\n"
                 "\n"
                 "int main() { $1 }\n"
                 "\" > /tmp/exec.cpp\n"
                 "clang++ /tmp/exec.cpp -o /tmp/compiled_exec && /tmp/compiled_exec";
          return os_exec("/tmp/script.sh \'" + join(args, ' ') + "\'");
        }()
    );
  }
};

} // namespace bot

#endif // WHAT_THE_FUCK_IS_MY_SCRATCH_BOT_CPP_SHELL_HPP
