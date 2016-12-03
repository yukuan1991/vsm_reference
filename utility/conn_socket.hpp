#pragma once

#include <utility>
#include <stdint.h>
#include <iostream>
#include "utility/array_view.hpp"
#include <experimental/string_view>
#include <winsock.h>
#include <algorithm>

class conn_socket final
{
public:
    using string_view = std::experimental::string_view;
    template<typename T>
    using basic_string_view = std::experimental::basic_string_view<T>;

    conn_socket () = default;
    static conn_socket make_socket (int af = AF_INET, int type = SOCK_STREAM, int proto = 0) noexcept
    {
        WSADATA wsadata;
        if(WSAStartup(MAKEWORD(1,1),&wsadata)!=SOCKET_ERROR)
        {
            return ::socket (af, type, proto);
        }
        return {};
    }

    static int analysis_addr (const char *_addrstr, uint32_t *_addr) noexcept
    {
        *_addr = ::inet_addr (_addrstr);
        if (*_addr != INADDR_NONE)
        {
            return 0;
        }
        struct hostent* hent;
        if ((hent = gethostbyname (_addrstr)) != NULL )
        {
            *_addr = ((struct in_addr *)(hent->h_addr_list [0]))->s_addr;
            return 0;
        }

        return -1;
    }

    bool bind (const char* ip, uint16_t port) noexcept
    {
        sockaddr_in addr_info;
        addr_info.sin_family = AF_INET;
        uint32_t addr32bit;
        if (analysis_addr (ip, &addr32bit) == -1)
        {
            return false;
        }
        addr_info.sin_addr.s_addr = addr32bit;
        addr_info.sin_port = ::htons (port);
        return (::bind (socket_, (sockaddr*)&addr_info, sizeof (addr_info)) == 0);
    }

    bool connect (const char* ip, uint16_t port) noexcept
    {
        sockaddr_in addr_info;
        addr_info.sin_family = AF_INET;

        uint32_t addr32bit;
        if (analysis_addr (ip, &addr32bit) == -1)
        {
            return false;
        }
        addr_info.sin_addr.s_addr = addr32bit;

        addr_info.sin_port = ::htons (port);
        return (::connect (socket_, (sockaddr*)&addr_info, sizeof (addr_info)) == 0);
    }

    int read (array_view<char> arr) noexcept
    {
        return ::recv (socket_, arr.data (), static_cast<int>(arr.length ()), 0);
    }

    int readn (array_view<char>::iterator begin, array_view<char>::iterator end, long timeout_sec = 0, long timeout_usec = 0) noexcept
    {
        return readn ({begin, static_cast<array_view<char>::size_type> (end - begin)}, timeout_sec, timeout_usec);
    }

    int readn (array_view<char> arr, long timeout_sec = 0, long timeout_usec = 0) noexcept
    {
        auto current_view = arr;
        fd_set read_fdset;

        timeval timeout;
        int select_return = 0;
        while (current_view.length () > 0)
        {
            if (timeout_sec != 0 or timeout_usec != 0)
            {
                FD_ZERO(&read_fdset);
                FD_SET(socket_, &read_fdset);
                timeout.tv_sec = timeout_sec; timeout.tv_usec = timeout_usec;

                do
                {
                    select_return = select(static_cast<int>(socket_ + 1), &read_fdset, nullptr, nullptr, &timeout);
                } while (select_return == -1 and errno == EINTR);

                if (select_return == 0)
                {
                    errno = ETIMEDOUT;
                    break;
                }
            }
            auto read_len = read (current_view);

            if (read_len == static_cast<int>(current_view.length ()))
            {
                return static_cast<int>(arr.length ());
            }

            if (read_len <= 0)
            {
                errno = EIO;
                break;
            }
            auto new_data = current_view.data () + read_len;
            auto new_len = current_view.length () - static_cast<unsigned>(read_len);
            current_view = array_view<char> (new_data, new_len);
        }

        return static_cast<int>(arr.length () - current_view.length ());
    }

    template<typename RES_CB, typename ERR_CB>
    int readn_cb (array_view<char> arr, long timeout_sec = 0, long timeout_usec = 0, RES_CB&& res_cb = {}, ERR_CB&& err_cb = {}) noexcept
    {
        auto current_view = arr;
        fd_set read_fdset;

        timeval timeout;
        int select_return = 0;
        while (current_view.length () > 0)
        {
            if (timeout_sec != 0 or timeout_usec != 0)
            {
                FD_ZERO(&read_fdset);
                FD_SET(socket_, &read_fdset);
                timeout.tv_sec = timeout_sec; timeout.tv_usec = timeout_usec;

                do
                {
                    select_return = select(static_cast<int>(socket_ + 1), &read_fdset, nullptr, nullptr, &timeout);
                } while (select_return == -1 and errno == EINTR);

                if (select_return == 0)
                {
                    errno = ETIMEDOUT;
                    break;
                }
            }
            auto read_len = read (current_view);

            if (read_len == static_cast<int>(current_view.length ()))
            {
                return static_cast<int>(arr.length ());
            }

            if (read_len <= 0)
            {
                errno = EIO;
                break;
            }
            auto new_data = current_view.data () + read_len;
            auto new_len = current_view.length () - static_cast<unsigned>(read_len);
            current_view = array_view<char> (new_data, new_len);
            res_cb (arr.length () - current_view.length (), arr.length ());
        }

        err_cb ();
        return static_cast<int>(arr.length () - current_view.length ());
    }

