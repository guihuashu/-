#include "ffmopr.h"


void FFmOpr::initFFM()
{
    avcodec_register_all();
    av_register_all();
    avformat_network_init();
    qInfo()<<avformat_configuration();
}

bool FFmOpr::allocVFrame(AVFrame **frame, int w, int h, AVPixelFormat fmt)
{
    int align;

    if (!frame)
        return false;
    if (fmt == AV_PIX_FMT_YUV420P)
        align = 32;
    else if (fmt == AV_PIX_FMT_RGB24)
        align = 24;
    else if (fmt == AV_PIX_FMT_BGR24)
        align = 24;
    else  {
        CUR;
        qWarning()<<"unSupport allocFrame fmt";
        return false;
    }

    *frame = av_frame_alloc();
    (*frame)->width = w;
    (*frame)->height = h;
    (*frame)->format = fmt;
    (*frame)->pts = 0;
    if (av_frame_get_buffer((*frame), align))	{
        av_frame_free(frame);
        qWarning()<<"av_frame_get_buffer err";
        return false;
    }
    return true;
}

bool FFmOpr::allocAFrame(AVFrame **frame, int channels, int nb_samples, AVSampleFormat fmt)
{
    if (fmt != AV_SAMPLE_FMT_FLTP) {
        CUR;
        qWarning()<<"ERR: fmt";
        return false;
    }
   // CUR;
    *frame = av_frame_alloc();
    (*frame)->format = fmt;
    (*frame)->channels = channels;
    (*frame)->channel_layout = av_get_default_channel_layout(channels);
    (*frame)->nb_samples = nb_samples;
    (*frame)->pts = 0;
    if (av_frame_get_buffer((*frame), 0))	{
        av_frame_free(frame);
        //CUR;
        qWarning()<<"av_frame_get_buffer err";
        return false;
    }
    //CUR;
    return true;
}



void FFmOpr::freeFrame(AVFrame **frame)
{
    if (!frame)
        return;
    if (*frame)
        av_frame_unref(*frame);
    av_frame_free(frame);
    frame = NULL;
}
void FFmOpr::freePkt(AVPacket **pkt)
{
    if (!pkt)
        return;
    if (*pkt)
        av_packet_unref(*pkt);
    av_packet_free(pkt);
    pkt = NULL;
}
int FFmOpr::Nv21toYuv420p(SwsContext *SwsNv21toYuv420p, QVideoFrame &frame, AVFrame *yuv420p)
{
    int ret;
    // 准备格式转换
    uint8_t *indata[AV_NUM_DATA_POINTERS] = { 0 };	// 数据源(R.G.B./Y.U.V)
    int	insize[AV_NUM_DATA_POINTERS] = { 0 };		// 数据源(R.G.B./Y.U.V)
    int i=0;

    if (!SwsNv21toYuv420p) {
        qWarning()<<"SwsNv21toYuv420p=NULL";
        return -1;
    }
    if (frame.pixelFormat() != QVideoFrame::Format_NV21) {
        qWarning()<<"only support Format_NV21";
        return -1;
    }
    // Format_NV21是平面存取格式, 有多个平面
    while (frame.bits(i)) {
        indata[i] = (unsigned char *)frame.bits(i);
        insize[i] = frame.bytesPerLine(i);
        i++;
    }
    ret = sws_scale(SwsNv21toYuv420p, indata, insize, 0, frame.height(), yuv420p->data, yuv420p->linesize);
    return ret;
}

int FFmOpr::NV21toRgb24(SwsContext *SwsNv21toRgb24, QVideoFrame &frame, AVFrame *rgb24)
{
    int ret;
    int i=0;
    // 准备格式转换
    uint8_t *indata[AV_NUM_DATA_POINTERS] = { 0 };	// 数据源(R.G.B./Y.U.V)
    int	insize[AV_NUM_DATA_POINTERS] = { 0 };		// 数据源(R.G.B./Y.U.V)
    uint8_t **outdata;			// uint8_t*[AV_NUM_DATA_POINTERS]数组
    int *outsize;				// int[AV_NUM_DATA_POINTERS]数组

    // Format_NV21是平面存取格式, 有多个平面
    while (frame.bits(i)) {
        indata[i] = (unsigned char *)frame.bits(i);
        insize[i] = frame.bytesPerLine(i);
        i++;
    }
    outdata = rgb24->data;						// 转换后帧的存储位置
    outsize = rgb24->linesize;					// 由系统内部进行计算和填充
    if (!SwsNv21toRgb24) {
        qWarning()<<"SwsNv21toYuv420p=NULL";
        return -1;
    }

    ret= sws_scale(SwsNv21toRgb24, indata, insize, 0, frame.height(), outdata, outsize);
    return ret;
}

int FFmOpr::NV21toBgr24(SwsContext *sws, QVideoFrame &frame, AVFrame *bgr24)
{
    int ret;
    int i=0;
    // 准备格式转换
    uint8_t *indata[AV_NUM_DATA_POINTERS] = { 0 };	// 数据源(R.G.B./Y.U.V)
    int	insize[AV_NUM_DATA_POINTERS] = { 0 };		// 数据源(R.G.B./Y.U.V)
    uint8_t **outdata;			// uint8_t*[AV_NUM_DATA_POINTERS]数组
    int *outsize;				// int[AV_NUM_DATA_POINTERS]数组

    // Format_NV21是平面存取格式, 有多个平面
    while (frame.bits(i)) {
        indata[i] = (unsigned char *)frame.bits(i);
        insize[i] = frame.bytesPerLine(i);
        i++;
    }
    outdata = bgr24->data;						// 转换后帧的存储位置
    outsize = bgr24->linesize;					// 由系统内部进行计算和填充
    if (!sws) {
        qWarning()<<"SwsNv21toYuv420p=NULL";
        return -1;
    }

    ret= sws_scale(sws, indata, insize, 0, frame.height(), outdata, outsize);
    return ret;
}

int FFmOpr::Rrgb24toYuv420P(SwsContext *sws, AVFrame *rgb24, AVFrame *yuv420p)
{
    int ret= sws_scale(sws, rgb24->data, rgb24->linesize, 0, rgb24->height, yuv420p->data, yuv420p->linesize);
    return ret;
}

int FFmOpr::Bgrb24toYuv420P(SwsContext *sws, AVFrame *brg24, AVFrame *yuv420p)
{
    int ret= sws_scale(sws, brg24->data, brg24->linesize, 0, brg24->height, yuv420p->data, yuv420p->linesize);
    return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//重采样源S16数据, 存放到pcm中
int FFmOpr::S16toFltp(SwrContext *swrS16toFltp, AudioFrame *s16, AVFrame *fltp)
{
    const uint8_t *indata[AV_NUM_DATA_POINTERS] = { 0 };
    indata[0] = (uint8_t *)s16->data;
    // s16是交叉存取模型

    if (!swrS16toFltp) {
        return -1;
    }
//    CUR;
//    qInfo()<<swrS16toFltp;
//    qInfo()<<fltp->data;
//    qInfo()<<fltp->nb_samples;
//    qInfo()<<s16->nb_samples;
//    qInfo()<<(uint8_t *)s16->data;

    int len = swr_convert(swrS16toFltp, fltp->data, fltp->nb_samples, //输出参数，输出存储地址和样本数量
        indata, s16->nb_samples);
    return len;
}


