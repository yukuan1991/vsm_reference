#include "qt-tools/application.h"
#include <QDebug>



bool application::run()
{
    return false;
}

int application::exec()
{
    return QApplication::exec ();
}

void application::customEvent(QEvent *event)
{
    if (event_msg == event->type ())
    {
        auto msg = dynamic_cast<msg_event*> (event); assert (msg);
        try
        {
            msg->func_ ();
        }
        catch (const std::exception& e)
        {
            qDebug () << e.what ();
        }
        return;
    }

    QApplication::customEvent (event);
}
