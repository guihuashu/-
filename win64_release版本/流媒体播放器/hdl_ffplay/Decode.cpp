#include "Decode.h"
using namespace std;


void  free_Pcm(Pcm *pcm)
{
	if (!pcm)
		return;
	if (pcm->data)
		delete[] pcm->data;
	delete pcm;
	pcm = NULL;
}
void  free_Rgb32(Rgb32 *rgb32)
{
	if (!rgb32)
		return;
	if (rgb32->data)
		delete[] rgb32->data;
	delete rgb32;
	rgb32 = NULL;
}

inline void pcmListPopFront(std::list<Pcm *> *pcmList)
{
	if (!pcmList || !pcmList->size())
		return;
	Pcm *pcm = pcmList->front();
	pcmList->pop_front();	// 从链表中删除元素
	free_Pcm(pcm);			// 释放元素空间
}
inline void rgb32ListPopFront(std::list<Rgb32 *> *rgb32List)
{
	if (!rgb32List || !rgb32List->size())
		return;
	Rgb32 *rgb32 = rgb32List->front();
	rgb32List->pop_front();		// 从链表中删除元素
	free_Rgb32(rgb32);			// 释放元素空间
}

// 取出第一个PCM
inline Pcm *getOutFrontPcm(std::list<Pcm *> *pcmList)
{
	if (!pcmList || !pcmList->size())
		return NULL;
	Pcm *pcm = pcmList->front();
	pcmList->pop_front();		// 删除第一个节点
	return pcm;
}
// 取出第一个Rgb32
inline Rgb32 *getOutFrontRgb32(std::list<Rgb32 *> *rgb32List)
{
	if (!rgb32List || !rgb32List->size())
		return NULL;
	Rgb32 *rgb32 = rgb32List->front();
	rgb32List->pop_front();		// 删除第一个节点
	return rgb32;
}
Pcm *Decode::getPcm()
{
	if (pcmList.size() == 0)
		return NULL;
	pcmListMutex.lock();
	Pcm *pcm = getOutFrontPcm(&pcmList);
	pcmListMutex.unlock();
	return pcm;
}
Rgb32 *Decode::getRgb32()
{
	if (rgb32List.size() == 0)
		return NULL;
	rgb32ListMutex.lock();
	Rgb32 *rgb32 = getOutFrontRgb32(&rgb32List);
	rgb32ListMutex.unlock();
	return rgb32;
}

static inline void dump_pkt(AVPacket *pkt)
{
	cout << "pkt->size = " << pkt->size << endl;
	cout << "pkt->pts = " << pkt->pts << endl;	//显示的时间
	//cout << "pkt->pts ms = " << pkt->pts * (calc_AVRational(demux->inFmtCtx->streams[pkt->stream_index]->time_base) * 1000) << endl;
	cout << "pkt->dts = " << pkt->dts << endl;	//解码时间
}

Decode::~Decode()
{

}

Decode::Decode(Demux *demux_, QObject *parent)
{
	//////////////////////////////////////////////////////////
	this->demux = demux_;

	AVCodec *vcodec = avcodec_find_decoder(demux->vCodecParam->codec_id);
	AVCodec *acodec = avcodec_find_decoder(demux->aCodecParam->codec_id);
	if (!acodec || !vcodec) {
		cout << "can't find the codec id " << endl;
		exit(0);
	}
	vDecodeCtx = avcodec_alloc_context3(vcodec);
	aDecodeCtx = avcodec_alloc_context3(acodec);

	//配置解码器上下文参数
	avcodec_parameters_to_context(vDecodeCtx, demux->vCodecParam);
	avcodec_parameters_to_context(aDecodeCtx, demux->aCodecParam);
	vDecodeCtx->thread_count = 8;	//八线程解码
	aDecodeCtx->thread_count = 8;	//八线程解码


	// 设置解码器的属性,降低画面延迟
	av_opt_set(vDecodeCtx->priv_data, "fast", "faster", 0); //设置priv_data的option
	av_opt_set(vDecodeCtx->priv_data, "tune", "zerolatency", 0); //设置priv_data的option




	//打开解码器上下文
	if (avcodec_open2(vDecodeCtx, 0, 0) || avcodec_open2(aDecodeCtx, 0, 0)) {	
		cout << "avcodec_open2  failed! :" << endl;
		exit(0);
	}
	cout << "video avcodec_open2 success!" << endl;
	cout << "audio avcodec_open2 success!" << endl;

	//////////////////////////////////////////////////////////
		//音频重采样 上下文初始化
	swrCtx = swr_alloc();
	swrCtx = swr_alloc_set_opts(swrCtx,
		av_get_default_channel_layout(2),					//输出格式
		AV_SAMPLE_FMT_S16,									//输出样本格式
		aDecodeCtx->sample_rate,							//输出采样率
		av_get_default_channel_layout(aDecodeCtx->channels),//输入格式
		aDecodeCtx->sample_fmt,
		aDecodeCtx->sample_rate,
		0, 0 );
	if (swr_init(swrCtx)) {
		cout << "swr_init  failed! :" << endl;
		exit(0);
	}
	cout << "audio swr_init() success!" << endl;

	////////////////////////////////////////////////////////////////////////
	pcmList.clear();
	rgb32List.clear();
	//memset(&frame, 0, sizeof(frame));
	//av_init_packet(&pkt);
}
// 第一次调用pcm应为NULL
int Decode::swe_s16Convert(AVFrame *aframe, unsigned char *pcm)
{
	
	uint8_t *outData[AV_NUM_DATA_POINTERS] = { 0 };	// 输出音频数据位置
	outData[0] = pcm;	// S16是交叉储存模型

	// 返回重采样后的单通道样本数量
	int ret =  swr_convert(swrCtx,
			outData, aframe->nb_samples,	// nb_samples: 单通道样本数
			(const uint8_t**)aframe->data, aframe->nb_samples); // 输入数据为FLTP平面存取模型					
	//av_frame_free(&aframe);
	return ret;
}

