#include "utility/tcp.h"
#include "update_widget.h"
#include "ui_update_widget.h"
#include "qt-tools/application.h"
#include "utility/raii.hpp"
#include "utility/interface.hpp"
#include "utility/interface.hpp"
#include <boost/filesystem/operations.hpp>
#include <QMessageBox>
#include <QDebug>
#include "cppformat.hpp"

using namespace std;
string update_widget::server_addr;

update_widget::update_widget(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::update_widget)
{
    ui->setupUi(this);
    this->setAttribute (Qt::WA_DeleteOnClose);
}

update_widget::~update_widget()
{
    delete ui;
}

void update_widget::init(nlohmann::json data)
{
    vector<string> file_list = data ["files"];
    if (file_list.empty ())
    {
        QMessageBox::information (this, "更新", "更新文件为空");
        close ();
        return;
    }

    boost::filesystem::create_directories ("tmp");

    file_list_ = ::move (file_list);
    for (auto& it : file_list_)
    {
        it = boost::locale::conv::between (it, "", "utf-8");
    }

    prefix_ = data["prefix"];
    update_widget::server_addr = data["server_addr"];

    ui->progress_file->setRange (0, static_cast<int> (file_list_.size ()));
    next_file ();
}

void update_widget::download_process(update_widget* ptr, std::string server_path, std::string path, weak_ptr<bool> wp)
{
    auto on_progress_callback = [wp, ptr] (unsigned long long current, unsigned long long total) mutable
    {
        call_after [wp, ptr, current, total]
        {
            if (wp.lock () == nullptr) { return; }

            ptr->on_progress_triggered (current, total);
            float f_current = current;
            float f_total = total;
            ptr->update_percentage (static_cast<unsigned> (f_current / f_total * 100));
        };
    };

    auto ret = http_dl (("http://" + update_widget::server_addr + server_path).data (), path.data (), on_progress_callback);
    call_after [wp = ::move (wp), ptr, ret]
    {
        if (wp.lock () == nullptr) return;

        if (ret == 0)
        {
            ptr->next_file ();
        }
        else
        {
        ptr->failed ();
    }

    };
}

void update_widget::on_progress_triggered(unsigned long long current, unsigned long long total)
{
    ui->progress_data->setValue (static_cast<int> (current));
    ui->progress_data->setRange (0, static_cast<int> (total));
}

void update_widget::update_percentage(unsigned p)
{
    ui->label_percent->setText (fmt::format ("更新进度:{} %", p).data ());
}

void update_widget::next_file()
{
    ui->progress_file->setRange (0, static_cast<int> (file_list_.size ()));
    ui->progress_file->setValue (static_cast<int> (index_));

    if (index_ == file_list_.size ())
    {
        finished ();
        return;
    }


    auto current_index = index_;
    index_ ++;

    ui->label_file->setText (fmt::format ("正在更新文件:{}, {}/{}", file_list_.at (current_index), current_index + 1, file_list_.size ()).data ());

    go [this, wp = weak (alive_), server_path = "/" + prefix_ + "/" + file_list_.at(current_index), file = file_list_.at (current_index)] () mutable
    {
        auto tmp_path = "tmp/" + file;
        auto tmp = boost::filesystem::absolute (tmp_path);
        boost::filesystem::create_directories (tmp.parent_path ());

        download_process (this, server_path, tmp_path, ::move (wp));
    };
}

void update_widget::finished()
{
    /// 移动文件到软件目录
    emit success ();
    for (auto & it : file_list_)
    {
        /// 创建父目录
        auto p = boost::filesystem::absolute (it);
        auto parent = p.parent_path ();
        boost::filesystem::create_directories (parent);
        /// 移动文件
        boost::filesystem::rename ("tmp/" + it, it);
    }

    /// 删除临时文件夹
    boost::filesystem::remove_all ("tmp");

    close ();
    QApplication::exit (0);
}

void update_widget::failed()
{
    /// 弹出对话框
    QMessageBox::information (this, "升级失败", "下载文件出错");
    boost::filesystem::remove_all ("tmp");

    close ();
    QApplication::exit (-1);
}
