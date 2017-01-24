
# C++14 standard
CONFIG += c++14

INCLUDEPATH += $$PWD

QMAKE_CXXFLAGS += -Wextra
QMAKE_CXXFLAGS += -Wno-deprecated-declarations
QMAKE_CXXFLAGS += -Werror=write-strings
QMAKE_CXXFLAGS += -Werror=return-type
QMAKE_CXXFLAGS += -Werror=parentheses
DEFINES += CURL_STATICLIB

LIBS += -lboost_filesystem-mgw53-mt-1_61
LIBS += -lboost_system-mgw53-mt-1_61
LIBS += -lboost_regex-mgw53-mt-1_61
LIBS += -lboost_thread-mgw53-mt-1_61
LIBS += -lboost_locale-mgw53-mt-1_61
LIBS += -liconv
LIBS += -lwininet
#LIBS += -lcurl
LIBS += -lws2_32
#LIBS += -lwldap32
