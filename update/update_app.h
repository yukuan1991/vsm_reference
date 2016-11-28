#ifndef UPDATE_APP_H
#define UPDATE_APP_H

#include "qt-tools/application.h"
#include <string>


class update_app : public application
{
public:
    update_app (int argc, char** argv) : application (argc, argv) {}

    bool run () override;
    void at_exit () override;
private:
    std::string start_program_;
    bool success_ = false;
};

#endif // UPDATE_APP_H
