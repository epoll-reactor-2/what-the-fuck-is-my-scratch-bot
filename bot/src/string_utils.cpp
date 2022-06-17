#include "string_utils.hpp"

#include "cpp_vk_lib/runtime/string_utils/implementation/split.hpp"

#include <stdexcept>
#include <thread>

std::string bot::string_utils::cut_first(std::string_view str) {
  const size_t curr = str.find_first_of(" \f\n\r\t\v");
  return (curr == std::string::npos) ? "" : std::string(str.substr(curr + 1));
}

std::string bot::string_utils::get_first(std::string_view str) {
  const size_t curr = str.find_first_of(" \f\n\r\t\v");
  return (curr == std::string::npos) ? std::string(str) : std::string(str.substr(0, curr));
}

std::vector<std::pair<std::string_view, std::string_view>> bot::string_utils::args_resolve(
  const std::vector<std::string_view>& args
) {
  std::vector<std::pair<std::string_view, std::string_view>> resolved;
  resolved.reserve(args.size());
  using namespace runtime::string_utils;
  for (std::string_view arg : args) {
    auto record = split(arg, '=');
    if (record.size() != 2)
      throw std::runtime_error("Недопустимые аргументы.");
    resolved.emplace_back(record[0], record[1]);
  }
  return resolved;
}

#include <thread>


std::string bot::string_utils::thread_local_filename(
  std::string_view identifier,
  std::string_view extension
) {
  std::string filename(identifier);
  filename += '_';
  filename += std::to_string(
      std::hash<std::thread::id>{}(std::this_thread::get_id())
  );
  return extension.empty() ? filename : filename + "." + extension.data();
}