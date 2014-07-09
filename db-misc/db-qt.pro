TEMPLATE = lib
VERSION = 0.3
TARGET = qt-db
DESTDIR = lib
OBJECTS_DIR = obj

HEADERS += \
	db.h \
	abstract-db-connections.h \	
	db-qt.h

SOURCES += \
	db.cpp \
	abstract-db-connections.cpp \
	db-qt.cpp

LIBS += \
	-lmysqlclient \
	-L../lib \
	-ltools 

CONFIG += QT
CONFIG += debug

INCLUDEPATH += \ 
	../include 

headers.files = $$HEADERS
headers.path = ../include/db
target.path = ../lib

INSTALLS += headers target

