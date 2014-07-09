TEMPLATE = lib
VERSION =
TARGET = climate-with-realization
DESTDIR = lib
OBJECTS_DIR = obj

QMAKE_CXXFLAGS += -std=c++0x

USER_LIBS_DIR = ..
SYS_LIBS_DIR = ../../sys-libs

HEADERS += \
	climate.h \
  regionalization.h

SOURCES += \
	climate.cpp \
  regionalization.cpp

#config
#------------------------------------------------------------

#CONFIG += debug
#CONFIG += staticlib #create_prl
#CONFIG += link_prl
CONFIG -= qt

#includes
#-------------------------------------------------------------

INCLUDEPATH += \
. \
.. \
$${SYS_LIBS_DIR}/include \
$${SYS_LIBS_DIR}/boost-1.39.0 \
$${SYS_LIBS_DIR}/loki-lib/include \
$${SYS_LIBS_DIR}/sqlite-amalgamation-3070603

win32:INCLUDEPATH += \
$${SYS_LIBS_DIR}/windows/mysql/x86/include \
$${SYS_LIBS_DIR}/windows/hdf5/x86/1.8.7/include

#libs
#------------------------------------------------------------

#win32:QMAKE_CXXFLAGS += /MT

LIBS += \
-L$${USER_LIBS_DIR}/lib \
-ldb -ltools -lgrid \
-L$${SYS_LIBS_DIR}/lib \
-lproj

win32 {
LIBS += \
-L$${SYS_LIBS_DIR}/windows/hdf5/x86/1.8.7/bin \
-lhdf5dll

CONFIG(debug, debug|release):LIBS += \
-L$${SYS_LIBS_DIR}/windows/mysql/x86/lib/debug

CONFIG(release, debug|release):LIBS += \
-L$${SYS_LIBS_DIR}/windows/mysql/x86/lib/opt

LIBS += \
-llibmysql
}

unix:LIBS += \
-lhdf5 \
-lmysqlclient

#win32 {
#CONFIG(debug, debug|release):QMAKE_LFLAGS += \
#/NODEFAULTLIB:msvcrt.lib #\
#/VERBOSE:lib

#CONFIG(release, debug|release):QMAKE_LFLAGS += \
#/NODEFAULTLIB:msvcrtd.lib
#}

#install
#-------------------------------------------------------------

headers.files = $$HEADERS
headers.path = ../include/climate
target.path = ../lib

INSTALLS += headers target