    template<typename INT, typename SFINAE = typename std::enable_if<std::is_integral<INT>::value , void>::type>
    uint32_t writen (const INT& data) noexcept
    {
        string_view view {reinterpret_cast<const char*>(&data), sizeof data};
        return writen (view);
    }

    unsigned writen (string_view view) noexcept
    {
        auto current_view = view;
        while (current_view.length () > 0)
        {
            auto write_len = write (current_view);
            if (write_len <= 0)
            {
                break;
            }
            auto new_data = current_view.data () + write_len;
            auto new_len = current_view.length () - static_cast<unsigned>(write_len);

            current_view = {new_data, new_len};
        }

        return view.length () - current_view.length ();
    }

    unsigned writen (string_view::iterator begin, string_view::iterator end) noexcept
    {
        return writen ({begin, static_cast<string_view::size_type> (end - begin)});
    }

    int write (string_view view) noexcept
    {
        return ::send (socket_, view.data (), static_cast<int>(view.length ()), 0);
    }

    int write (string_view::iterator begin, string_view::iterator end) noexcept
    {
        return ::send (socket_, begin, end - begin, 0);
    }

    int read_until (array_view<char> view, string_view until, long timeout_sec = 0, long timeout_usec = 0) noexcept
    {
        auto start = view.begin ();
        auto stop = view.end ();
        auto search_cur = start;

        while (start != stop)
        {
            auto len = recv_peek (start, stop, timeout_sec, timeout_usec);
            if (len <= 0)
            {
                return -1;
            }
            auto peek_end = start + len;
            auto search_ret = std::search (search_cur, peek_end, until.begin (), until.end ());

            if (search_ret != peek_end)
            {
                auto ret = readn (start, search_ret + until.length ());
                if (ret != search_ret - start + static_cast<int> (until.length ()))
                {
                    return -1;
                }
                else
                {
                    return start + ret - view.begin ();
                }
            }

            auto readn_len = readn (start, search_ret, timeout_sec, timeout_usec);
            if (readn_len != search_ret - start)
            {
                return -1;
            }

            if (readn_len >= static_cast<int> (until.length ()))
            {
                search_cur = search_ret - until.length () + 1;
            }
            start += readn_len;
        }

        return -1;
    }

    conn_socket (conn_socket&& that) noexcept {*this = std::move (that);}
    conn_socket (const conn_socket&) = delete;
    explicit operator bool () noexcept { return socket_ != INVALID_SOCKET;}
    void operator= (const conn_socket&) = delete;
    void operator= (conn_socket&& that) noexcept { socket_ = that.socket_; that.socket_ = INVALID_SOCKET;}
    ~conn_socket () { clear (); }
    void clear () noexcept { if (socket_ != INVALID_SOCKET) ::closesocket (socket_); socket_ = INVALID_SOCKET; }
private:

    int recv_peek (array_view<char>::iterator begin, array_view<char>::iterator end, long timeout_sec = 0, long timeout_usec = 0) noexcept
    {
        return recv_peek ({begin, static_cast<array_view<char>::size_type> (end - begin)}, timeout_sec, timeout_usec);
    }

    int recv_peek (array_view<char> view, long timeout_sec = 0, long timeout_usec = 0) noexcept
    {
        if (timeout_sec != 0 and timeout_usec != 0)
        {
            fd_set read_fdset;
            timeval timeout;

            FD_ZERO(&read_fdset);
            FD_SET(socket_, &read_fdset);
            timeout.tv_sec = timeout_sec; timeout.tv_usec = timeout_usec;

            if (select(static_cast<int>(socket_ + 1), &read_fdset, nullptr, nullptr, &timeout) < 1)
            {
                return -1;
            }
        }

        return recv (socket_, view.data (), static_cast<int>(view.length ()), MSG_PEEK);
    }

    constexpr conn_socket(SOCKET fd) noexcept : socket_ (fd) {}
private:
    SOCKET socket_ = INVALID_SOCKET;
};

struct tcp_helper
{
    conn_socket operator [] (unsigned short port)
    {
        auto sock = conn_socket::make_socket ();
        if (sock.connect (p_, port))
        {
            return sock;
        }
        else
        {
            return {};
        }
    }
    const char* p_ = nullptr;
};

inline tcp_helper operator "" _tcp (const char* host, std::size_t)
{
    tcp_helper h;
    h.p_ = host;
    return h;
}
