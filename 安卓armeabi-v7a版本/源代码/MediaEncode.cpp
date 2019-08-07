#include "MediaEncode.h"

using namespace std;


MediaEncode::MediaEncode(vEncodeArgs &vArgs, aEncodeArgs &aArgs)
{
    qInfo()<< "---------------  MediaEncode ----------------" << endl;
    _vArgs = vArgs;
    _aArgs = aArgs;
    _sysPts = av_gettime();
    _sysTimebase = { 1, 1000000 };	// 系统时间基数	(时间秒数 = 时间戳 * 时间基数)

    init_vEncodeCtx();
}

MediaEncode::~MediaEncode()
{
    //avcodec_free_context(&vEncodeCtx);
    //av_frame_free(&yuv420p);
    //avcodec_close(vEncodeCtx);
}


bool MediaEncode::init_vEncodeCtx()
{
    /* 1.初始化视频编码器 */
    AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        qWarning()<<"ERR: avcodec_find_encoder";
        return false;
    }
    _vEncodeCtx = avcodec_alloc_context3(codec);
    if (!_vEncodeCtx) {
        qWarning()<<"ERR: avcodec_alloc_context3";
        return false;
    }
    _vEncodeCtx->codec_id = codec->id;					// h264
    _vEncodeCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;	 // 全局标记
    _vEncodeCtx->thread_count = 6;		// 用于编码的线程数
    _vEncodeCtx->bit_rate =  20 *1024 *8;				// 码率, 越大视频越清晰
    _vEncodeCtx->width = 640;                           // 输出图像的宽度 = 输入的宽度
    _vEncodeCtx->height = 480;                          // 输出图像的高度
    _vEncodeCtx->time_base = { 1,1000000 };                 // 时间基数,微秒
    _vEncodeCtx->framerate = { 30, 1 };                      // fps
    _vEncodeCtx->gop_size = 50;                             // 关键帧周期
    _vEncodeCtx->max_b_frames = 0;                          // 最大b帧数, 有b帧就会有延迟
    _vEncodeCtx->pix_fmt = AV_PIX_FMT_YUV420P;				// 输入帧的像素格式
    // 优化推流延迟, 设置编码器的的私有属性来优化推流延迟
    //_vEncodeCtx->delay = 0;
    CUR;
    // 设置太快,编码器吃不消, 会有马赛克
    //av_opt_set(_vEncodeCtx->priv_data, "fast", "faster", 0); //设置priv_data的option
    //av_opt_set(_vEncodeCtx->priv_data, "tune", "zerolatency", 0); //设置priv_data的option
    av_opt_set(_vEncodeCtx->priv_data, "tune", "fastdecode", 0); //设置priv_data的option
    // 安卓上设置这个会报错
    //av_opt_set(_vEncodeCtx->priv_data, "x264opts", "sync-lookahead=0: sliced-threads", 0);
    if (avcodec_open2(_vEncodeCtx, NULL, NULL))	{       // 打开编码器(编码器开始待解码)
        qWarning()<<"ERR: avcodec_open2(_vEncodeCtx, 0, 0)";
        return false;
    }
    CUR;
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







#if 0
/////////////////////////////////////////////////////////
bool MediaEncode::aEncode(AVFrame *pcm_, AVPacket *aPkt)
{
#if 0	// 这是基于采样率计算的pts

	pcm_->pts = aPts;

	// 编码时间戳 = 一帧音频的秒数sec / 编码时间基数
	//apts += (pcm->nb_samples / sampleRate) / (ac->time_base.num / ac->time_base.den);
	aPts += av_rescale_q(pcm_->nb_samples, { 1,sampleRate }, aEncodeCtx->time_base);
#endif	
	// CPU过一段时间才计时,避免相邻两帧pts相同
	static long long prePts = -1;
	if (prePts == aPkt->pts)
		aPkt->pts += 1000;
	prePts = aPkt->pts;

	pcm_->pts = aPkt->pts;
	// 将一帧音频放入编码器队列
	if (avcodec_send_frame(aEncodeCtx, pcm_))
		return false;

	av_init_packet(aPkt);
	// 从编码器队列中取出已经编码的帧
	if (avcodec_receive_packet(aEncodeCtx, aPkt))
		return false;
	return true;
}

////////////////////////////////////////////////////////////////////
bool MediaEncode::init_aEncode()
{
	// 1.创建音频编解码器上下文
	int ret;
	AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
	if (!codec) {
		cout << "avcodec_find_encoder AV_CODEC_ID_AAC failed!" << endl;
		return false;
	}

	//音频编码器上下文
	aEncodeCtx = avcodec_alloc_context3(codec);
	if (!aEncodeCtx) {
		cout << "avcodec_alloc_context3 AV_CODEC_ID_AAC failed!" << endl;
		return false;
	}

	cout << "avcodec_alloc_context3 success!" << endl;
	aEncodeCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    aEncodeCtx->thread_count = 8;
	aEncodeCtx->bit_rate = aBit_rate;

	/* 注意: 编码器时间戳 = 1 / 采样率 = 1 / 44100	*/
	aEncodeCtx->sample_rate = sampleRate;	// 44100
	aEncodeCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;
	aEncodeCtx->channels = channels;
	aEncodeCtx->channel_layout = av_get_default_channel_layout(channels);

	// 音频延迟优化
	aEncodeCtx->profile = FF_PROFILE_MPEG2_AAC_HE;

	//打开音频编码器
	ret = avcodec_open2(aEncodeCtx, 0, 0);
	if (ret != 0)
		return false;
	cout << "avcodec_open2 success!" << endl;

	/* 2.创建重采样上下文 */
	AVSampleFormat inSampleFmt = AV_SAMPLE_FMT_S16;
	AVSampleFormat outSampleFmt = AV_SAMPLE_FMT_FLTP;

	this->aSwr = swr_alloc_set_opts(this->aSwr,
		av_get_default_channel_layout(this->channels), outSampleFmt, sampleRate,//输出格式
		av_get_default_channel_layout(this->channels), inSampleFmt, sampleRate, 0, 0);//输入格式
	if (!this->aSwr) {
		cout << "swr_alloc_set_opts failed!";
		return false;
	}
	ret = swr_init(this->aSwr);
	if (ret != 0) {
		return false;
	}
	cout << "音频重采样 上下文初始化成功!" << endl;

	/* 3. 分配保存重采样后的帧 */
	pcm = av_frame_alloc();
	pcm->format = outSampleFmt;
	pcm->channels = channels;
	pcm->channel_layout = av_get_default_channel_layout(channels);
	pcm->nb_samples = this->nb_samples;
	ret = av_frame_get_buffer(pcm, 0);  // 给pcm分配存储空间
	if (ret != 0) {
		cout << "ERR: av_frame_get_buffer(pcm, 0)" << endl;
		return false;
	}
	return true;
}

//重采样源S16数据, 存放到pcm中
AVFrame *MediaEncode::fmtS16_to_fmtFltp(char *inData)
{
	const uint8_t *indata[AV_NUM_DATA_POINTERS] = { 0 };
	indata[0] = (uint8_t *)inData;
	int len = swr_convert(this->aSwr, pcm->data, pcm->nb_samples, //输出参数，输出存储地址和样本数量
		indata, pcm->nb_samples);
	if (len <= 0)
		return NULL;
	return pcm;
}

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






