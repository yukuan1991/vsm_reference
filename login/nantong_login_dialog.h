#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include <QDialog>
#include <QString>
#include "json.hpp"
#include <QPixmap>
#include <QResizeEvent>
#include "login/login.hpp"

namespace Ui {
class nantong_login_dialog;
}

class nantong_login_dialog : public abstract_login_dialog
{
    Q_OBJECT
public:
    explicit nantong_login_dialog(QWidget *parent = 0);
    ~nantong_login_dialog() override;
public:
    void set_username (QString) override;
    void set_password (QString) override;

    QString password () override;
    QString username () override;

    void set_pass_remembered (bool) override;
    bool is_pass_remembered () override;

    void start_login_scene () override;
    void stop_login_scene () override;

protected:
    void mousePressEvent (QMouseEvent* event) override;
    void mouseMoveEvent (QMouseEvent* event) override;
    void mouseReleaseEvent (QMouseEvent* event) override;
    void keyPressEvent (QKeyEvent* event) override;

private slots:
    void on_button_login_clicked();

    void set_background_image (const QString& path);

private:
    Ui::nantong_login_dialog *ui;
    bool is_dragged_ = false;
    QPoint drag_pos_;
};

#endif // LOGIN_DIALOG_H
