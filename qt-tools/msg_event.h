#ifndef MSG_EVENT_HPP
#define MSG_EVENT_HPP
#include <QEvent>
#include <functional>
#include <memory>
#include <QApplication>

constexpr QEvent::Type event_msg = static_cast<QEvent::Type> (10000);
class msg_event : public QEvent
{
public:
    template<typename FUNC>
    msg_event (FUNC&& f) : QEvent (event_msg), func_ (std::forward<FUNC> (f)) {}

    ~msg_event () override;
    std::function<void ()> func_;
};


#endif // MSG_EVENT_HPP
