TEMPLATE = app
VERSION = 0.1
TARGET = regionalization
DESTDIR = .
OBJECTS_DIR = obj

HEADERS += \
	regionalization.h

SOURCES += \
	regionalization.cpp \
  main.cpp

LIBS += \
	-lmysqlclient \ 
	-L../lib \
	-L../../sys-libs/lib \
	-lproj -lhdf5 -lgsl -lgslcblas \
	-ldb -ltools -lclimate -lgrid

CONFIG += debug

INCLUDEPATH += \
	. \
	.. \
	../../sys-libs/include \
	../../sys-libs/boost-1.39.0 \
	../../sys-libs/loki-0.1.7/include

#headers.files = $$HEADERS
#headers.path = ../include/climate
#target.path = ../lib

#INSTALLS += headers target
