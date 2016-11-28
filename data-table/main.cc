#include "data-table/data_table.h"
#include <QApplication>
#include "utility/using.hpp"
#include "utility/raii.hpp"
#include <QDebug>
#include <algorithm>
#include "data_list.h"
#include <QHeaderView>
#include <QPushButton>


int main (int argc, char** argv)
{
    QApplication app (argc, argv);
    data_list list;
    list.refresh_list ({"123", "456", "789"});
    auto item0 = list.item (0);
    auto table = new QTableWidget (&list);
    table->verticalHeader ()->setVisible (false);
    table->setRowCount (10);
    table->setColumnCount (12);
    qDebug () << table->sizeHint ();
    list.setItemWidget (item0, table);
    item0->setSizeHint (table->sizeHint ());

    list.show ();

    return app.exec ();
}
