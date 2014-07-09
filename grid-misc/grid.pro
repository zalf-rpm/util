TEMPLATE = lib
VERSION =
TARGET = grid
DESTDIR = lib
OBJECTS_DIR = obj

USER_LIBS_DIR = ..
SYS_LIBS_DIR = ../../sys-libs

QMAKE_CXXFLAGS += -std=c++0x

isEmpty(ARCH){
ARCH = x64
}

message("grid: building code for " $$ARCH " architecture")

#../import-export.h \
HEADERS += \
grid.h \
platform.h \
grid+.h \
grid-manager.h \
types.h

SOURCES += \
grid.cpp \
platform.cpp \
feldw.cpp \
grid+.cpp \
grid-manager.cpp

#config
#------------------------------------------------------------

#CONFIG += debug
win32:CONFIG += staticlib
CONFIG -= qt

#includes
#-------------------------------------------------------------

INCLUDEPATH += \
#. \
.. \
$${SYS_LIBS_DIR}/boost-1.39.0 \
$${SYS_LIBS_DIR}/loki-lib/include \
$${SYS_LIBS_DIR}/sqlite-amalgamation-3070603 \
$${SYS_LIBS_DIR}/proj-4.7.0/src \
$${SYS_LIBS_DIR}/hdf5-1.8.7-include

win32:INCLUDEPATH += \
$${SYS_LIBS_DIR}/windows/mysql/include \
$${SYS_LIBS_DIR}/dirent-1.11

#libs
#------------------------------------------------------------

LIBS += \
-L$${USER_LIBS_DIR}/lib \
-ltools -ldb

win32 {
LIBS += \
-L$${SYS_LIBS_DIR}/windows/$${ARCH} \
-lproj \
-lhdf5dll

CONFIG(debug, debug|release):LIBS += \
-llibmysqld
CONFIG(release, debug|release):LIBS += \
-llibmysql

CONFIG(debug, debug|release):QMAKE_LFLAGS += \
/NODEFAULTLIB:msvcrt.lib #\
#/VERBOSE:lib

CONFIG(release, debug|release):QMAKE_LFLAGS += \
/NODEFAULTLIB:msvcrtd.lib
}

unix {
LIBS += \
-lm \
-lpthread \
-L$${SYS_LIBS_DIR}/linux/hdf5-1.8.7/$${ARCH}/lib \
-lhdf5 \
-lmysqlclient \
-L$${SYS_LIBS_DIR}/lib \
-lproj
}

#install
#-------------------------------------------------------------

headers.files = $$HEADERS
headers.path = ../include/grid
target.path = ../lib

INSTALLS += headers target
