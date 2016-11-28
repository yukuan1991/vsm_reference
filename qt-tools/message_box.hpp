#ifndef MESSAGE_BOX_H
#define MESSAGE_BOX_H

#include <QMessageBox>
#include <utility/raii.hpp>
#include <QPoint>
#include <QMouseEvent>

class message_box : public QMessageBox
{
public:
    template<typename ... ARGS>
    message_box(ARGS && ... args) : QMessageBox (std::forward<ARGS> (args)...) {}

    void mousePressEvent (QMouseEvent* event) override
    {
        is_dragged_ = true;
        drag_pos_ = event->globalPos () - pos ();
        QMessageBox::mousePressEvent (event);
    }
    void mouseMoveEvent (QMouseEvent* event) override
    {
        SCOPE_EXIT { QMessageBox::mouseMoveEvent (event); };
        if (is_dragged_ and event->buttons () & Qt::LeftButton )
        {
            move (event->globalPos () - drag_pos_);
            event->accept ();
            setWindowOpacity (0.5);
        }
    }
    void mouseReleaseEvent (QMouseEvent* event) override
    {
        is_dragged_ = false;
        setWindowOpacity (1);
        QMessageBox::mouseReleaseEvent (event);
    }

    ~message_box () override = default;
private:
    bool is_dragged_ = false;
    QPoint drag_pos_;

};

#endif // MESSAGE_BOX_H
