#ifndef DATA_TABLE_H
#define DATA_TABLE_H

#include <QTableWidget>
#include "json.hpp"
#include "utility/interface.hpp"
#include <set>

class data_table : public QTableWidget
{
    Q_OBJECT
public:
    data_table(QWidget* parent = nullptr, const nlohmann::json& frame_info = {});

    void reset_frame_info (const nlohmann::json& frame_info);
    nlohmann::json dump () const;
    void reset_data (const nlohmann::json& data_info);
    void append_row (const nlohmann::json& data_info);
    not_null<QTableWidgetItem*> find_item (int row, const QString& text);

    void set_editable (const QString& text, bool is_editable = true);
    void set_item_text (int row, const QString& title, const QString& text);
    void set_non_editable (const QString& text);
    void set_text (int row, int col, const QString& text);
    QString get_name_by_col (int col);
private:
    int get_col_by_name (const QString& text);

private:
    std::set<int> editable_col_;
};

#endif // DATA_TABLE_H
