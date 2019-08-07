#ifndef FFMOPR_H
#define FFMOPR_H
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavcodec/mediacodec.h>
    #include <libavutil/avutil.h>
    #include <libavutil/opt.h>
    #include <libavutil/dict.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
    #include <libavutil/time.h>
}

#include <QDebug>
#include <QVideoFrame>
#include <iostream>
#include <mutex>
#include <debug.h>

class FFmOpr{
public:
    static void initFFM();
    static bool allocFrame(AVFrame **frame, int w, int h, AVPixelFormat fmt);
    static void freePkt(AVPacket **pkt);
    static void freeFrame(AVFrame **frame);
    static int Nv21toYuv420p(SwsContext *SwsNv21toYuv420p, QVideoFrame &frame, AVFrame *yuv420p);
    static int NV21toRgb24(SwsContext *SwsNv21toRgb24, QVideoFrame &frame, AVFrame *rgb24);
    static int NV21toBgr24(SwsContext *sws, QVideoFrame &frame, AVFrame *bgr24);
    static int Rrgb24toYuv420P(SwsContext *sws, AVFrame *rgb24, AVFrame *yuv420p);
    static int Bgrb24toYuv420P(SwsContext *sws, AVFrame *rgb24, AVFrame *yuv420p);
};






#endif // FFMOPR_H
