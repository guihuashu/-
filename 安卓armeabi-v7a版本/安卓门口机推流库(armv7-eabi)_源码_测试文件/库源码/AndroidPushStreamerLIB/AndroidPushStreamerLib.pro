#-------------------------------------------------
#
# Project created by QtCreator 2019-07-29T19:40:47
#
#-------------------------------------------------

#QT       += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AndroidPushStreamerLib
#TEMPLATE = app
TEMPLATE = lib
CONFIG += staticlib



# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

CONFIG += mobility
MOBILITY = 



INCLUDEPATH += $$PWD/../../include
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


#win32: LIBPATH=-L$$PWD/../../lib/win64/
unix:!macx: LIBPATH=-L$$PWD/../../lib/armeabi-v7a/

LIBS += $${LIBPATH} \
    -lavformat \
    -lavcodec \
    -lavutil \
    -lswresample \
    -lswscale

INCLUDEPATH += $$PWD/../../lib/armeabi-v7a
DEPENDPATH += $$PWD/../../lib/armeabi-v7a
#INCLUDEPATH += $$PWD/../../include
#DEPENDPATH += $$PWD/../../include



HEADERS += \
    MediaEncode.h \
    qtvideocap.h \
    OutMedia.h \
    ffmopr.h \
    task.h \
    pktlist.h \
    control.h \
    debug.h \
    showvcap.h \
    qtaudiorecord.h \
    frameList.h \
    call.h

SOURCES += \
    main.cpp \
    MediaEncode.cpp \
    qtvideocap.cpp \
    OutMedia.cpp \
    ffmopr.cpp \
    task.cpp \
    pktlist.cpp \
    control.cpp \
    showvcap.cpp \
    qtaudiorecord.cpp \
    frameList.cpp \
    call.cpp


FORMS += \
    showvcap.ui

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_EXTRA_LIBS = \
        $$PWD/../../lib/armeabi-v7a/libavcodec.so \
        $$PWD/../../lib/armeabi-v7a/libavdevice.so \
        $$PWD/../../lib/armeabi-v7a/libavfilter.so \
        $$PWD/../../lib/armeabi-v7a/libavformat.so \
        $$PWD/../../lib/armeabi-v7a/libavutil.so \
        $$PWD/../../lib/armeabi-v7a/libpostproc.so \
        $$PWD/../../lib/armeabi-v7a/libswresample.so \
        $$PWD/../../lib/armeabi-v7a/libswscale.so
}




