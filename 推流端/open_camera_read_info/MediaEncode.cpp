#include "MediaEncode.h"
#include <iostream>

using namespace std;
using namespace cv;


MediaEncode::~MediaEncode()
{
	avcodec_free_context(&vEncodeCtx);
	av_frame_free(&yuv420p);
	avcodec_close(vEncodeCtx);
}

MediaEncode::MediaEncode()
{
	cout << "---------------  MediaEncode ----------------" << endl;
	memset(&this->vArgs, 0, sizeof(&this->vArgs));
	this->vArgs.thread_count = get_sysCpuNum();		// 用于编码的线程数, 系统所有线程数
	//this->args.thread_count = 6;		// 用于编码的线程数
	this->vArgs.bit_rate = 50 * 1024 * 8;			// 码率, 越大视频越清晰
	this->vArgs.inWidth =  640;						// 输入图像的宽度
	this->vArgs.inHeight = 480;						// 输入图像的高度
	this->vArgs.outWidth = this->vArgs.inWidth;		// 输出图像的宽度
	this->vArgs.outHeight = this->vArgs.inHeight;	// 输出图像的高度
	this->vArgs.fps = 30;
	this->vArgs.gop_size = 50;						// 关键帧周期
	this->vArgs.max_b_frames = 0;					// 最大b帧数, 有b帧就会有延迟
	this->vArgs.in_pixFmt = AV_PIX_FMT_BGR24;		// 输出帧的像素格式
	this->vArgs.out_pixFmt = AV_PIX_FMT_YUV420P;	// 输入帧的像素格式
	
}

///////////////////////////////////////////////////////////////////////////
bool MediaEncode::init_vEncode()
{
	/* 1.初始化视频编码器 */
	AVCodec *codec = NULL;
	codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	vEncodeCtx = avcodec_alloc_context3(codec);

	vEncodeCtx->codec_id = codec->id;					// h264
	vEncodeCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;	 // 全局标记
	vEncodeCtx->thread_count = vArgs.thread_count;		// 用于编码的线程数
	vEncodeCtx->bit_rate = vArgs.bit_rate;				// 码率, 越大视频越清晰
	vEncodeCtx->width = vArgs.outWidth;					// 输出图像的宽度 = 输入的宽度
	vEncodeCtx->height = vArgs.outHeight;				// 输出图像的高度
	vEncodeCtx->time_base = { 1,1000000 };			// 时间基数
	vEncodeCtx->framerate = { vArgs.fps, 1 };			// fps
	vEncodeCtx->gop_size = vArgs.gop_size;				// 关键帧周期
	vEncodeCtx->max_b_frames = vArgs.max_b_frames;		// 最大b帧数, 有b帧就会有延迟
	vEncodeCtx->pix_fmt = vArgs.out_pixFmt;				// 输入帧的像素格式
	// 优化推流延迟, 设置编码器的的私有属性来优化推流延迟
	vEncodeCtx->delay = 0;

#if 0
	if (id == AV_CODEC_ID_H265) {
		//av_opt_set(encoderCtx->priv_data, "preset", "x265-params","qp=20", 0); //设置priv_data的option
		av_opt_set(encoderCtx->priv_data, "preset", "superfast", 0); //设置priv_data的option
		av_opt_set(encoderCtx->priv_data, "tune", "zero-latency", 0); //设置priv_data的option
	}
#endif
	av_opt_set(vEncodeCtx->priv_data, "fast", "faster", 0); //设置priv_data的option
	av_opt_set(vEncodeCtx->priv_data, "tune", "zerolatency", 0); //设置priv_data的option
	//av_opt_set(encoderCtx->priv_data, "x264opts", "sync-lookahead=0: sliced-threads", 0);
	if (avcodec_open2(vEncodeCtx, 0, 0))	// 打开编码器(编码器开始待解码)
		return false;

	/* 2.分配输出帧 */
	if (!(this->yuv420p = av_frame_alloc())) 
		return false;
	/*if (!create_vOutFrame(this->yuv420p, &this->vArgs))
		return false;*/
	yuv420p->format = vArgs.out_pixFmt;
	yuv420p->width = vArgs.outWidth;
	yuv420p->height = vArgs.outHeight;
	yuv420p->pts = 0;
	if (av_frame_get_buffer(yuv420p, 32))	// 32字节对其
		return false;

	/* 3.分配视频转码上下文 */
	vSwsCtx = sws_getCachedContext(vSwsCtx, \
		vArgs.inWidth, vArgs.inHeight, vArgs.in_pixFmt, \
		vArgs.outWidth, vArgs.outHeight, vArgs.out_pixFmt, \
		SWS_BICUBIC, 0, 0, 0);
	return vSwsCtx;
}


