#include "utility/encryption.h"
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/phoenix/phoenix.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>
#include <assert.h>
#include <utility>
#include "md5.h"
#include "utility/file.hpp"
#include <algorithm>
#include "utility/raii.hpp"

using namespace boost::phoenix::arg_names;
using namespace boost::phoenix;
using namespace std;
using namespace std::experimental;
using namespace std::experimental::string_view_literals;

std::string binary_to_base64 (string_view input)
{
    using namespace boost::archive::iterators;
    using namespace std;
    using it_base64_t = base64_from_binary<transform_width<string::const_iterator, 6, 8>>;

    string result (it_base64_t (input.begin ()), it_base64_t (input.end ()));

    return result;
}

std::string base64_to_binary (string_view view) try
{
    using namespace boost::archive::iterators;
    using namespace std;
    using it_binary_t = transform_width<binary_from_base64<string::const_iterator>, 8, 6>;

    string result(it_binary_t(view.begin()), it_binary_t(view.end())); // decode
    return result;
}
catch (const std::exception&)
{
    return {};
}

string get_file_md5 (not_null<const char*> path)
{
    auto fp = file::fopen (path, "rb");

    if (!fp)
    {
        return {};
    }
    string result (32, '\0');
    std::array<unsigned char, 16> md5_buffer;
    MD5_CTX md5;

    fp.fseek (file_pos::tail);
    auto len = fp.ftell ();

    std::string file_buffer (static_cast<size_t> (len), '\0');

    fp.fseek (file_pos::head);
    if (fp.fread (file_buffer) != file_buffer.length ())
    {
        return {};
    }

    MD5Init (&md5);
    MD5Update (&md5, reinterpret_cast<const unsigned char*> (file_buffer.data ()), file_buffer.length ());
    MD5Final (&md5, md5_buffer.data ());

    auto convert_to_hex = [] (auto&& ch) { ch += (ch < 10 ? '0' : '7'); };

    size_t i = 0;
    for (auto it : md5_buffer)
    {
        SCOPE_EXIT { i ++; };
        result[i * 2] = it / 16;
        result[i * 2 + 1] = it % 16;
        convert_to_hex (result[i * 2]);
        convert_to_hex (result[i * 2 + 1]);
    }

    return result;
}

const string_view escape_set = R"(><&" )";
const array<string_view, 5> escape_content {string_view{"&gt;"}, string_view{"&lt;"}, string_view{"&amp"}, string_view{"&quot;"}, string_view{"&nbsp;"}};

string html_escape(string_view view)
{
    assert (escape_set.length () == 5);
    auto cur = view.begin ();
    auto stop = view.end ();
    std::string result;
    result.reserve (view.length () * 2);


    auto escape_pos = escape_set.begin ();
    while (cur != stop)
    {
        auto pos = find_if (cur, stop, [&] (auto ch) { return (escape_pos = find (escape_set.begin (), escape_set.end (), ch)) != escape_set.end (); });

        if (pos != cur)
        {
            result.append (cur, pos - cur);
        }
        if (pos == stop)
        {
            break;
        }

        auto& escape_str = escape_content [static_cast<unsigned int> (escape_pos - escape_set.begin ())];
        result.append (escape_str.data (), escape_str.length ());
        pos += 1;
        cur = pos;
    }

    return result;
}

#include <QDebug>
string url_encode(string_view raw)
{
    std::string result;
    auto convert_to_hex = [] (auto&& ch) { ch += (ch < 10 ? '0' : '7'); };

    result.reserve (raw.length () * 3);
    typename std::string::size_type i = 0;

    for (auto it : raw)
    {
        if (it >= 0)
        {
            result.push_back (it);
        }
        else
        {
            std::array<char, 3> arr;
            arr.at (0) = '%';
            arr.at (1) = static_cast<unsigned char> (it) / 16;
            arr.at (2) = static_cast<unsigned char> (it) % 16;

            convert_to_hex (arr.at (1));
            convert_to_hex (arr.at (2));
            result.append (arr.data (), arr.data () + arr.size ());
        }
    }

    return result;
}


























