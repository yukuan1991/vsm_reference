#pragma once

#include "utility/conn_socket.hpp"
#include "json.hpp"
#include <wininet.h>
#include "utility/file.hpp"
constexpr int max_longlong_buffer = 32;

conn_socket tcp_open (const char* ip, uint16_t port);
bool write_string (conn_socket& sock, std::experimental::string_view view);
std::string read_string (conn_socket& sock, uint32_t max = 1024 * 1024 * 16);

std::string http_get (const char* host, not_null<const char*> path, const std::map<std::string, std::string>& params = {}, uint16_t port = 80);
nlohmann::json json_http_get (const char* host, not_null<const char*> path, const std::map<std::string, std::string>& params = {}, uint16_t port = 80);
nlohmann::json json_http_post (const char* host, not_null<const char*> path, const nlohmann::json& data, uint16_t port = 80);

// template download functions with callbacks
struct close_wininet_deleter { void operator () (void* p) { ::InternetCloseHandle (p); } };
using inet_session_ptr = std::unique_ptr<void, close_wininet_deleter>;

template<typename RES_CB>
int http_dl (const char* url, const char* save_path, RES_CB res)
{
    unsigned long long ul_current_byte = 0; //当前下载进度
    unsigned long long ul_total_byte = 0; //总字节数

    std::array<char, max_longlong_buffer> arr_buffer_size;
    unsigned long int buffer_length = arr_buffer_size.size ();
    std::string net_agent = "RookID/1.0"; //http头部的客户端标识符

    std::string tmp_block;
    tmp_block.resize (4096);

    inet_session_ptr handle_session {InternetOpenA (net_agent.data (), INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0)};

    if (handle_session == nullptr)
    {
        return -1;
    }

    inet_session_ptr handle_url {::InternetOpenUrlA (handle_session.get(), url, nullptr, 0, INTERNET_FLAG_DONT_CACHE, 0) };

    if (handle_url == nullptr)
    {
        return -1;
    }

    bool query_success = ::HttpQueryInfoA(handle_url.get (), HTTP_QUERY_CONTENT_LENGTH, arr_buffer_size.data (), &buffer_length, nullptr);

    if (!query_success)
    {
        return -1;
    }

    ul_total_byte = std::stoull (arr_buffer_size.data ());

    res (0, ul_total_byte);

    auto fp = file::fopen (save_path, "wb");
    if (!fp)
    {
        return -1;
    }
    fp.fseek (file_pos::head);


    long unsigned int read_len;
    ul_current_byte = 0;

    do
    {
        ::InternetReadFile (handle_url.get (), const_cast<char*> (tmp_block.data ()), tmp_block.length (), &read_len);

        ul_current_byte += read_len;
        if (read_len > tmp_block.length ())
        {
            return -1;
        }

        if (read_len > 0)
        {
            std::experimental::string_view view = tmp_block;
            fp.fwrite (view.substr (0, read_len));
        }

        res (ul_current_byte, ul_total_byte);

    }while (read_len > 0);

    if (ul_current_byte != ul_total_byte)
    {
        return -1;
    }

    return 0;
}
