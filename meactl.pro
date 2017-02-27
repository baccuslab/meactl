######################################################################
# Automatically generated by qmake (3.1) Sun Feb 19 13:20:32 2017
######################################################################

TEMPLATE = app
TARGET = meactl
INCLUDEPATH += . \
	include \
	../ \
	../libblds-client/include \
	/usr/local/include

QT += gui widgets network 
CONFIG += c++11 debug_and_release
CONFIG -= app_bundle

LIBS += -L../libblds-client/lib -L/usr/local/lib 

QMAKE_RPATHDIR += ../libblds-client/lib \
	../libdata-source/lib \
	../libdatafile/lib

win32 {
	LIBS += -lblds-client0
} else {
	LIBS += -lblds-client
}

LIBS += -lhdf5_cpp -lhdf5

# Input
HEADERS += include/meactl-window.h \
		include/source-settings-window.h \
		include/meactl-widget.h
SOURCES += src/meactl-window.cc \
		src/source-settings-window.cc \
		src/meactl-widget.cc \ 
		src/main.cc
