#ifndef EXCUSE_ALL_THE_BLOOD_BOT_UTILS_STRING_UTILS_HPP
#define EXCUSE_ALL_THE_BLOOD_BOT_UTILS_STRING_UTILS_HPP

#include <string>
#include <vector>

namespace bot::string_utils {

std::string cut_first(std::string_view str);
std::string get_first(std::string_view str);

std::vector<std::pair<std::string_view, std::string_view>> args_resolve(
  const std::vector<std::string_view>& args
);

}// namespace bot::string_utils

#endif // EXCUSE_ALL_THE_BLOOD_BOT_UTILS_STRING_UTILS_HPP