AVFrame *MediaEncode::brg24ToYuv420p(Mat *inMat)
{
	int ret;
	uint8_t *indata[AV_NUM_DATA_POINTERS] = { 0 };	// 数据源(R.G.B./Y.U.V)
	int	insize[AV_NUM_DATA_POINTERS] = { 0 };		// 数据源(R.G.B./Y.U.V)
	uint8_t **outdata;			// uint8_t*[AV_NUM_DATA_POINTERS]数组
	int *outsize;				// int[AV_NUM_DATA_POINTERS]数组
	
	indata[0] = inMat->data;						// 采集到的帧数据
	insize[0] = inMat->cols * inMat->elemSize();	// 输入数据对其后一行的字节数
	outdata = yuv420p->data;						// 转换后帧的存储位置
	outsize = yuv420p->linesize;					// 由系统内部进行计算和填充
	int h = sws_scale(vSwsCtx, indata, insize, 0, inMat->rows, outdata, outsize);

	if (h <= 0)
		return NULL;
	return yuv420p;
}

bool MediaEncode::vEncode(AVFrame *yuv420p_, AVPacket *vPkt)
{
	
#if 0	// 这是基于帧率计算的pts
	yuv420p_->pts = vPts;
	/* 编码时间戳: = 一帧时间 / 编码时间基数
				   = 1/fps * {1/fps} = 1 */
	vPts += 1;
#endif
#if 1
	// CPU过一段时间才计时,避免相邻两帧pts相同
	static long long prePts = -1;
	if (prePts == vPkt->pts)
		vPkt->pts += 1000;
	prePts = vPkt->pts;
#endif
	yuv420p_->pts = vPkt->pts;	// 数据帧的采集时间戳
	if (avcodec_send_frame(this->vEncodeCtx, yuv420p_)) // 将YUV420p帧放入h264编码器队列
		return false;

	av_init_packet(vPkt);
	if (avcodec_receive_packet(this->vEncodeCtx, vPkt))	// 从编码器中读取h264编码后的数据包
		return false;
	if (vPkt->size <= 0)
		return false;
	return true;
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
	aEncodeCtx->thread_count = get_sysCpuNum();
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
	if (aPktList.size() <= 0) {
		return NULL;
	}
	aList_mutex.lock();
	AVPacket *pkt = aPktList.back();	// 取出链表最后一项
	aPktList.pop_back();				// 删除链表最后一项
	aList_mutex.unlock();
	return pkt;
}

AVPacket *MediaEncode::gain_vPkt()
{
#if 0
	while (vPktList.size() <= 0) {
		av_usleep(50);
	}
#endif
	if (vPktList.size() <= 0)
		return NULL;
	vList_mutex.lock();
	AVPacket *pkt = vPktList.back();	// 取出链表最后一项
	vPktList.pop_back();				// 删除链表最后一项
	vList_mutex.unlock();
	return pkt;
}



