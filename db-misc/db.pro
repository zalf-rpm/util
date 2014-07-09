TEMPLATE = lib
VERSION = 
TARGET = db
DESTDIR = lib
OBJECTS_DIR = obj

QMAKE_CXXFLAGS += -std=c++0x

USER_LIBS_DIR = ..
SYS_LIBS_DIR = ../../sys-libs

isEmpty(ARCH){
ARCH = x64
}

message("db: building code for " $$ARCH " architecture")

HEADERS += \
../import-export.h \
db.h \
abstract-db-connections.h \
$${SYS_LIBS_DIR}/sqlite-amalgamation-3070603/sqlite3.h

SOURCES += \
db.cpp \
abstract-db-connections.cpp \
$${SYS_LIBS_DIR}/sqlite-amalgamation-3070603/sqlite3.c

#config
#------------------------------------------------------------

#CONFIG += debug
win32:CONFIG += staticlib #create_prl
#CONFIG += link_prl
CONFIG -= qt

#includes
#-------------------------------------------------------------

INCLUDEPATH += \
$${USER_LIBS_DIR}/include \
$${SYS_LIBS_DIR}/boost-1.39.0 \
$${SYS_LIBS_DIR}/loki-lib/include \
$${SYS_LIBS_DIR}/sqlite-amalgamation-3070603

win32:INCLUDEPATH += \
$${SYS_LIBS_DIR}/windows/mysql/include \
$${SYS_LIBS_DIR}/dirent-1.11

#libs
#------------------------------------------------------------

LIBS += \
-L$${USER_LIBS_DIR}/lib \
-ltools

win32 {
LIBS += \
-L$${SYS_LIBS_DIR}/windows/$${ARCH}

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
-lmysqlclient
}

#install
#-------------------------------------------------------------

headers.files = $$HEADERS
headers.path = ../include/db
target.path = ../lib

INSTALLS += headers target

