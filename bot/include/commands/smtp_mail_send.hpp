// Гугл я ебал твой рот.
#ifndef EXCUSE_ALL_THE_BLOOD_BOT_SMTP_MAIL_SEND_HPP
#define EXCUSE_ALL_THE_BLOOD_BOT_SMTP_MAIL_SEND_HPP

#include "command.hpp"
#include "string_utils.hpp"

#include "cpp_vk_lib/runtime/net/network.hpp"
#include "cpp_vk_lib/runtime/string_utils/implementation/join.hpp"
#include "cpp_vk_lib/vk/events/message_new.hpp"

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <sys/stat.h>

typedef struct {
  char         *message;
  char         *subject;
  char         *to;
  char         *from;
  char         *sender;
  char         *password;
  char        **filenames;
  const char   *host;
  size_t        bytes_read;
  size_t        total_read;
  size_t        verbose;
} smtp_ctx_t;

void smtp_ctx_init(smtp_ctx_t *ctx) {
  ctx->message    = NULL;
  ctx->subject    = NULL;
  ctx->to         = NULL;
  ctx->from       = NULL;
  ctx->sender     = NULL;
  ctx->password   = NULL;
  ctx->filenames  = NULL;
  ctx->host       = "smtps://smtp.gmail.com:465";
  ctx->bytes_read = 0;
  ctx->total_read = 0;
  ctx->verbose    = 0;
}

void smtp_ctx_destroy(smtp_ctx_t *ctx) {
  char **filenames = ctx->filenames;
  while (filenames && *filenames)
    free(*filenames++);
  free(ctx->filenames);
}

void send_mail(smtp_ctx_t *ctx) {
  CURL *curl_handle;
  CURLcode curl_res;
  char header[1024];
  char **filenames;
  struct curl_slist *recipients = NULL;
  struct curl_slist *headers = NULL;
  curl_mime *mime;
  curl_mimepart *part;

  curl_handle = curl_easy_init();
  if (!curl_handle) {
    printf("curl_easy_init() faied");
    return;
  }

  sprintf(header,
    "To:      <%s>\r\n"
    "From:    <%s>"
    "         (%s)\r\n"
    "Subject: %s\r\n"
    "Content-Type: multipart/mixed\r\n",
    ctx->to, ctx->from, ctx->sender, ctx->subject
  );

  /// Add file attachments.
  mime = curl_mime_init(curl_handle);
  filenames = ctx->filenames;

  while (filenames && *filenames) {
    part = curl_mime_addpart(mime);
    curl_mime_filedata(part, *filenames++);
    curl_mime_name(part, "data");
  }

  part = curl_mime_addpart(mime);
  curl_mime_data(part, ctx->message, CURL_ZERO_TERMINATED);
  curl_easy_setopt(curl_handle, CURLOPT_MIMEPOST, mime);

  /// Add text headers.
  headers = curl_slist_append(headers, header);
  recipients = curl_slist_append(recipients, ctx->to);
  curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);

  curl_easy_setopt(curl_handle, CURLOPT_USERNAME, ctx->from);
  curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, ctx->password);
  curl_easy_setopt(curl_handle, CURLOPT_URL, ctx->host);
  curl_easy_setopt(curl_handle, CURLOPT_MAIL_FROM, ctx->from);
  curl_easy_setopt(curl_handle, CURLOPT_MAIL_RCPT, recipients);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "smtp-curl-agent/1.0");
  curl_easy_setopt(curl_handle, CURLOPT_UPLOAD, 1L);
  curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, ctx->verbose);
  curl_res = curl_easy_perform(curl_handle);
  if(curl_res != CURLE_OK)
    fprintf(stderr,
      "curl_easy_perform() failed: %s\n",
      curl_easy_strerror(curl_res)
    );
  curl_slist_free_all(recipients);
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl_handle);
  if (mime)
    curl_mime_free(mime);
}

/// From Stackoverflow.
char** string_split(char* a_str, const char a_delim) {
  char** result;
  size_t count = 0;
  char* tmp = a_str;
  char* last_comma = 0;
  char delim[2];
  delim[0] = a_delim;
  delim[1] = 0;

  while (*tmp) {
    if (a_delim == *tmp) {
      count++;
      last_comma = tmp;
    }
    tmp++;
  }

  count += last_comma < (a_str + strlen(a_str) - 1);
  count++;

  result = (char **)malloc(count * sizeof(char *));
  if (!result) {
    perror("malloc");
    exit(-1);
  }

  if (result) {
    size_t idx  = 0;
    char* token = strtok(a_str, delim);

    while (token) {
      *(result + idx++) = strdup(token);
      token = strtok(0, delim);
    }
    *(result + idx) = 0;
  }

  return result;
}

void parse_command_line_args(
  smtp_ctx_t *ctx,
  const std::vector<std::string_view> &args
) {
  for (size_t i = 0; i < args.size() - 1; ++i) {
    char *ptr = const_cast<char *>(args[i].data());

    if (args[i] == "password")
      ctx->password = ptr;
    if (args[i] == "from")
      ctx->from = ptr;
    if (args[i] == "to")
      ctx->to = ptr;
    if (args[i] == "subject")
      ctx->subject = ptr;
    if (args[i] == "text")
      ctx->message = ptr;
    if (args[i] == "sender")
      ctx->sender = ptr;
  }
}

void try_parse_files(smtp_ctx_t *ctx, const vk::event::message_new &event) {
  std::vector<std::string> saved_files;

  for (const auto &attachment : event.attachments()) {
    const auto *document = dynamic_cast<vk::attachment::document *>(attachment.get());
    if (!document)
      continue;
    std::vector<uint8_t> buffer;
    if (!runtime::network::download(document->raw_url(), document->value()))
      throw std::runtime_error("Ошибка при скачивании файла.");
  }

  if (saved_files.empty())
    return;

  ctx->filenames = string_split(
    runtime::string_utils::join(saved_files, ',').data(),
    ','
  );
}

namespace bot {

class smtp_mail_send_command : public command {
  virtual std::string_view trigger() const override {
    return "/почта";
  }
  virtual void run(
    const vk::event::message_new &event,
    const std::vector<std::string_view> &args
  ) override {
    smtp_ctx_t ctx;
    smtp_ctx_init(&ctx);
    parse_command_line_args(&ctx, args);
    try_parse_files(&ctx, event);
    send_mail(&ctx);
    smtp_ctx_destroy(&ctx);
  }
};

} // namespace bot

#endif // EXCUSE_ALL_THE_BLOOD_BOT_SMTP_MAIL_SEND_HPP
