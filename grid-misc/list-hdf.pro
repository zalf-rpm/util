TEMPLATE = app
VERSION = 1.0
TARGET = list-hdf
DESTDIR = .
OBJECTS_DIR = obj

HEADERS += \
	grid.h \
	platform.h \

SOURCES += \
	grid.cpp \
	platform.cpp \
	feldw.cpp \
  list-hdf-main.cpp

LIBS += \
	-lm \
	-L../../sys-libs/lib \
	-lfftw3 \	 
  -lproj \
	-lm -lgsl -lgslcblas \
	-lhdf5 \
	-L../lib \
	-ltools

CONFIG += debug

INCLUDEPATH += \
	. \
	../include \
	../../sys-libs/include \
	../../sys-libs/boost-1.39.0 \
  ../../sys-libs/loki-0.1.7/include
