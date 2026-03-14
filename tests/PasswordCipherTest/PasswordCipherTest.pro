QT += testlib core
CONFIG += console testcase c++11
TEMPLATE = app
TARGET = tst_passwordcipher

INCLUDEPATH += $$PWD/../..

SOURCES += \
    tst_passwordcipher.cpp \
    ../../core/PasswordCipher.cpp

HEADERS += \
    ../../core/PasswordCipher.h

# OpenSSL
!isEmpty(OPENSSLINCLUDEPATH) {
    INCLUDEPATH += $$OPENSSLINCLUDEPATH
}
!isEmpty(OPENSSLLIBPATH) {
    LIBS += -L$$OPENSSLLIBPATH
}

win32 {
    LIBS += -llibcrypto
} else {
    LIBS += -lcrypto
}
