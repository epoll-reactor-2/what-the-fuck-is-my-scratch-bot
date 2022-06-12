#include "string_utils.hpp"

std::string bot::string_utils::cut_first(std::string_view str) {
  const size_t curr = str.find_first_of(" \f\n\r\t\v");
  return (curr == std::string::npos) ? "" : std::string(str.substr(curr + 1));
}

std::string bot::string_utils::get_first(std::string_view str) {
  const size_t curr = str.find_first_of(" \f\n\r\t\v");
  return (curr == std::string::npos) ? std::string(str) : std::string(str.substr(0, curr));
}