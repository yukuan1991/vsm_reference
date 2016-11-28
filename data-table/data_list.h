#ifndef DATA_LIST_H
#define DATA_LIST_H
#include <QListWidget>
#include <vector>
#include <string>


class data_list : public QListWidget
{
    Q_OBJECT
public:
    data_list(QWidget* parent = nullptr);

    std::vector<std::string> dump_checked ();
    void refresh_list (const std::vector<std::string>& list);
    void set_checkable (bool checkable = true) { is_checkable_ = checkable; }
    ~data_list ();
private:
    bool is_checkable_ = false;
};

#endif // DATA_LIST_H