static inline void clean_pkt(AVPacket **pkt)
{
	if (!pkt)
		return;
	if (*pkt) 
		av_packet_unref(*pkt);
	av_packet_free(pkt);
}
static inline void clean_frame(AVFrame **frame)
{
	if (!frame)
		return;
	if (*frame)
		av_frame_unref(*frame);
	av_frame_free(frame);
}

void Decode::run()
{
	CUR;
	AVCodecContext *decoderCtx = NULL;
	AVFormatContext *inFmtCtx = demux->inFmtCtx;
	int vsIndex = demux->vsIndex;
	int asIndex = demux->asIndex;
	int ret;
	CUR;
	// 读取音视频帧,并压入缓冲区
	while (1)
	{
		AVPacket *pkt = av_packet_alloc();

		if (av_read_frame(inFmtCtx, pkt)) {
			clean_pkt(&pkt);
			continue;
		}

		if (pkt->stream_index == vsIndex) {
			//cout << "------------  图像 ------------" << endl;
			decoderCtx = vDecodeCtx;
		}else if (pkt->stream_index == asIndex) {
			//cout << "------------  音频 ------------" << endl;
			decoderCtx = aDecodeCtx;
		}else {	// 可能是字幕流
			clean_pkt(&pkt);
			continue;
		}

		//dump_pkt(&pkt);
		//发送packet到解码线程  send传NULL后调用多次receive取出所有缓冲帧
		if (avcodec_send_packet(decoderCtx, pkt)) {
			clean_pkt(&pkt);
			continue;
		}

		clean_pkt(&pkt);

		for (;;)
		{
			//CUR;

			//memset(&frame, 0, sizeof(frame));
			AVFrame *frame = av_frame_alloc();
			//从线程中获取解码接口,一次send可能对应多次receive
			if (avcodec_receive_frame(decoderCtx, frame)) {
				clean_frame(&frame);
				break;
			}
			//cout << "recv frame " << frame.format << " " << frame.linesize[0] << endl;
			//视频
			if (decoderCtx == vDecodeCtx)
			{
				SwsContext *swsCtx = NULL;
				Rgb32 *rgb32 = new Rgb32();
				rgb32->w = frame->width;
				rgb32->h = frame->height;
				rgb32->lines = rgb32->w * 4;
				rgb32->data = new unsigned char[rgb32->lines * rgb32->h];	// 每个像素4字节
				this->sws_getRGB32CachedContext(&swsCtx, frame);
				ret = this->sws_rgb32Scale(swsCtx, frame, rgb32);
				
				clean_frame(&frame);	// frame已经用不着了
				sws_freeContext(swsCtx);

				//cout << "sws_scale = " << ret << endl;
				if (ret <= 0) {				
					free_Rgb32(rgb32);
					continue;
				}
				rgb32ListMutex.lock();
				if (rgb32List.size() == rgb32ListSize) {
					rgb32ListPopFront(&rgb32List);	// 抛弃第一个元素
				}
				rgb32List.push_back(rgb32);			// 新元素压入链表尾部
				rgb32ListMutex.unlock();
				//CUR;

			}
			else if ((decoderCtx == aDecodeCtx))//音频包
			{
				//CUR;

				// 为解码后的音频帧准备空间: nb_samples(单通道样本数) * 通道数(2) * 样本大小(S16, 2字节)
				int size = frame->nb_samples * 2 * 2;
				Pcm *pcm = new Pcm();
				pcm->size = size;
				pcm->data = new unsigned char[size];

				ret = this->swe_s16Convert(frame, pcm->data);
				clean_frame(&frame);
				//cout << "swr_convert = " << ret  << endl;
				if (ret <= 0) {
					free_Pcm(pcm);
					continue;
				}
				pcmListMutex.lock();
				if (pcmList.size() == pcmListSize) {
					pcmListPopFront(&pcmList);	// 抛弃第一个元素
				}
				pcmList.push_back(pcm);	// 新元素压入链表尾部
				pcmListMutex.unlock();
				//CUR;
			}
		}
	}
}




void Decode::sws_getRGB32CachedContext(SwsContext **swsCtx,AVFrame *frame)
{
	*swsCtx = sws_getCachedContext(
		*swsCtx,	//传NULL会新创建
		frame->width, frame->height,	//输入的宽高
		(AVPixelFormat)frame->format,	//输入格式 YUV420p, h264解压后的格式
		frame->width, frame->height,	//输出的宽高
		AV_PIX_FMT_RGB32,				//输入格式RGB32, 用于QT显示的图片
		SWS_BICUBIC,					//尺寸变化的算法
		0, 0, 0);
}
int Decode::sws_rgb32Scale(SwsContext *swsCtx, AVFrame *frame, Rgb32 *rgb32)
{
	uint8_t *data[AV_NUM_DATA_POINTERS] = { 0 };
	int lines[AV_NUM_DATA_POINTERS] = { 0 };
	data[0] = rgb32->data;
	lines[0] = rgb32->lines;

	return sws_scale(swsCtx,
		frame->data,		//输入数据
		frame->linesize,	//输入行大小
		0,
		rgb32->h,			//输入高度
		data,		//输出数据和大小
		lines
	);
}




