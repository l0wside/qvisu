#-------------------------------------------------
#
# Project created by QtCreator 2015-05-18T14:40:55
#
#-------------------------------------------------

QT       += core gui svg xml network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtVisu
TEMPLATE = app

# Uncomment this line to use the experimental VLC code
#CONFIG += use_video

use_video {
	INCLUDEPATH += vlc/include

	LIBS += ../qvisu/vlc/lib/libvlc.lib
}

SOURCES += main.cpp\
    qvelement.cpp \
    qvitem.cpp \
    qvsvgwidget.cpp \
    qvdriver.cpp \
    qvwebsocket.cpp \
    elements/qvshutter.cpp \
    elements/qvswitch.cpp \
    qvmainwindow.cpp \
    qvpopupframe.cpp \
    qvswitchicon.cpp \
    elements/qvdimmer.cpp \
    elements/qvplot.cpp \
    elements/qvheating.cpp \
    elements/qvselector.cpp \
    elements/qvfritz.cpp \
    elements/qvweather.cpp \
    elements/qvgooglecalendar.cpp \
    qviconwidget.cpp

use_video {
    SOURCES += elements/qvvideo.cpp \
               elements/qvvideoplayer.cpp
}
    

HEADERS  += \
    qvelement.h \
    qvswitch.h \
    qvitem.h \
    qvsvgwidget.h \
    qvdriver.h \
    qvwebsocket.h \
    qvshutter.h \
    elements/qvshutter.h \
    elements/qvswitch.h \
    qvmainwindow.h \
    qvpopupframe.h \
    qvswitchicon.h \
    elements/qvdimmer.h \
    elements/qvplot.h \
    elements/qvheating.h \
    elements/qvselector.h \
    elements/qvweather.h \
    elements/qvfritz.h \
    elements/qvgooglecalendar.h \
    qviconwidget.h 

use_video {
    HEADERS += elements/qvvideo.h \
               elements/qvvideoplayer.h
}

FORMS    +=

RESOURCES += \
    qvicons.qrc

use_video {
    DEFINES += VIDEO
}
