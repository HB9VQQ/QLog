QT += testlib core widgets
CONFIG += console testcase c++11
TEMPLATE = app
TARGET = tst_filecompressor

INCLUDEPATH += $$PWD/../..

SOURCES += \
    tst_filecompressor.cpp \
    ../../core/FileCompressor.cpp

HEADERS += \
    ../../core/FileCompressor.h

# zlib
!isEmpty(ZLIBINCLUDEPATH) {
    INCLUDEPATH += $$ZLIBINCLUDEPATH
}
!isEmpty(ZLIBLIBPATH) {
    LIBS += -L$$ZLIBLIBPATH
}
unix: LIBS += -lz
win32: LIBS += -lzlib
