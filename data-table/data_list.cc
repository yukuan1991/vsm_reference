#include "data_list.h"
#include <memory>
#include "utility/interface.hpp"
#include <assert.h>
#include "utility/raii.hpp"
#include "utility/interface.hpp"


using namespace std;

data_list::data_list(QWidget* parent)
        :QListWidget (parent)
{

}

vector<string> data_list::dump_checked()
{
    vector<string> result;
    auto rows = step (count ());

    for (auto i : rows)
    {
        auto the_item = item (i); assert (the_item);
        if (the_item->checkState () == Qt::CheckState::Checked)
        {
            result.emplace_back (the_item->text ().toStdString ());
        }
    }
    return result;
}

void data_list::refresh_list(const std::vector<std::string> &list)
{
    clear ();
    for (auto& it : list)
    {
        auto item = make_unique<QListWidgetItem> (it.data ());
        if (is_checkable_)
        {
            item->setFlags (item->flags () | Qt::ItemIsUserCheckable);
            item->setCheckState (Qt::CheckState::Unchecked);
        }
        this->addItem (item.release ());
    }

}

data_list::~data_list()
{

}
