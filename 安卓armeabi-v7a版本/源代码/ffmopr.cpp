#include "ffmopr.h"


static void FFmOpr::initFFM()
{
    avcodec_register_all();
    av_register_all();
    avformat_network_init();
    qInfo()<<avformat_configuration();
}
static bool FFmOpr::allocYuv420p(AVFrame **yuv420p)
{
    (*yuv420p) = av_frame_alloc();
    (*yuv420p)->format = AV_PIX_FMT_YUV420P;
    (*yuv420p)->width = _vArgs.outWidth;
    (*yuv420p)->height = _vArgs.outHeight;
    (*yuv420p)->pts = 0;
    if (av_frame_get_buffer((*yuv420p), 32))	{   // 32字节对其
        qWarning()<<"av_frame_get_buffer err";
        return false;
    }
    return true;
}

static void FFmOpr::freeFrame(AVFrame **frame)
{
    if (!frame)
        return;
    if (*frame)
        av_frame_unref(*frame);
    av_frame_free(frame);
    frame = NULL;
}
static void FFmOpr::freePkt(AVPacket **pkt)
{
    if (!pkt)
        return;
    if (*pkt)
        av_packet_unref(*pkt);
    av_packet_free(pkt);
    pkt = NULL;
}
