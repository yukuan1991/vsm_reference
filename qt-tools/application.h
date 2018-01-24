#ifndef QT_APPLICATION_HPP
#define QT_APPLICATION_HPP
#include <QApplication>
#include "qt-tools/msg_event.h"
#include <qt_windows.h>
#include <boost/thread.hpp>
#include "verification/verification.h"
#include <QTimer>

#define APP_REGISTER(class_name) \
int main (int argc, char** argv) \
{\
    try \
    {\
        class_name app (argc, argv);\
        if(!verification_process())\
        {\
            return -1;\
        }\
        int ret = -1; \
        if (app.run ())\
        {\
            QTimer timer;\
            timer.setInterval (1000);\
            timer.setSingleShot (true);\
            QObject::connect (&timer, &QTimer::timeout, [&] { check_date (); timer.start (); });\
            timer.start ();\
            ret = app.exec ();\
        }\
        app.at_exit (); \
        return ret;\
    }\
    catch (const std::exception& e)\
    {\
        show_exception (e.what ());\
        return -1;\
    }\
}


inline void show_exception (const char* what)
{
    MessageBoxA (0, what, "unhandled exception", MB_OK);
}

class application : public QApplication
{
public:
    application (int argc, char** argv) : QApplication (argc, argv) {}
    static void post_event (QApplication* app, std::unique_ptr<QEvent> event, int priority = Qt::NormalEventPriority)
    {
        QApplication::postEvent (app, event.release (), priority);
    }

    virtual bool run ();
    int exec ();
    virtual void at_exit () {}
    void customEvent (QEvent* event) override;
    virtual ~application () override = default;
private:
};


#define call_after __CALL_AFTER__{},

struct __CALL_AFTER__
{
    template<typename T>
    void operator, (T&& t)
    {
        application::post_event (static_cast<QApplication*> (QApplication::instance ()), std::make_unique<msg_event> (std::forward<T> (t)));
    }
};


#define go __GO__{},
struct __GO__
{
    template<typename T>
    void operator, (T&& t)
    {
        boost::thread {std::forward<T>(t)}.detach();
        //boost::thread { [callable = std::forward<T> (t)] { try { callable ();} catch (const std::exception& e) {show_exception (e.what ());} }}.detach ();
    }
};

#endif // QT_APPLICATION_HPP
