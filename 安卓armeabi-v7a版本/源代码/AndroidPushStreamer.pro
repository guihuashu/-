#-------------------------------------------------
#
# Project created by QtCreator 2019-07-29T19:40:47
#
#-------------------------------------------------

QT       += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AndroidPushStreamer
TEMPLATE = app

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
    AudioRecord.h \
    qtvideocap.h \
    videoAudioTst.h \
    OutMedia.h \
    ffmopr.h
#    ffm_audioTst.h \
#    OutMedia.h \
#    videoAudioTst.h \
#    showwidget.h

SOURCES += \
    main.cpp \
    MediaEncode.cpp \
    AudioRecord.cpp \
    qtvideocap.cpp \
    videoAudioTst.cpp \
    OutMedia.cpp \
    ffmopr.cpp
#    ffm_audioTst.cpp \
#    ffm_videoTst.cpp \
#    OutMedia.cpp \
#    videoAudioTst.cpp \
#    showwidget.cpp

#FORMS += \
#    showwidget.ui

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_EXTRA_LIBS = \
        D:/VST___/src/AndroidPushStreamer/../../lib/armeabi-v7a/libavcodec.so \
        D:/VST___/src/AndroidPushStreamer/../../lib/armeabi-v7a/libavfilter.so \
        D:/VST___/src/AndroidPushStreamer/../../lib/armeabi-v7a/libavformat.so \
        D:/VST___/src/AndroidPushStreamer/../../lib/armeabi-v7a/libavutil.so \
        D:/VST___/src/AndroidPushStreamer/../../lib/armeabi-v7a/libswresample.so \
        D:/VST___/src/AndroidPushStreamer/../../lib/armeabi-v7a/libswscale.so \
        $$PWD/../../lib/armeabi-v7a/libfdk-aac.so
}
