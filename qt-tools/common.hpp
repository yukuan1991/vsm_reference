﻿#pragma once
#include "utility/tcp.h"
#include <QMessageBox>
#include "qt-tools/message_box.hpp"
#include <QPushButton>
#include <memory>
#include "utility/encryption.h"
#include <experimental/string_view>
#include "utility/raii.hpp"

inline std::vector<std::pair<std::string, std::string>> check_for_update (const char* server_addr, const char* path, std::experimental::string_view software_name) try
{
    auto data = ::json_http_get (server_addr, path, {{"software_name", software_name.to_string ()}});
    auto & files = data ["info"];

    std::vector<std::pair<std::string, std::string>> update_files;
    for (auto& file : files)
    {
        std::string path = file ["path"];
        std::string md5 = file ["md5"];
        int override = file ["override"];

        auto real_md5 = ::get_file_md5 (::utf_to_sys (path.data ()).data());
        if (real_md5.empty () or (md5 != real_md5 and override != 0))
        {
            update_files.emplace_back (path, md5);
        }
    }

    return update_files;
}
catch (const std::exception&)
{
    return {};
}

