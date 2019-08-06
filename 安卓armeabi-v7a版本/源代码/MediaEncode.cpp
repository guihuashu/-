#include "MediaEncode.h"

using namespace std;


MediaEncode::~MediaEncode()
{
    //avcodec_free_context(&vEncodeCtx);
    //av_frame_free(&yuv420p);
    //avcodec_close(vEncodeCtx);
}

MediaEncode::MediaEncode()
{
    qInfo()<< "---------------  MediaEncode ----------------" << endl;


    //memset(&_vArgs, 0, sizeof(&_vArgs));
    _vArgs.thread_count = 6;		// 用于编码的线程数, 系统所有线程数
    _vArgs.bit_rate = 5 * 1024 * 8;			// 码率, 越大视频越清晰
    _vArgs.gop_size = 50;						// 关键帧周期
    _vArgs.max_b_frames = 0;					// 最大b帧数, 有b帧就会有延迟
    _vArgs.outWidth = 640;
    _vArgs.outHeight = 480;
//#ifdef ANDROID
   // this->vArgs.in_pixFmt = AV_PIX_FMT_BGR24;		// 输出帧的像素格式
//#elif  WIN64
    //this->vArgs.in_pixFmt = AV_PIX_FMT_BGR24;		// 输出帧的像素格式
//#endif
    _vArgs.out_pixFmt = AV_PIX_FMT_YUV420P;	// 输入帧的像素格式

    _sysPts = av_gettime();			// 微妙
    _sysTimebase = { 1, 1000000 };	// 系统时间基数	(时间秒数 = 时间戳 * 时间基数)
    gpool = QThreadPool::globalInstance();
    CUR;


}
bool MediaEncode::init_vEncodeCtx(QtVideoCap *cap)
{
    int ret;
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
    _vEncodeCtx->thread_count = _vArgs.thread_count;		// 用于编码的线程数
    _vEncodeCtx->bit_rate = _vArgs.bit_rate;				// 码率, 越大视频越清晰
    _vEncodeCtx->width = _vArgs.outWidth;					// 输出图像的宽度 = 输入的宽度
    _vEncodeCtx->height = _vArgs.outHeight;                 // 输出图像的高度
    _vEncodeCtx->time_base = { 1,1000000 };                 // 时间基数,微秒
    _vEncodeCtx->framerate = { (int)cap->fps, 1 };             // fps
    _vEncodeCtx->gop_size = _vArgs.gop_size;				// 关键帧周期
    _vEncodeCtx->max_b_frames = _vArgs.max_b_frames;		// 最大b帧数, 有b帧就会有延迟
    _vEncodeCtx->pix_fmt = _vArgs.out_pixFmt;				// 输入帧的像素格式
    // 优化推流延迟, 设置编码器的的私有属性来优化推流延迟
    //_vEncodeCtx->delay = 0;
    CUR;
    av_opt_set(_vEncodeCtx->priv_data, "fast", "faster", 0); //设置priv_data的option
    av_opt_set(_vEncodeCtx->priv_data, "tune", "zerolatency", 0); //设置priv_data的option
    // 安卓上设置这个会报错
    //av_opt_set(_vEncodeCtx->priv_data, "x264opts", "sync-lookahead=0: sliced-threads", 0);
    if (avcodec_open2(_vEncodeCtx, NULL, NULL))	{       // 打开编码器(编码器开始待解码)
        qWarning()<<"ERR: avcodec_open2(_vEncodeCtx, 0, 0)";
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////
bool MediaEncode::init_vEncode(QtVideoCap *cap)
{
    CUR;
    int ret;

    if (!init_vEncodeCtx(cap)) {
        qWarning()<<"ERR: init_vEncodeCtx";
        return false;
    }
#ifdef ANDROID
    /* 初始化格式转换上下文(Format_NV21->AV_PIX_FMT_YUV420P) */
    SwsNv21toYuv420p = sws_getCachedContext(SwsNv21toYuv420p, \
        cap->inSize.width(), cap->inSize.height(), cap->inFmt, \
        _vArgs.outWidth, _vArgs.outHeight, _vArgs.out_pixFmt, \
        SWS_FAST_BILINEAR, NULL, NULL, NULL);
#endif
    CUR;
    // 连接视频采集信号
    connect(cap->_probe, SIGNAL(videoFrameProbed(const QVideoFrame &)), this, SLOT(newVideoFrame(const QVideoFrame &)));
    //connect(cap->_probe, &QVideoProbe::videoFrameProbed, this, &MediaEncode::newVideoFrame);
    CUR;
    return true;
}



// 抛弃第一个元素
void MediaEncode::pktListPopFont(std::list<AVPacket *> *list)
{
    if (!list || !list->size())
        return;
    AVPacket *pkt = list->front();
    list->pop_front();          // 从链表中删除第一个元素
    freePkt(&pkt);			// 释放元素空间
}


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

AVPacket *MediaEncode::gain_vPkt()
{
#if 0
	while (vPktList.size() <= 0) {
		av_usleep(50);
	}
#endif
    if (_vPktList.size() <= 0)
		return NULL;
    _vList_mutex.lock();
    AVPacket *pkt = _vPktList.back();	// 取出链表最后一项
    _vPktList.pop_back();				// 删除链表最后一项
    _vList_mutex.unlock();
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

void make_vPkt(MediaEncode *This)
{

}

bool MediaEncode::init_avEncode()
{
	// 初始化视频编码器和音频编码器相关
    //if (!init_vEncode())
        //return false;
	if (!init_aEncode())
		return false;

	
    //thread threada(make_aPkt,this);
    //threada.detach();
#if 1
    //thread threadv(make_vPkt, this);
    //threadv.detach();
#endif
	
    return true;
}






//////////////////////////////////////////////////////////////////////////
void MediaEncode::set_vCodeArg(enum vEncodeArgType type, int64_t val)
{
	if (type == THREAD_COUNT)
        _vArgs.thread_count = (int)val;

	else if (type == BIT_RATE)
        _vArgs.bit_rate = (int64_t)val;

	else if (type == OUTWIDTH)
        _vArgs.outWidth = (int)val;

	else if (type == OUTHEIGHT)
        _vArgs.outHeight = (int)val;

	else if (type == MAX_B_FRAMES)
        _vArgs.max_b_frames = (int)val;

	else
		cout << "Unknown parameter types" << endl;
}


void MediaEncode::vPktPushFront(AVPacket *pkt)
{
    _vList_mutex.lock();
    if (_vPktList.size() == _vListSize) {	// 缓冲区是满的
        pktListPopFont(&_vPktList);
    }
    _vPktList.push_front(pkt);  // 包压入链表
    _vList_mutex.unlock();
    qInfo()<<"_vPktList.size = "<<_vPktList.size();
}

int MediaEncode::Nv21toYuv420p(QVideoFrame &frame, AVFrame *yuv420p)
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
bool MediaEncode::vEncode(AVFrame *yuv420p_, AVPacket *vPkt)
{

#if 0	// 这是基于帧率计算的pts
    yuv420p_->pts = vPts;
    /* 编码时间戳: = 一帧时间 / 编码时间基数
                   = 1/fps * {1/fps} = 1 */
    vPts += 1;
#endif
    // CPU过一段时间才计时,避免相邻两帧pts相同
    static long long prePts = -1;
    if (prePts == vPkt->pts)
        vPkt->pts += 1000;
    prePts = vPkt->pts;
    //CUR;
    yuv420p_->pts = vPkt->pts;	// 数据帧的采集时间戳

    if (!_vEncodeCtx)
        return false;
    if (avcodec_send_frame(_vEncodeCtx, yuv420p_)) // 将YUV420p帧放入h264编码器队列
        return false;
    av_init_packet(vPkt);
    if (avcodec_receive_packet(this->_vEncodeCtx, vPkt))	// 从编码器中读取h264编码后的数据包
        return false;
    if (vPkt->size <= 0)
        return false;
    return true;
}

void MediaEncode::newVideoFrame(const QVideoFrame &frame)
{
    int ret;

    qInfo()<<"MediaEncode::newYuv420p";
    QVideoFrame curFrame = frame;
    curFrame.map(QAbstractVideoBuffer::ReadWrite);
    if (!curFrame.isValid()) {
        qWarning()<<"curFrame inValid";
        return;
    }
}



vTask::QRunnable(QVideoFrame curFrame, MediaEncode *This)
{
    _curFrame = curFrame;
    _This = This;
}

void vTask::run()
{
    int ret;
    AVFrame *yuv420p;

    if (!allocYuv420p(&yuv420p)){
        qWarning()<<"allocYuv420p Err";
        return;
    }
    ret = _This->Nv21toYuv420p(_curFrame,yuv420p);
    qInfo()<<"ret ="<< ret;
    if (ret <= 0) {
        qWarning()<<"Nv21toYuv420p Err";
        freeFrame(&yuv420p);
        return;
    }

    AVPacket *pkt = av_packet_alloc();
    if (!pkt) {
        freeFrame(&yuv420p);
        return;
    }
    av_init_packet(pkt);
    pkt->pts = av_gettime() - _This->_sysPts;	// 包的时间戳(毫秒) = 当前时间-开始记录的时间
    if (!(_This->vEncode(yuv420p, pkt))) {
        qWarning()<<"ERR: vEncode()";
        freeFrame(&yuv420p);
        freePkt(&pkt);
        return;
    }
    freeFrame(&yuv420p);
    vPktPushFront(pkt);
}
