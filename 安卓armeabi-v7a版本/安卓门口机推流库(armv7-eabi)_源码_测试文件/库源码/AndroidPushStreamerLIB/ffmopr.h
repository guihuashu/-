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
#include <qtaudiorecord.h>

class FFmOpr{
public:
    static void initFFM();
    static bool allocVFrame(AVFrame **frame, int w, int h, AVPixelFormat fmt);
    static bool allocAFrame(AVFrame **frame, int channels, int nb_samples, AVSampleFormat fmt);
    static void freePkt(AVPacket **pkt);
    static void freeFrame(AVFrame **frame);
    // 视频转码
    static int Nv21toYuv420p(SwsContext *SwsNv21toYuv420p, QVideoFrame &frame, AVFrame *yuv420p);
    static int NV21toRgb24(SwsContext *SwsNv21toRgb24, QVideoFrame &frame, AVFrame *rgb24);
    static int NV21toBgr24(SwsContext *sws, QVideoFrame &frame, AVFrame *bgr24);
    static int Rrgb24toYuv420P(SwsContext *sws, AVFrame *rgb24, AVFrame *yuv420p);
    static int Bgrb24toYuv420P(SwsContext *sws, AVFrame *rgb24, AVFrame *yuv420p);
    // 音频从采样
    static int S16toFltp(SwrContext *swrS16toFltp, AudioFrame *s16, AVFrame *fltp);
};






#endif // FFMOPR_H
