#include "login_dialog.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    login_dialog w;

    w.init ({{"username", "Krys"}, {"password", "123456"}, {"background", IMAGE_PATH "/background_image.jpg"}});

    if (w.exec () == QDialog::Accepted)
    {
        //qDebug () << " user: " << w.get_user_name() << "  password: " << w.get_password ();
        auto result = w.result (); assert (result.is_object ());
        auto iter = result.find ("username"); assert (iter->is_string ());
        assert (iter != result.end ());
        std::string username = *iter;

        iter = result.find ("password"); assert (iter->is_string ());
        assert (iter != result.end ());
        std::string password = *iter;

		
        qDebug () << "username: " << username.data () << " password: " << password.data ();
    }

    return 0;
}
