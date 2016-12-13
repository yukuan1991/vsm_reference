#include "tcp.h"
#include "conn_socket.hpp"
#include <regex>
#include <boost/lexical_cast.hpp>

using namespace std::string_literals;
using namespace nlohmann;

static std::string http_response ( conn_socket& sock);
static std::string read_chunked (conn_socket& sock);

conn_socket tcp_open(const char *host, uint16_t port)
{
    auto sock = conn_socket::make_socket (AF_INET, SOCK_STREAM, 0);

    if (sock.connect (host, port))
    {
        return sock;
    }
    else
    {
        return {};
    }
}

bool write_string(conn_socket &sock, std::experimental::fundamentals_v1::string_view view)
{
    if (sock.writen (htonl (view.length ())) != sizeof (decltype (htonl (int{}))))
    {
        return false;
    }

    if (sock.writen (view) != view.length ())
    {
        return false;
    }

    return true;
}

std::string read_string(conn_socket &sock, uint32_t max)
{
    uint32_t len;
    if (sock.readn (len) != sizeof len)
    {
        return {};
    }

    len = ntohl (len);

    if (len > max)
    {
        return {};
    }

    std::string ret;
    ret.resize (len);

    if (sock.readn (ret) != static_cast<int>(len))
    {
        return {};
    }
    else
    {
        return ret;
    }
}

std::string http_get (const char* host, not_null<const char*> path, const std::map<std::string, std::string>& params, uint16_t port) try
{
    auto sock = conn_socket::make_socket ();

    if (!sock.connect (host, port))
    {
        return {};
    }

    std::string str_get = "GET ";
    if (path[0] != '/')
    {
        str_get += "/";
    }
    str_get += path;
    str_get += "?";

    if (path)
    for (auto & it : params)
    {
        str_get += (it.first + "=" + it.second + "&");
    }

    str_get.pop_back ();

    auto str_header = str_get + " HTTP/1.1\r\n" "Host: " + host + "\r\n" "\r\n";

    sock.writen (str_header);

    return http_response (sock);
}
catch (...)
{
    return {};
}

json json_http_get (const char *host, not_null<const char*> path, const std::map<std::string, std::string> &params, uint16_t port) try
{
    return json::parse (http_get (host, path, params, port));
}
catch (...)
{
    return {};
}


std::string http_response (conn_socket& sock)
{
    unsigned long body_len;
    std::string str_http_body;

    std::string str_header;
    str_header.resize (8192);

    auto header_len = sock.read_until (str_header, "\r\n\r\n");
    if (header_len == -1)
    {
        return {};
    }

    str_header.resize (static_cast<unsigned>(header_len));

    do
    {
        std::regex expression("\r\nContent-Length:[[:space:]]+([[:digit:]]+)");
        std::smatch hit;
        if (std::regex_search (str_header, hit, expression))
        {
            try
            {
                body_len = boost::lexical_cast<unsigned long> (hit[1].str());
            }
            catch (...)
            {
                errno = EIO;
                return "";
            }
        }
        else
        {
            break;
        }

        str_http_body.resize (body_len);

        unsigned long body_rcv = sock.readn (str_http_body);

        if (body_rcv != body_len)
        {
            errno = EIO;
            return {};
        }

        return str_http_body;
    }
    while (0);

    std::regex expression("\r\nTransfer-Encoding:[[:space:]]+chunked");
    std::smatch hit;
    if (!std::regex_search (str_header, hit, expression))
    {
        errno = EIO;
        return {};
    }

    return read_chunked (sock);
}

constexpr uint32_t max_chunk_len = 1024 * 1024;
constexpr uint32_t max_total_len = 10 * 1024 * 1024;
std::string read_chunked (conn_socket& sock)
{
    int read_len;

    std::string str_chunk_len;

    uint32_t n_chunk_len;

    std::string str_chunk_content;

    std::vector<std::string> list_chunk;
    uint32_t n_total_len = 0;

    while (1)
    {
        str_chunk_len.resize (128);
        read_len = sock.read_until (str_chunk_len, "\r\n");

        if (read_len <= 2)
        {
            return {};
        }
        str_chunk_len.resize (static_cast<unsigned>(read_len) - 2);
        sscanf(str_chunk_len.data(), "%x", &n_chunk_len);

        if (n_chunk_len > max_chunk_len)
        {
            return {};
        }
        else if (n_chunk_len == 0)
        {
            break;
        }

        str_chunk_content.resize (n_chunk_len + 2);

        read_len = sock.readn (str_chunk_content);

        if (read_len != static_cast<int>(n_chunk_len + 2))
        {
            return {};
        }

        str_chunk_content.resize (n_chunk_len);

        list_chunk.emplace_back (move (str_chunk_content));

        if ((n_total_len += n_chunk_len) >  max_total_len)
        {
            return {};
        }
    }

    std::string str_output;
    str_output.reserve (n_total_len);

    for (auto it : list_chunk)
    {
        str_output += it;
    }

    return str_output;
}


json json_http_post(const char *host, not_null<const char*> path, const json &data, uint16_t port) try
{
    auto sock = conn_socket::make_socket ();

    if (!sock.connect (host, port))
    {
        return {};
    }

    auto text = data.dump (4);

    std::stringstream ss;
    ss << "POST " << path << " HTTP/1.1\r\n";
    ss << "Host: " << host << "\r\n";
    ss << "Content-Length:" << text.length () << "\r\n";


    ss << "\r\n";
    ss << text;


    sock.writen (ss.str ());

    auto str =  http_response (sock);
    return json::parse (str);

}
catch (...)
{
    return {};
}


