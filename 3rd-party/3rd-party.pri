
INCLUDEPATH += $$PWD

HEADERS += \
	$$PWD/json.hpp \
	$$PWD/fmt/format.h \
	$$PWD/fmt/ostream.h \
	$$PWD/fmt/posix.h \
	$$PWD/fmt/printf.h \
	$$PWD/fmt/string.h \
	$$PWD/fmt/time.h \
	$$PWD/cppformat.hpp \
    $$PWD/md5.h \
	$$PWD/tinyxml2.h \
	$$PWD/folly/detail/UncaughtExceptionCounter.h \
    $$PWD/des.h

SOURCES += \
    $$PWD/md5.cc \
    $$PWD/tinyxml2.cpp \
    $$PWD/des.cc
