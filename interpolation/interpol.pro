TEMPLATE = lib
VERSION = 0.8
TARGET = interpol
DESTDIR = lib
OBJECTS_DIR = obj

USER_LIBS_DIR = ..
SYS_LIBS_DIR = ../../sys-libs

QMAKE_CXXFLAGS += -std=c++0x

win32 {
	DEFINES += _LC_DLL_
}

# add new header files here
HEADERS += interpol.h \
    db.h

# add new source files here
SOURCES += interpol.cpp \
    db.cpp

LIBS += -lm \
    -lmysqlclient \
    -L../lib \
    -lgrid

CONFIG += debug
CONFIG -= qt
#CONFIG += staticlib

INCLUDEPATH += \
	. \
    ..\
    ../include/grid \
    $${USER_LIBS_DIR}/include \
	$${USER_LIBS_DIR}/interpolation \
    $${USER_LIBS_DIR}/include/grid \
    $${SYS_LIBS_DIR}/hdf5-1.8.7-include

headers.files = $$HEADERS
headers.path = ../include/interpolation
target.path = ../lib

INSTALLS += headers target
