#include "MediaEncode.h"
#include <iostream>
using namespace std;


MediaEncode::MediaEncode(vEncodeArgs &vArgs, aEncodeArgs &aArgs)
{
    qInfo()<< "---------------  MediaEncode ----------------" << endl;
    _vArgs = vArgs;
    _aArgs = aArgs;
    _sysPts = av_gettime();
    _sysTimebase = { 1, 1000000 };	// 系统时间基数	(时间秒数 = 时间戳 * 时间基数)

    if (!init_vEncode() || !init_aEncode()){
        qWarning()<<"ERR: MediaEncode init";
        getchar();
    }
    qInfo()<<"OK: MediaEncode init";
}

MediaEncode::~MediaEncode()
{
    //avcodec_free_context(&vEncodeCtx);
    //av_frame_free(&yuv420p);
    //avcodec_close(vEncodeCtx);
}


bool MediaEncode::init_vEncode()
{
    /* 1.初始化视频编码器 */
    AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        qWarning()<<"ERR: avcodec_find_encoder(AV_CODEC_ID_H264)";
        return false;
    }
    _vEncodeCtx = avcodec_alloc_context3(codec);
    if (!_vEncodeCtx) {
        qWarning()<<"ERR: avcodec_alloc_context3";
        return false;
    }

    _vEncodeCtx->codec_id = codec->id;					// h264
    _vEncodeCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;	 // 全局标记
    _vEncodeCtx->thread_count = _vArgs.thread_count;		// 用于编码的线程数
    _vEncodeCtx->bit_rate =  _vArgs.bit_rate;				// 码率, 越大视频越清晰
    _vEncodeCtx->width = _vArgs.outWidth;                           // 输出图像的宽度 = 输入的宽度
    _vEncodeCtx->height = _vArgs.outHeight;                          // 输出图像的高度
    _vEncodeCtx->time_base = { 1,1000000 };                 // 时间基数,微秒
    _vEncodeCtx->framerate = { _vArgs.fps, 1 };                      // fps
    _vEncodeCtx->gop_size = _vArgs.gop_size;                             // 关键帧周期
    _vEncodeCtx->max_b_frames = _vArgs.max_b_frames;                          // 最大b帧数, 有b帧就会有延迟
    _vEncodeCtx->pix_fmt = _vArgs.out_pixFmt;				// 输入帧的像素格式
    // 新增
    _vEncodeCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    _vEncodeCtx->global_quality = 1;

    // 优化推流延迟, 设置编码器的的私有属性来优化推流延迟
    _vEncodeCtx->delay = 0;
    CUR;
    // 设置太快,编码器吃不消, 会有马赛克
    av_opt_set(_vEncodeCtx->priv_data, "preset ", "ultrafast ", 0); //设置priv_data的option
    av_opt_set(_vEncodeCtx->priv_data, "tune", "zerolatency", 0); //设置priv_data的option
    av_opt_set(_vEncodeCtx->priv_data, "x264opts", _vArgs.CRF.c_str(), 0);  // crf:0-51, 0表示不压缩, 51表示最高压缩比
    // 安卓上设置这个会报错
    //av_opt_set(_vEncodeCtx->priv_data, "x264opts", "sync-lookahead=0: sliced-threads", 0);
    if (avcodec_open2(_vEncodeCtx, NULL, NULL))	{       // 打开编码器(编码器开始待解码)
        qWarning()<<"ERR: avcodec_open2(_vEncodeCtx, 0, 0)";
        return false;
    }
    qInfo()<<"avcodec_open2(_vEncodeCtx, NULL, NULL) OK";
    return true;
}
bool MediaEncode::init_aEncode()
{
    // 1.创建音频编解码器上下文
    int ret;
    int aBit_rate = _aArgs.bit_rate;
    int thread_count = _aArgs.thread_count;
    int sampleRate = _aArgs.sample_rate;
    int channels =  _aArgs.channels;

    enum AVSampleFormat sampleFmt = _aArgs.sample_fmt;
    enum AVSampleFormat resampleFmt = _aArgs.resample_fmt;




    AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!codec) {
        qWarning() << "ERR: avcodec_find_encoder(AV_CODEC_ID_AAC) !";
        return false;
    }
    qInfo() << "avcodec_find_encoder(AV_CODEC_ID_AAC) OK";
    //音频编码器上下文
    _aEncodeCtx = avcodec_alloc_context3(codec);
    if (!_aEncodeCtx) {
        qWarning() << "ERR: avcodec_alloc_context3: AV_CODEC_ID_AAC!";
        return false;
    }
    _aEncodeCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    _aEncodeCtx->thread_count = thread_count;
    _aEncodeCtx->bit_rate = aBit_rate;

    /* 注意: 编码器时间戳 = 1 / 采样率 = 1 / 44100	*/
    _aEncodeCtx->sample_rate = sampleRate;	// 44100
    _aEncodeCtx->sample_fmt = resampleFmt;
    _aEncodeCtx->channels = channels;
    _aEncodeCtx->channel_layout = av_get_default_channel_layout(channels);

    // 音频编码速度优化
    _aEncodeCtx->profile = FF_PROFILE_MPEG2_AAC_HE;

    //打开音频编码器
    if(avcodec_open2(_aEncodeCtx, 0, 0) < 0) {
        qWarning()<<"ERR: avcodec_open2(aEncodeCtx, 0, 0)";
        return false;
    }
    qInfo() << "avcodec_open2(_aEncodeCtx) success!";


    /* 2.创建重采样上下文 */
    _swrS16toFltp = swr_alloc_set_opts(_swrS16toFltp,
        av_get_default_channel_layout(channels), resampleFmt, sampleRate,//输出格式
        av_get_default_channel_layout(channels), sampleFmt, sampleRate,  //输入格式
        0, 0);
    if (!_swrS16toFltp) {
        CUR;
        qWarning()<<"ERR: swr_alloc_set_opts!";
        return false;
    }
    if (swr_init(_swrS16toFltp) < 0) {
        qWarning()<<"ERR: swr_init!";
        return false;
    }
    qInfo()<<"create _swrS16toFltp OK";
    return true;
}


