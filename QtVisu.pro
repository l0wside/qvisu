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


linux {
INCLUDEPATH += /usr/include/qt5/QtCore /usr/include/qt5 /usr/include/qt5/QtNetwork
LIBS += -L/usr/lib/i386-linux-gnu/
# Doorcom w/ PJSUA
DEFINES += DOORCOM
INCLUDEPATH += ../sip/pjproject-2.4/pjsip/include ../sip/pjproject-2.4/pjlib/include ../sip/pjproject-2.4/pjlib-util/include ../sip/pjproject-2.4/pjmedia/include ../sip/pjproject-2.4/pjnath/include
LIBS += -L../sip/pjproject-2.4/pjlib/lib -L../sip/pjproject-2.4/pjlib-util/lib -L../sip/pjproject-2.4/pjnath/lib -L../sip/pjproject-2.4/pjmedia/lib -L../sip/pjproject-2.4/pjsip/lib -L../sip/pjproject-2.4/third_party/lib
LIBS += -lpjsua-i686-pc-linux-gnu -lpjsip-ua-i686-pc-linux-gnu -lpjsip-simple-i686-pc-linux-gnu -lpjsip-i686-pc-linux-gnu -lpjmedia-codec-i686-pc-linux-gnu -lpjmedia-i686-pc-linux-gnu -lpjmedia-videodev-i686-pc-linux-gnu -lpjmedia-audiodev-i686-pc-linux-gnu -lpjmedia-i686-pc-linux-gnu -lpjnath-i686-pc-linux-gnu -lpjlib-util-i686-pc-linux-gnu  -lsrtp-i686-pc-linux-gnu -lresample-i686-pc-linux-gnu -lgsmcodec-i686-pc-linux-gnu -lspeex-i686-pc-linux-gnu -lilbccodec-i686-pc-linux-gnu -lg7221codec-i686-pc-linux-gnu -lportaudio-i686-pc-linux-gnu  -lpj-i686-pc-linux-gnu -lm -lrt -lpthread  -lasound
}

SOURCES += main.cpp\
    qvelement.cpp \
    qvitem.cpp \
    qvsvgwidget.cpp \
    qvdriver.cpp \
    qvwebsocket.cpp \
    qvmainwindow.cpp \
    qvpopupframe.cpp \
    qvswitchicon.cpp \
    qvdeployserver.cpp \
    elements/qvshutter.cpp \
    elements/qvswitch.cpp \
    elements/qvdimmer.cpp \
    elements/qvefa.cpp \
    elements/qvcolor.cpp \
    elements/qvplot.cpp \
    elements/qvheating.cpp \
    elements/qvselector.cpp \
    elements/qvfritz.cpp \
    elements/qvweather.cpp \
    elements/qvgooglecalendar.cpp \
    qviconwidget.cpp

linux {
SOURCES += elements/qvdoorcom.cpp
}
use_video {
    SOURCES += elements/qvvideo.cpp \
               elements/qvvideoplayer.cpp
}

HEADERS  += \
    qvelement.h \
    qvitem.h \
    qvsvgwidget.h \
    qvdriver.h \
    qvwebsocket.h \
    qvdeployserver.h \
    qvmainwindow.h \
    qvpopupframe.h \
    qvswitchicon.h \
    qviconwidget.h \ 
    elements/qvcolor.h \
    elements/qvshutter.h \
    elements/qvswitch.h \
    elements/qvdimmer.h \
    elements/qvplot.h \
    elements/qvheating.h \
    elements/qvselector.h \
    elements/qvweather.h \
    elements/qvfritz.h \
    elements/qvgooglecalendar.h \
    elements/qvefa.h

use_video {
    HEADERS += elements/qvvideo.h \
               elements/qvvideoplayer.h
}

linux {
HEADERS += elements/qvdoorcom.h
}

FORMS    +=

RESOURCES += \
    qvicons.qrc

use_video {
    DEFINES += VIDEO
}

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

OTHER_FILES += \
    android/AndroidManifest.xml
