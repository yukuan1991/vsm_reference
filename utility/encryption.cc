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
#include "des.h"

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


void krys3des_decryption (const std::string & input, const char* key24bytes, std::string & output)
{

    /*--------------------------------------------init data--------------------------------------------*/
    des3_context context;
    des3_set_3keys (&context, (const unsigned char*)key24bytes);
    int length = input.length()/2;

    char tmp[3];
    tmp[2] = 0;

    auto source_3des = std::make_unique<char[]>(length);
    auto output_3des = std::make_unique<char[]>(length);
    output.resize (length);
    /*----------------------------------------------END------------------------------------------------*/



    /*------------------------------------------length examine-----------------------------------------*/
    if (length % 8 != 0 or length == 0)
    {
        output.clear();
        return;
    }
    /*----------------------------------------------END------------------------------------------------*/


    /*---------------------------------hex to bytestream converting------------------------------------*/
    for (unsigned long i=0; i<input.size()/2; i++)
    {
        strncpy (tmp, input.c_str() + 2 * i, 2);
        sscanf (tmp, "%hhx", source_3des.get() + i);
    }
    /*----------------------------------------------END------------------------------------------------*/


    /*-------------------------------------------decryption loop---------------------------------------*/
    for (int i = 0; i < length; i += 8)
    {
        //des3_decrypt (&context, (unsigned char*)source_3des.get() + i, (unsigned char*)output_3des.get() + i);
        des3_decrypt (&context, (unsigned char*)source_3des.get () + i, (unsigned char*)output.data () + i);
    }
    /*----------------------------------------------END------------------------------------------------*/

    /*-----------------------------------------validating result---------------------------------------*/
    if (output[length - 1] < 1 or output[length - 1] > 8)
    {
        output.clear ();
        return;
    }

    for (const char* p = output.data () + length - 1; p > output.data () + length - 1 - output[length - 1]; p--)
    {
        if (*p != output[length - 1])
        {
            output.clear ();
            return;
        }
    }
    output.resize (length - output[length - 1]);
    /*----------------------------------------------END------------------------------------------------*/
}


void krys3des_encryption (const std::string & source, const char* key24bytes, std::string & output)
{
    if (key24bytes == nullptr)
    {
        errno = EINVAL;
        output.clear();
        return;
    }

    /*------------------------------generating des3 context--------------------------------------------*/
    des3_context context;
    int input_length;
    int des3_length;
    /*-------------------------------------------------------------------------------------------------*/

    /*-------------------------------------------init data---------------------------------------------*/
    des3_set_3keys (&context, (unsigned char*)key24bytes);
    input_length = source.length();
    des3_length = input_length + (8 - input_length % 8);

    char hexbyte[3];  /*hex temp*/
    output.clear();

    auto source_3des = std::make_unique<char[]> (des3_length);		/*des3 formatted input*/
    auto output_3des = std::make_unique<char[]> (des3_length);		/*des3 formatted output*/
    /*----------------------------------------------END------------------------------------------------*/



    /*-----------------------------------set memory with fixed length----------------------------------*/
    memcpy (source_3des.get(), source.c_str(), input_length);
    memset (source_3des.get() + input_length, des3_length - input_length, des3_length - input_length);
    /*----------------------------------------------END------------------------------------------------*/


    /*-------------------------------------encryption blockwise----------------------------------------*/
    for (int i = 0; i < des3_length; i += 8)
    {
        des3_encrypt (&context, (unsigned char*)source_3des.get () + i, (unsigned char*)output_3des.get() + i);
    }
    /*----------------------------------------------END------------------------------------------------*/


    /*--------------------------------------------hex output-------------------------------------------*/
    for (int i = 0; i < des3_length; i++)
    {
        sprintf (hexbyte, "%02hhx", (unsigned char)output_3des[i]);
        output += hexbyte;
    }
    /*----------------------------------------------END------------------------------------------------*/

}
