void make_aPkt(MediaEncode *This)
{
	AudioRecord audioRecord;
	AVFrame *pcm_;

	audioRecord.init_record();
	char *buf = new char[This->aFrameBufSize];
	list<AVPacket *> *aPktList = &This->aPktList;
	int ListSize = This->aListSize;
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

		pkt->pts = av_gettime() - This->sysPts;	// 包的时间戳(毫秒) = 当前时间-开始记录的时间


		if (!This->aEncode(pcm_, pkt)) {
			av_packet_unref(pkt);	// 包中数据的引用计数-1
			av_packet_free(&pkt);	// 释放尾部的包
			pkt = NULL;
			continue;
		}
		//av_packet_unref(pkt);	// 包中数据的引用计数-1
		This->aList_mutex.lock();
		if (aPktList->size() == ListSize) {	// 缓冲区是满的
			aPktList->pop_back();			// 删掉尾部的包
			av_packet_unref(pkt);	// 包中数据的引用计数-1
			av_packet_free(&aPktList->back());	// 释放尾部的包
		}
		aPktList->push_front(pkt);	// 插到链表前面
		This->aList_mutex.unlock();
	}
}
void make_vPkt(MediaEncode *This)
{
	VideoCap camera;	// opencv相机
	AVFrame *yuv420p_;
	Mat *mat;				// opencv相机一帧

	if (!camera.init_videoCap()) {
		cout << "ERR init_videoCap" << endl;
		exit(0);
	}
	list<AVPacket *> *vPktList = &This->vPktList;
	int ListSize = This->vListSize;
	vPktList->clear();
	while (1)
	{
		AVPacket *pkt = av_packet_alloc();
		av_init_packet(pkt);
		if (!pkt)
			continue;
		/* 8.opencv相机读取一帧,并且转换为了BGR24 */
		if (!(mat = camera.get_mat()))	//调用:  camera.grab(), camera.retrieve(frame)
			continue;
#if 0
		imshow("video", *mat);

		// 延迟一毫秒并刷新"video"窗口
		waitKey(1);	
#endif
		if (!(yuv420p_ = This->brg24ToYuv420p(mat))) {
			av_packet_unref(pkt);	// 包中数据的引用计数-1
			av_packet_free(&pkt);	// 释放尾部的包
			continue;
		}
		pkt->pts = av_gettime() - This->sysPts;	// 包的时间戳(毫秒) = 当前时间-开始记录的时间
		
		if (!(This->vEncode(yuv420p_, pkt))) {
			av_packet_unref(pkt);	// 包中数据的引用计数-1
			av_packet_free(&pkt);	// 释放尾部的包
			continue;
		}
		//av_packet_unref(pkt);	// 包中数据的引用计数-1
		This->vList_mutex.lock();
		if (vPktList->size() == ListSize) {	// 缓冲区是满的
			vPktList->pop_back();			// 删掉尾部的包
			av_packet_unref(pkt);	// 包中数据的引用计数-1
			av_packet_free(&vPktList->back());	// 释放尾部的包
		}
		vPktList->push_front(pkt);
		This->vList_mutex.unlock();
	}
}

bool MediaEncode::init_avEncode()
{
	// 初始化视频编码器和音频编码器相关
	if (!init_vEncode())
		return false;
	if (!init_aEncode())
		return false;

	// 记录系统时间戳
	sysPts = av_gettime();			// 微妙
	sysTimebase = { 1, 1000000 };	// 系统时间基数	(时间秒数 = 时间戳 * 时间基数)
	
	thread threada(make_aPkt,this);
	threada.detach();
#if 1
	thread threadv(make_vPkt, this);
	threadv.detach();
#endif
	
	return true;
}




//////////////////////////////////////////////////////////////////////////
void MediaEncode::set_vCodeArg(enum vEncodeArgType type, int64_t val)
{
	if (type == THREAD_COUNT)
		vArgs.thread_count = (int)val;

	else if (type == BIT_RATE)
		vArgs.bit_rate = (int64_t)val;

	else if (type == INWIDTH)
		vArgs.inWidth = (int)val;

	else if (type == INHEIGHT)
		vArgs.inHeight = (int)val;

	else if (type == OUTWIDTH)
		vArgs.outWidth = (int)val;

	else if (type == OUTHEIGHT)
		vArgs.outHeight = (int)val;

	else if (type == FPS)
		vArgs.fps = (int)val;

	else if (type == MAX_B_FRAMES)
		vArgs.max_b_frames = (int)val;

	else
		cout << "Unknown parameter types" << endl;
}







