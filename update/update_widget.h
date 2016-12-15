#ifndef UPDATE_WIDGET_H
#define UPDATE_WIDGET_H

#include <QWidget>
#include <memory>
#include "json.hpp"

namespace Ui {
class update_widget;
}

class update_widget : public QWidget
{
    Q_OBJECT

public:
    explicit update_widget(QWidget *parent = 0);
    ~update_widget();
    void init (nlohmann::json data);
signals:
    void success ();

private:
    static void download_process (update_widget* , std::string, std::string, std::weak_ptr<bool>, std::string);
    void on_progress_triggered (unsigned long long current, unsigned long long total);
    void update_percentage (unsigned p);
    void next_file ();
    void finished ();
    void failed ();

private:
    Ui::update_widget *ui;
    std::shared_ptr<bool> alive_ = std::make_shared<bool> ();
    std::vector<std::pair<std::string, std::string>> file_list_;
    std::string prefix_;
    unsigned index_ = 0;
private:
    static std::string server_addr;
};

#endif // UPDATE_WIDGET_H
