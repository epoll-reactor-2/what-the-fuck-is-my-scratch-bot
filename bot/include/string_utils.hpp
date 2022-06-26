#ifndef NOBODY_CAN_FUCK_WITH_ME_BOT_UTILS_STRING_UTILS_HPP
#define NOBODY_CAN_FUCK_WITH_ME_BOT_UTILS_STRING_UTILS_HPP

#include <string>
#include <vector>

namespace bot::string_utils {

std::string cut_first(std::string_view str);
std::string get_first(std::string_view str);

std::vector<std::pair<std::string_view, std::string_view>> args_resolve(
  const std::vector<std::string_view>& args
);

std::string thread_local_filename(
    std::string_view identifier,
    std::string_view extension = ""
);

}// namespace bot::string_utils

#endif // NOBODY_CAN_FUCK_WITH_ME_BOT_UTILS_STRING_UTILS_HPP