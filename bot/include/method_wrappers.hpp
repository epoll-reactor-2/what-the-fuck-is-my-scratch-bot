#ifndef WHAT_THE_FUCK_IS_MY_SCRATCH_BOT_METHOD_WRAPPERS_HPP
#define WHAT_THE_FUCK_IS_MY_SCRATCH_BOT_METHOD_WRAPPERS_HPP

#include "cpp_vk_lib/vk/attachment/attachment.hpp"

#include <string>
#include <vector>

namespace bot::method_wrappers {

std::string get_messages_upload_server(int64_t peer_id);

vk::attachment::attachment_ptr_t save_messages_photo(std::string_view filename, std::string_view upload_server);

void send_image(size_t peer_id, std::string_view path, std::string_view text);

} // namespace bot::method_wrappers

#endif // WHAT_THE_FUCK_IS_MY_SCRATCH_BOT_METHOD_WRAPPERS_HPP
