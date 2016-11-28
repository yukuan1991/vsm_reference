#include "data_table.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QDebug>
#include "utility/raii.hpp"
#include "utility/using.hpp"
#include <boost/iterator/counting_iterator.hpp>

using namespace std;
using nlohmann::json;

data_table::data_table(QWidget* parent, const json& frame_info)
        :QTableWidget (parent)
{
    reset_frame_info (frame_info);
    horizontalHeader ()->setSectionResizeMode (QHeaderView::Interactive);
    horizontalHeader ()->setCascadingSectionResizes (true);

    verticalHeader ()->setVisible (false);
}

void data_table::reset_frame_info(const nlohmann::json &frame_info)
{
    if (frame_info.is_null ())
    {
        return;
    }

    setColumnCount (static_cast<int> (frame_info.size ()));
    int i = 0;
    for (auto& it : frame_info)
    {
        SCOPE_EXIT { i ++; };
        string item_name = it;
        auto item = make_unique<QTableWidgetItem> (item_name.data ());
        item->setSizeHint ({-1, 25});

        QFont font;
        font.setPixelSize (15);
        font.setBold (true);
        item->setFont (font);

        this->setHorizontalHeaderItem (i, item.release ());
    }
}

nlohmann::json data_table::dump() const
{
    json dumped = json::array ();

    for (int row = 0; row < rowCount (); row ++)
    {
        json row_json = json::object ();
        for (int col = 0; col < columnCount (); col ++)
        {
            auto h_header = horizontalHeaderItem (col); assert (h_header);
            auto header_text = h_header->text ().toStdString ();
            auto cell = item (row, col); assert (cell);
            row_json[header_text] = cell->text ().toStdString ();
        }
        dumped.push_back (::move (row_json));
    }

    return dumped;
}

void data_table::reset_data(const nlohmann::json &data_info)
{
    clearContents ();
    setRowCount (0);
    if (!data_info.is_array ())
    {
        return;
    }

    for (auto& it : data_info)
    {
        append_row (it);
    }
}

void data_table::append_row(const nlohmann::json &data_info)
{
    if (data_info.is_null ())
    {
        return;
    }

    auto old_row_count = rowCount ();
    setRowCount (old_row_count + 1);

    for (auto i : step (columnCount ()))
    {
        auto h_header = horizontalHeaderItem (i); assert (h_header);
        auto iter = data_info.find (h_header->text ().toStdString ());
        if (iter != data_info.end ())
        {
            string content;
            if (iter->is_string ())
            {
                content = *iter;
            }
            else
            {
                content = iter->dump ();
            }
            set_text (old_row_count, i, content.data ());
        }
        else
        {
            auto item = horizontalHeaderItem (i); assert (item);
            if (item->text ().isEmpty ())
            {
                set_text (old_row_count, i, "");
            }
            else
            {
                if (editable_col_.find (i) != editable_col_.end ())
                {
                    set_text (old_row_count, i, "");
                }
                else
                {
                    set_text (old_row_count, i, "N/A");
                }
            }
        }
    }
}


not_null<QTableWidgetItem *> data_table::find_item(int row, const QString &text)
{
    int column = get_col_by_name (text);

    if (column == -1)
    {
        except ("i == -1");
    }

    auto found = item (row, column);
    if (!found)
    {
        except ("found");
    }
    return found;
}

void data_table::set_editable(const QString &text, bool is_editable)
{
    int column = get_col_by_name (text);
    if (column == -1)
    {
        except ("找不到列名");
    }

    for (auto row : step (this->rowCount ()))
    {
        auto the_item = item (row, column); assert (the_item);
        if (is_editable)
        {
            the_item->setFlags (the_item->flags () | Qt::ItemIsEditable);
        }
        else
        {
            the_item->setFlags (the_item->flags () & (~Qt::ItemIsEditable));
        }
    }

    if (is_editable)
    {
        editable_col_.insert(column);
    }
    else
    {
        auto iter = editable_col_.find(column);
        if (iter != editable_col_.end())
        {
            editable_col_.erase(iter);
        }
    }
}

void data_table::set_item_text(int row, const QString &title, const QString &text)
{
    auto col = get_col_by_name (title);
    set_text (row, col, text);
}


void data_table::set_text(int row, int col, const QString& text)
{
    if (row >= rowCount () or col >= columnCount ())
    {
        return;
    }
    auto item = make_unique<QTableWidgetItem> (text);
    if (editable_col_.find(col) != editable_col_.end())
    {
        item->setFlags (item->flags () | Qt::ItemIsEditable);
    }
    else
    {
        item->setFlags (item->flags () & (~Qt::ItemIsEditable));
    }
    QFont font;
    font.setPixelSize (15);
    item->setFont (font);
    setItem (row, col, item.release ());
}

QString data_table::get_name_by_col(int col)
{
    auto header = horizontalHeaderItem (col);
    if (header)
    {
        return header->text ();
    }
    else
    {
        return {};
    }
}

int data_table::get_col_by_name(const QString &text)
{
    boost::counting_iterator<int> start = 0;
    boost::counting_iterator<int> stop = columnCount ();

    auto found = std::find_if (start, stop, [&] (auto&& i) { return this->horizontalHeaderItem (i)->text () == text; });

    return found == stop ? -1 : *found;
}
