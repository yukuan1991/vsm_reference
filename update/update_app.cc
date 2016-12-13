#include "update_app.h"
#include <QDebug>
#include <QMessageBox>
#include "utility/encryption.h"
#include <json.hpp>
#include "utility/raii.hpp"
#include "update_widget.h"
#include "utility/file.hpp"

using json = nlohmann::json;


APP_REGISTER(update_app)

bool update_app::run()
{
    auto args = this->arguments ();
    if (args.size () < 2)
    {
        QMessageBox::information (nullptr, "更新", "启动参数错误");
        return false;
    }

    auto base64_args = args[1].toStdString ();

    std::string json_args = ::base64_to_binary (base64_args);
    if (json_args.empty ())
    {
        QMessageBox::information (nullptr, "更新", "启动参数错误 (不符合base64格式)");
        return false;
    }

    try
    {
        auto json_data = json::parse (json_args);
        auto w = new update_widget;
        connect (w, &update_widget::success, [this] { success_ = true; });
        start_program_ = json_data["exec"];
        w->init (::move (json_data));
        w->show ();
        return true;
    }
    catch (const std::exception&)
    {
        QMessageBox::information (nullptr, "更新", "启动参数错误 (不符合json格式)");
        return false;
    }
}

void update_app::at_exit()
{

    if (success_)
    {
        ::system (("start " + start_program_).data ());
    }
}


