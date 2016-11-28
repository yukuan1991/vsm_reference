#ifndef LOGIN_HPP
#define LOGIN_HPP

#include <experimental/optional>
#include "json.hpp"
#include <QDialog>

class abstract_login_dialog : public QDialog
{
    Q_OBJECT
signals:
    void start_login ();
public:
    template<typename ... ARGS>
    abstract_login_dialog (ARGS && ... args) : QDialog (std::forward<ARGS> (args)...) {}

    virtual void set_username (QString) = 0;
    virtual void set_password (QString) = 0;
    virtual QString password () = 0;
    virtual QString username () = 0;

    virtual void set_pass_remembered (bool) = 0;
    virtual bool is_pass_remembered () = 0;
    virtual void start_login_scene () = 0;
    virtual void stop_login_scene () = 0;
private:

};

#endif // LOGIN_HPP
