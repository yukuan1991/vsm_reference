#pragma once

#include <stdio.h>
#include <string>
#include <utility>
#include <experimental/string_view>
#include <boost/locale.hpp>
#include <memory>
#include "utility/interface.hpp"
#include "utility/array_view.hpp"
#include <experimental/optional>
#include "utility/raii.hpp"

enum class file_pos : int
{
    head = SEEK_SET,
    tail = SEEK_END,
    current = SEEK_CUR,
};

/// @note 使用 RAII 封装的文件指针类
class file final
{
    template<typename T>
    using basic_string_view = std::experimental::basic_string_view<T>;
    using string_view = std::experimental::string_view;
    using string = std::string;
public:
    /// 打开文件
    /// @param para file_name 要打开的文件名
    /// @param mode 打开模式 r -> 读 w -> 写 a -> 追加 b -> 二进制
    template<typename ... ARGS>
    static file fopen (const char* file_name, const char* mode) { return ::fopen (file_name, mode);}
    /// 偏移至文件某处
    int fseek (file_pos origin, long offset = 0)
    {
        return ::fseek (fp_.get (), offset, static_cast<int> (origin));
    }

    long ftell ()
    {
        return ::ftell (fp_.get ());
    }


    file () = default;

    /// 获取内部文件指针
    FILE* get () const noexcept { return fp_.get ();}

    /// 返回内部文件指针并且释放控制
    FILE* release () noexcept { return fp_.release ();}


    /// 写入文件 返回写入长度
    auto fwrite (string_view view) noexcept
    {
        return ::fwrite (view.data (), sizeof (char), view.length (), fp_.get ());
    }

    /// 读取文件中信息
    auto fread (array_view<char> view)
    {
        return ::fread (view.data (), sizeof (char), view.length (), fp_.get ());
    }

    /// 格式化写入文件
    template<typename ... ARGS>
    auto fprintf (const char* fmt, ARGS&& ... args) { return ::fprintf (fp_, fmt, std::forward<decltype(args)> (args)...);}

    explicit operator bool () { return fp_ != nullptr;}

private:
    file (FILE* fp) noexcept : fp_ (fp) {}
private:
    struct fclose_deleter { void operator () (FILE* fp) { fclose (fp);}};
    std::unique_ptr<FILE, fclose_deleter> fp_ = nullptr;
};

inline std::string make_sized_string (typename std::string::size_type s)
{
    std::string str;
    str.resize (s);
    return str;
}

inline file operator "" _r (const char* path, std::size_t)
{
    return file::fopen (path, "rb");
}

inline file operator "" _w (const char* path, std::size_t)
{
    return file::fopen (path, "wb");
}

inline file operator "" _a (const char* path, std::size_t)
{
    return file::fopen (path, "ab+");
}


/// 读取一个文件中所有的内容
inline std::experimental::optional<std::string> read_all (const char* path)
{
    using std::experimental::nullopt;
    auto fp = file::fopen (path, "rb");

    if (!fp)
    {
        return nullopt;
    }

    if (fp.fseek (file_pos::tail))
    {
        return nullopt;
    }
    int length = ftell (fp.get());

    if (length < 0)
    {
        return nullopt;
    }

    if (fp.fseek (file_pos::head) != 0)
    {
        return nullopt;
    }

    auto buffer = std::string (static_cast<size_t> (length), '\0');

    if (fp.fread (buffer) != buffer.length ())
    {
        return nullopt;
    }

    return std::move (buffer);
}


/// 把一个字符串或者二进制数据写入文件成功返回true, 失败返回false
namespace file_opt
<%
constexpr struct {} truncate {};
constexpr struct {} append {};
%>
template <typename opt_type = decltype (file_opt::truncate)>
inline bool write_buffer (const char* path, std::experimental::string_view buff, opt_type opt = file_opt::truncate)
{
    MAYBE_UNUSED (opt);
    static_assert (std::is_same<std::decay_t<opt_type>, std::decay_t<decltype (file_opt::append)>>::value
                   or std::is_same<std::decay_t<opt_type>, std::decay_t<decltype (file_opt::truncate)>>::value,
                   "opt_type can either be truncate or append");

    auto fp = file::fopen (path,
                           std::is_same<std::decay_t<opt_type>,
                                        std::decay_t<decltype (file_opt::truncate)>>::value ?
                               "wb" : "a+");

    if (!fp)
    {
        return false;
    }
    return buff.size () == fp.fwrite (buff);
}

/// 从UTF-8转换成为系统编码
inline std::string utf_to_sys (std::experimental::string_view from)
{
    return boost::locale::conv::from_utf (from.begin (), from.end (), "");
}

/// 从系统编码转成UTF-8
inline std::string sys_to_utf (std::experimental::string_view from)
{
    return boost::locale::conv::to_utf<char> (from.begin (), from.end (), "");
}








































