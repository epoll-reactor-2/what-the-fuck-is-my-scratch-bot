#include "method_wrappers.hpp"

#include "cpp_vk_lib/runtime/net/network.hpp"
#include "cpp_vk_lib/vk/methods/basic.hpp"
#include "cpp_vk_lib/vk/methods/constructor.hpp"
#include "cpp_vk_lib/vk/error/exception.hpp"

#include "simdjson.h"

std::string bot::method_wrappers::get_messages_upload_server(int64_t peer_id) {
  return vk::method::group_constructor()
    .method("photos.getMessagesUploadServer")
    .param("peer_id", std::to_string(peer_id))
    .perform_request();
}

vk::attachment::attachment_ptr_t bot::method_wrappers::save_messages_photo(
    std::string_view filename,
    std::string_view upload_server
) {
  namespace net = runtime::network;
  simdjson::dom::parser parser;
  const std::string server = parser.parse(upload_server)["response"]["upload_url"].get_c_str().take_value();
  const auto response
    = net::upload(server, "file", "application/octet-stream", filename, net::data_flow::require);
  const simdjson::dom::object upload_response = parser.parse(response);
  if (upload_response["photo"].get_string().take_value() == "[]" ||
      upload_response["photo"].get_string().take_value().empty()) {
    throw std::runtime_error("Не удалось загрузить фото.");
  }
  auto get_args = [](simdjson::dom::object upload_response) -> std::map<std::string, std::string> {
    return {
        {"photo",  upload_response["photo"].get_c_str().take_value()},
        {"hash",   upload_response["hash"].get_c_str().take_value()},
        {"server", std::to_string(upload_response["server"].get_int64())}
    };
  };
  const std::string saved_photo_json = vk::method::group_constructor()
    .method("photos.saveMessagesPhoto")
    .append_map(get_args(upload_response))
    .perform_request();
  const simdjson::dom::object uploaded = parser.parse(saved_photo_json)["response"].at(0);
  return std::make_unique<vk::attachment::photo>(
      uploaded["owner_id"].get_int64(),
      uploaded["id"].get_int64()
  );
}

void bot::method_wrappers::send_image(size_t peer_id, std::string_view path, std::string_view text) {
  const std::string upload_server = method_wrappers::get_messages_upload_server(peer_id);
  auto photo = save_messages_photo(path, upload_server);
  vk::method::messages::send(peer_id, text, std::move(photo));
}