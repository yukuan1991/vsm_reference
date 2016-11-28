#include "utility/file.hpp"
#include "utility/tcp.h"
#include "utility/raii.hpp"
#include "utility/encryption.h"
#include <boost/phoenix.hpp>

using std::experimental::string_view;
using namespace boost::phoenix::arg_names;
using namespace boost::phoenix;


int main ()
{
    auto jpg = read_all ("E:\\1.jpg");
    if (!jpg)
    {
        std::cout << "open jpg failed\n";
    }
    auto b64_jpg = binary_to_base64 (*jpg);

    auto restore_jpg = base64_to_binary (b64_jpg);
    std::cout << b64_jpg << '\n';

    if (!write_buffer ("E:\\2.jpg", restore_jpg))
    {
        std::cout << "write_buffer failed\n";
    }
    return 0;
}