bool MediaEncode::vEncode(AVFrame *yuv420p_, AVPacket *vPkt)
{

    // CPU过一段时间才计时,避免相邻两帧pts相同
    static long long prePts = -1;
    if (prePts == vPkt->pts)
        vPkt->pts += 1000;
    prePts = vPkt->pts;
    //CUR;
    yuv420p_->pts = vPkt->pts;	// 数据帧的采集时间戳
    av_init_packet(vPkt);
    if (!_vEncodeCtx)
        return false;
    vMutex.lock();
    if (avcodec_send_frame(_vEncodeCtx, yuv420p_)) {    // 将YUV420p帧放入h264编码器队列
        vMutex.unlock();
        return false;
    }
    if (avcodec_receive_packet(_vEncodeCtx, vPkt)){	// 从编码器中读取h264编码后的数据包
        vMutex.unlock();
        return false;
    }
    vMutex.unlock();
    if (vPkt->size <= 0)
        return false;
    return true;
}

/////////////////////////////////////////////////////////
bool MediaEncode::aEncode(AVFrame *fltp, AVPacket *aPkt)
{

#if 0	// 这是基于采样率计算的pts

    fltp->pts = aPts;

	// 编码时间戳 = 一帧音频的秒数sec / 编码时间基数
    //apts += (fltp->nb_samples / sampleRate) / (ac->time_base.num / ac->time_base.den);
    aPts += av_rescale_q(fltp->nb_samples, { 1,sampleRate }, aEncodeCtx->time_base);
#endif	
	// CPU过一段时间才计时,避免相邻两帧pts相同
	static long long prePts = -1;
	if (prePts == aPkt->pts)
        aPkt->pts += 1000;  // 单位为微秒
	prePts = aPkt->pts;
    fltp->pts = aPkt->pts;

    av_init_packet(aPkt);
	// 将一帧音频放入编码器队列
    _aEncodeCtxMutex.lock();
    if (avcodec_send_frame(_aEncodeCtx, fltp)) {
        _aEncodeCtxMutex.unlock();
        return false;
    }
	// 从编码器队列中取出已经编码的帧
    if (avcodec_receive_packet(_aEncodeCtx, aPkt)){
        _aEncodeCtxMutex.unlock();
        return false;
    }
    _aEncodeCtxMutex.unlock();
    return true;
}



#if 0

///////////////////////////////////////////////////////////////
AVPacket *MediaEncode::gain_aPkt()
{
#if 0
	while (aPktList.size() <= 0) {
		av_usleep(50);
	}
#endif
    if (_aPktList.size() <= 0) {
		return NULL;
	}
    _aList_mutex.lock();
    AVPacket *pkt = _aPktList.back();	// 取出链表最后一项
    _aPktList.pop_back();				// 删除链表最后一项
    _aList_mutex.unlock();
	return pkt;
}


void make_aPkt(MediaEncode *This)
{
#if 0
    AudioRecord audioRecord;
    AVFrame *pcm_;

    audioRecord.init_record();
    char *buf = new char[This->aFrameBufSize];
    list<AVPacket *> *aPktList = &This->_aPktList;
    int ListSize = This->_aListSize;
    aPktList->clear();
    while (1)
    {
        AVPacket *pkt = av_packet_alloc();
        av_init_packet(pkt);

        if (!pkt)
            continue;
        memset(buf, 0, This->aFrameBufSize);

        audioRecord.get_audio(buf, This->aFrameBufSize);
        if (!(pcm_ = This->fmtS16_to_fmtFltp(buf))) {// 音频重采样
            av_packet_unref(pkt);	// 包中数据的引用计数-1
            av_packet_free(&pkt);	// 释放尾部的包
            pkt = NULL;
            continue;
        }

        pkt->pts = av_gettime() - This->_sysPts;	// 包的时间戳(毫秒) = 当前时间-开始记录的时间


        if (!This->aEncode(pcm_, pkt)) {
            av_packet_unref(pkt);	// 包中数据的引用计数-1
            av_packet_free(&pkt);	// 释放尾部的包
            pkt = NULL;
            continue;
        }
        //av_packet_unref(pkt);	// 包中数据的引用计数-1
        This->_aList_mutex.lock();
        if (aPktList->size() == ListSize) {	// 缓冲区是满的
            aPktList->pop_back();			// 删掉尾部的包
            av_packet_unref(pkt);	// 包中数据的引用计数-1
            av_packet_free(&aPktList->back());	// 释放尾部的包
        }
        aPktList->push_front(pkt);	// 插到链表前面
        This->_aList_mutex.unlock();
    }
 #endif
}
#endif


//////////////////////////////////////////////////////////////////////////






