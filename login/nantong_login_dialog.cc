#include "login/nantong_login_dialog.h"
#include "ui_nantong_login_dialog.h"
#include "json.hpp"
#include <QKeyEvent>
#include <QColor>
#include "qt-tools/common.hpp"
#include <QPalette>
#include <QPainter>
#include <QMessageBox>
#include <QDebug>
#include <QPainterPath>

using json = nlohmann::json;

nantong_login_dialog::nantong_login_dialog(QWidget *parent) :
    abstract_login_dialog(parent, Qt::FramelessWindowHint),
    ui(new Ui::nantong_login_dialog)
{
    ui->setupUi(this);
    ui->button_close->setIcon (this->style ()->standardIcon (QStyle::SP_TitleBarCloseButton));
}

nantong_login_dialog::~nantong_login_dialog()
{
    delete ui;
}


void nantong_login_dialog::mousePressEvent(QMouseEvent *event)
{
    SCOPE_EXIT { abstract_login_dialog::mousePressEvent (event); };
    if (!(event->buttons () & Qt::LeftButton)) return;

    is_dragged_ = true;
    drag_pos_ = event->globalPos () - pos ();
    event->accept ();
}

void nantong_login_dialog::mouseMoveEvent(QMouseEvent *event)
{
    SCOPE_EXIT { abstract_login_dialog::mouseMoveEvent (event); };
    if (!(is_dragged_ and event->buttons () & Qt::LeftButton )) return;

    setWindowOpacity (0.5);
    move (event->globalPos () - drag_pos_);
    event->accept ();
}

void nantong_login_dialog::mouseReleaseEvent(QMouseEvent *event)
{
    SCOPE_EXIT { abstract_login_dialog::mouseMoveEvent (event); };

    setWindowOpacity (1);
    is_dragged_ = false;
}

void nantong_login_dialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key () != Qt::Key_Enter and event->key () != Qt::Key_Return)
    {
        abstract_login_dialog::keyPressEvent (event);
    }
}

void nantong_login_dialog::set_background_image(const QString& path)
{
    if (!QFile::exists (path)) return;

    QPixmap pix ("white");
    pix.load (path);
    QPalette pal;
    pal.setBrush (QPalette::Window, QBrush (pix));
    this->setPalette (pal);
    this->setAutoFillBackground (false);

}

void nantong_login_dialog::set_username(QString username)
{
    ui->edit_user->setText (::move (username));
}

void nantong_login_dialog::set_password(QString password)
{
    ui->edit_password->setText (::move (password));
}

QString nantong_login_dialog::password()
{
    return ui->edit_password->text ();
}

QString nantong_login_dialog::username()
{
    return ui->edit_user->text ();
}

void nantong_login_dialog::set_pass_remembered(bool checked)
{
    ui->checkbox_remember_pass->setChecked (checked);
}

bool nantong_login_dialog::is_pass_remembered()
{
    return ui->checkbox_remember_pass->isChecked ();
}

void nantong_login_dialog::start_login_scene()
{
    ui->button_login->setEnabled (false);
    ui->label_title->setText ("正在登录...");
}

void nantong_login_dialog::stop_login_scene()
{
    ui->button_login->setEnabled (true);
    ui->label_title->setText ("用户登录");
}

void nantong_login_dialog::on_button_login_clicked()
{
    emit start_login ();
}
