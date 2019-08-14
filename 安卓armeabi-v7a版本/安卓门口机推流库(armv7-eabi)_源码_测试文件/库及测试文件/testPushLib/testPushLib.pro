#QT       += core gui multimedia multimediawidgets

TEMPLATE = app
CONFIG += console c++11
CONFIG -= qt

SOURCES += \
        main.cpp

HEADERS += \
    call.h

INCLUDEPATH += $$PWD/include

# 推流库, ffmpeg库, QT库
unix:!macx: LIBPATH=-L$$PWD/../../lib/armeabi-v7a/
unix:!macx: LIBS += -L$$PWD/./ -lAndroidPushStreamerLib
LIBS += $${LIBPATH} \
    -lavformat \
    -lavcodec \
    -lavutil \
    -lswresample \
    -lswscale

LIBS += $${LIBPATH} \
    -lQt5Core \
    -lQt5Gui \
    -lQt5Multimedia \
    -lQt5MultimediaWidgets \
    -lQt5Widgets




unix:!macx: PRE_TARGETDEPS += $$PWD/lib/armeabi-v7a/libAndroidPushStreamerLib.a
unix:!macx: LIBS += -L$$PWD/lib/armeabi-v7a/ -lAndroidPushStreamerLib
INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.
unix:!macx: LIBPATH=-L$$PWD/lib/armeabi-v7a/

INCLUDEPATH += $$PWD/lib/armeabi-v7a
DEPENDPATH += $$PWD/lib/armeabi-v7a


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

