#include "screen_saver.h"
#include <QPaintEvent>
#include <QScreen>
#include <QPainter>
#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QDebug>
#include <assert.h>
#include <QMouseEvent>
#include <QClipboard>
#include "utility/raii.hpp"

QPixmap screen_saver::grab(copy_to_clip opt)
{
    QPixmap map;
    screen_saver s (map);
    auto result = s.exec ();

    if (result == Accepted)
    {
        if (opt == copy_to_clip::yes)
        {
            QApplication::clipboard ()->setPixmap (map);
        }

        return map;
    }
    else
    {
        return {};
    }
}

QPixmap screen_saver::snapshot(WId win_id)
{
    return QApplication::primaryScreen ()->grabWindow (win_id);
}

screen_saver::screen_saver(QPixmap& pix_ref, QWidget *parent)
        : QDialog(parent),
          screen_shot_ (pix_ref)
{
}

screen_saver::~screen_saver()
{

}

void screen_saver::showEvent(QShowEvent *event)
{
    if (!isFullScreen ())
    {
        showFullScreen ();
    }
    setCursor (QCursor (Qt::CrossCursor));
    QDialog::showEvent (event);
}

void screen_saver::paintEvent(QPaintEvent *event)
{
    SCOPE_EXIT { QDialog::paintEvent (event); };
    QPainter p (this);

    p.drawPixmap (0, 0, full_screen_);
    p.fillRect(this->rect (), QColor(0, 0, 0, 200));
    QFont font;

    if (!pressed_pos_)
    {
        return;
    }

    auto& start_pos = *pressed_pos_;
    auto end_pos = QCursor::pos ();

    font.setPixelSize (font_size);
    font.setBold (false);
    p.setFont (font);

    // 文字颜色
    p.setPen (QColor (Qt::red));

    QString show_str;

    show_str.sprintf ("截图范围:起点(%d , %d), 终点(%d , %d), 矩形长宽 (%d , %d)",
                      start_pos.x (), start_pos.y (),
                      end_pos.x (), end_pos.y (),
                      std::abs (end_pos.x () - start_pos.x ()), std::abs (end_pos.y () - start_pos.y ()));

    QTextOption opt;
    opt.setTextDirection (start_pos.x () < end_pos.x () ? Qt::LeftToRight : Qt::RightToLeft);
    p.drawText (QRect (end_pos.x () > start_pos.x () ? start_pos.x () : (start_pos.x () - font_size * 50),
                       end_pos.y () > start_pos.y () ? (start_pos.y () - font_size) : (start_pos.y ()),
                       font_size * 50,
                       font_size), show_str, opt);

    QRect selected_rect (QPoint (std::min (start_pos.x (), end_pos.x ()), std::min (start_pos.y (), end_pos.y ())),
                         QPoint (std::max (start_pos.x (), end_pos.x ()), std::max (start_pos.y (), end_pos.y ())));
    p.drawPixmap (selected_rect, full_screen_, selected_rect);

    //框线颜色
    p.setPen (QColor (Qt::black));
    p.drawRect (start_pos.x (), start_pos.y (), end_pos.x () - start_pos.x (), end_pos.y () - start_pos.y ());
}

void screen_saver::mousePressEvent(QMouseEvent *event)
{
    if (event->button () == Qt::LeftButton)
    {
        pressed_pos_.emplace (event->globalPos ());
    }
    else if (event->button () == Qt::RightButton)
    {
        reject ();
    }
    QDialog::mousePressEvent (event);
}

void screen_saver::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button () == Qt::LeftButton)
    {
        assert (pressed_pos_);
        auto start = *pressed_pos_;
        auto stop = event->globalPos ();

        QPoint left_top (std::min (start.x (), stop.x ()), std::min (start.y (), stop.y ()));
        QPoint right_bottom (std::max (start.x (), stop.x ()), std::max (start.y (), stop.y ()));
        screen_shot_ = full_screen_.copy (QRect (left_top, right_bottom));

        pressed_pos_ = {};

        if (left_top == right_bottom)
        {
            repaint ();
            return;
        }

        after_grab (event);
    }
    else
    {
        QDialog::mouseReleaseEvent (event);
    }
}

void screen_saver::mouseMoveEvent(QMouseEvent *event)
{
    if (pressed_pos_)
    {
        repaint ();
    }
    QDialog::mouseMoveEvent (event);
}

void screen_saver::after_grab(QMouseEvent* event)
{
    enum class pix_action : int
    {
        save_pix,
        redo_pix,
        discard_pix,
        no_selection
    };

    pix_action action = pix_action::no_selection;
    QMenu menu;
    connect (menu.addAction ("完成截图"), &QAction::triggered, [&] { action = pix_action::save_pix; });
    connect (menu.addAction ("从新截图"), &QAction::triggered, [&] { action = pix_action::redo_pix; });
    connect (menu.addAction ("取消截图"), &QAction::triggered, [&] { action = pix_action::discard_pix; });

    menu.exec (event->pos ());

    if (action == pix_action::save_pix)
    {
        accept ();
    }
    else if (action == pix_action::discard_pix)
    {
        reject ();
    }
    else
    {
        repaint ();
    }
}
