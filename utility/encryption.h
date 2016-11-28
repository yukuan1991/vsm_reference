#pragma once

#include <string>
#include <experimental/string_view>


std::string binary_to_base64 (std::experimental::string_view input);
std::string base64_to_binary (std::experimental::string_view view);


/// FILE MD5
std::string get_file_md5 (const char* path);

std::string html_escape (std::experimental::string_view view);
