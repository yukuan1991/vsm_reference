#pragma once

#include <string>
#include <experimental/string_view>
#include "utility/interface.hpp"


std::string binary_to_base64 (std::experimental::string_view input);
std::string base64_to_binary (std::experimental::string_view view);
std::string url_encode (std::experimental::string_view raw);

void krys3des_decryption (const std::string & input, const char* key24bytes, std::string & output);
void krys3des_encryption (const std::string & source, const char* key24bytes, std::string & output);

/// FILE MD5
std::string get_file_md5 (not_null<const char*> path);

std::string html_escape (std::experimental::string_view view);
