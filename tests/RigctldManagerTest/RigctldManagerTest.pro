QT += testlib core network
CONFIG += console testcase c++11
TEMPLATE = app
TARGET = tst_rigctldmanager

# Local stubs first, then project includes
INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/../..

SOURCES += \
    tst_rigctldmanager.cpp \
    ../../rig/RigctldManager.cpp \
    ../../data/SerialPort.cpp \
    test_stubs.cpp

HEADERS += \
    ../../rig/RigctldManager.h \
    ../../data/SerialPort.h \
    data/RigProfile.h \
    core/debug.h

# Hamlib
!isEmpty(HAMLIBINCLUDEPATH) {
    INCLUDEPATH += $$HAMLIBINCLUDEPATH
}

isEmpty(HAMLIBVERSION_MAJOR): HAMLIBVERSION_MAJOR = 4
isEmpty(HAMLIBVERSION_MINOR): HAMLIBVERSION_MINOR = 0
isEmpty(HAMLIBVERSION_PATCH): HAMLIBVERSION_PATCH = 0

DEFINES += HAMLIBVERSION_MAJOR=$$HAMLIBVERSION_MAJOR
DEFINES += HAMLIBVERSION_MINOR=$$HAMLIBVERSION_MINOR
DEFINES += HAMLIBVERSION_PATCH=$$HAMLIBVERSION_PATCH

# pthreads (needed by hamlib headers on Windows)
!isEmpty(PTHREADINCLUDEPATH) {
    INCLUDEPATH += $$PTHREADINCLUDEPATH
}
!isEmpty(PTHREADLIBPATH) {
    LIBS += -L$$PTHREADLIBPATH
}

# Hamlib link
!isEmpty(HAMLIBLIBPATH) {
    LIBS += -L$$HAMLIBLIBPATH
}
win32: LIBS += -lws2_32 -llibhamlib-4
unix:  LIBS += -lhamlib
