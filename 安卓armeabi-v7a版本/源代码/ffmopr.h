#ifndef FFMOPR_H
#define FFMOPR_H
#include <QDebug>
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

class FFmOpr{
public:
    static void initFFM();
    static bool allocYuv420p(AVFrame **yuv420p);
    static void freePkt(AVPacket **pkt);
    static void freeFrame(AVFrame **frame);
public:
    SwsContext *SwsNv21toYuv420p = NULL;

};

#endif // FFMOPR_H
