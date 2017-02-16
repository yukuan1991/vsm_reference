#include "qt-tools/application.h"
#include <QPushButton>
#include <QWidget>
#include "qt-tools/video_widget.hpp"


int main (int argc, char** argv)
{
    QApplication app (argc, argv);
    auto v = video_widget::make();
    v->show();
    v->set_file ("D:/1.mp4");

    app.exec ();
    return 0;
}
