#include "utility/file.hpp"
#include "utility/tcp.h"
#include "utility/raii.hpp"
#include "utility/encryption.h"
#include <algorithm>

using std::experimental::string_view;
using namespace std;

void cut_string (std::vector<string_view>& result, string_view origin, string_view delim)
{
    result.clear();
    auto it = origin.begin ();

    while (it != origin.end ())
    {
        auto pos = search (it, origin.end (), delim.begin (), delim.end ());

        if (pos != it)
        {
            result.emplace_back (it, pos - it);
            if (pos == origin.end ())
            {
                break;
            }
        }

        it = (pos + static_cast<signed> (delim.length ()));
    }
}


int main ()
{
    auto conn = "www.baidu.com"_tcp[80];

    if (!conn)
    {
        std::cout << "cannot connect";
        return 0;
    }

    conn.writen ("GET / HTTP/1.0\r\n\r\n");

    std::string header;
    header.resize (8192);
    auto len = conn.read_until(header, "\r\n\r\n");
    if (len <= 0)
    {
        std::cout << "read until error";
        return 0;
    }
    auto u_len = static_cast<unsigned> (len);

    header.resize (u_len);

    std::vector<string_view> vec_str;
    cut_string (vec_str, header, "\r\n");

    int i = 0;
    for (auto & it : vec_str)
    {
        i ++;
        std::cout << "line:" << i << " " << it << '\n';
    }
    if (!vec_str.empty ())
    {
        vec_str.erase(vec_str.begin ());
    }

    std::cout << '\n' << '\n';

    std::map<std::string, std::string> header_kv;

    for (auto & view :vec_str)
    {
        std::vector<string_view> kv;
        cut_string (kv, view, ": ");
        if (kv.size () == 2)
        {
            header_kv [kv.at(0).to_string ()] = kv.at(1).to_string ();
        }
    }

    for (auto & it : header_kv)
    {
        std::cout << "key:" << it.first << " value:" << it.second << '\n';
    }

    auto content_length = header_kv.find ("Content-Length");
    if (content_length == header_kv.end ())
    {
        std::cout << "no content length";
        return 0;
    }

    auto length = std::stol (content_length->second);

    std::cout << "Content-Length is " << length << '\n';
    if (length < 0)
    {
        std::cout << "length < 0\n";
        return 0;
    }

    auto u_length = static_cast<unsigned> (length);

    std::string body;
    body.resize(u_length);

    conn.readn(body);

    ::write_buffer ("baidu.html", body);

    return 0;
}



