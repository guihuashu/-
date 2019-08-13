#include "Decode.h"
using namespace std;
#include <QDebug>

Decode::Decode(Demux *demux_, QObject *parent)
{
    //////////////////////////////////////////////////////////
    this->demux = demux_;

    AVCodec *vcodec = avcodec_find_decoder(demux->vCodecParam->codec_id);
    AVCodec *acodec = avcodec_find_decoder(demux->aCodecParam->codec_id);
    if (!acodec || !vcodec) {
        qInfo() << "can't find the codec id " ;
        getchar();
    }
    vDecodeCtx = avcodec_alloc_context3(vcodec);
    aDecodeCtx = avcodec_alloc_context3(acodec);

    //配置解码器上下文参数
    avcodec_parameters_to_context(vDecodeCtx, demux->vCodecParam);
    avcodec_parameters_to_context(aDecodeCtx, demux->aCodecParam);
    vDecodeCtx->thread_count = 8;	//八线程解码
    aDecodeCtx->thread_count = 8;	//八线程解码

    // 设置h264视频解码器的属性,降低画面延迟
    av_opt_set(vDecodeCtx->priv_data, "preset", "faster", 0); //设置priv_data的option
    av_opt_set(vDecodeCtx->priv_data, "tune", "zerolatency", 0); //设置priv_data的option

    //打开解码器上下文
    if (avcodec_open2(vDecodeCtx, 0, 0) || avcodec_open2(aDecodeCtx, 0, 0)) {
        qInfo() << "avcodec_open2  failed! :";
        getchar();
    }
    qInfo() << "video avcodec_open2 success!" ;
    qInfo() << "audio avcodec_open2 success!";

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
        qInfo() << "swr_init  failed! :";
        getchar();
    }
    qInfo() << "audio swr_init() success!";

    ////////////////////////////////////////////////////////////////////////
    pcmList.clear();
    rgb24List.clear();
    //memset(&frame, 0, sizeof(frame));
    //av_init_packet(&pkt);
}

void  free_Pcm(Pcm *pcm)
{
	if (!pcm)
		return;
	if (pcm->data)
		delete[] pcm->data;
	delete pcm;
	pcm = NULL;
}

inline void pcmListPopFront(std::list<Pcm *> *pcmList)
{
	if (!pcmList || !pcmList->size())
		return;
	Pcm *pcm = pcmList->front();
	pcmList->pop_front();	// 从链表中删除元素
	free_Pcm(pcm);			// 释放元素空间
}
inline Pcm *FrontPcm(std::list<Pcm *> *pcmList)
{
    if (!pcmList || !pcmList->size())
        return NULL;
    Pcm *pcm = pcmList->front();
    pcmList->pop_front();		// 删除第一个节点
    return pcm;
}

void  free_Rgb24(Rgb24 *rgb24)
{
    if (!rgb24)
        return;
    if (rgb24->data)
        delete[] rgb24->data;
    delete rgb24;
    rgb24 = NULL;
}

inline void PopFrontRgb24(std::list<Rgb24 *> *rgb24List)
{
    if (!rgb24List || !rgb24List->size())
		return;
    Rgb24 *rgb24 = rgb24List->front();
    rgb24List->pop_front();		// 从链表中删除元素
    free_Rgb24(rgb24);			// 释放元素空间
}

// 取出第一个PCM
// 取出第一个Rgb24
inline Rgb24 *FrontRgb24(std::list<Rgb24 *> *rgb24List)
{
    if (!rgb24List || !rgb24List->size())
		return NULL;
    Rgb24 *rgb24 = rgb24List->front();
    rgb24List->pop_front();		// 删除第一个节点
    return rgb24;
}
Pcm *Decode::getPcm()
{
	if (pcmList.size() == 0)
		return NULL;
	pcmListMutex.lock();
    Pcm *pcm = FrontPcm(&pcmList);
	pcmListMutex.unlock();
	return pcm;
}
Rgb24 *Decode::getRgb24()
{
    if (rgb24List.size() == 0)
		return NULL;
    rgb24ListMutex.lock();
    Rgb24 *rgb24 = FrontRgb24(&rgb24List);
    rgb24ListMutex.unlock();
    return rgb24;
}

static inline void dump_pkt(AVPacket *pkt)
{
	qInfo() << "pkt->size = " << pkt->size;
	qInfo() << "pkt->pts = " << pkt->pts ;	//显示的时间
	//qInfo() << "pkt->pts ms = " << pkt->pts * (calc_AVRational(demux->inFmtCtx->streams[pkt->stream_index]->time_base) * 1000) << endl;
	qInfo() << "pkt->dts = " << pkt->dts ;	//解码时间
}

Decode::~Decode()
{

}

// 第一次调用pcm应为NULL
int Decode::frameToS16(AVFrame *aframe, unsigned char *pcm)
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
    QThread::currentThread()->setPriority(QThread::HighPriority);
	// 读取音视频帧,并压入缓冲区
	while (1)
	{
		AVPacket *pkt = av_packet_alloc();

		if (av_read_frame(inFmtCtx, pkt)) {
			clean_pkt(&pkt);
			continue;
		}

		if (pkt->stream_index == vsIndex) {
			//qInfo() << "------------  图像 ------------" << endl;
			decoderCtx = vDecodeCtx;
		}else if (pkt->stream_index == asIndex) {
			//qInfo() << "------------  音频 ------------" << endl;
			decoderCtx = aDecodeCtx;
		}else {	// 可能是字幕流
			clean_pkt(&pkt);
			continue;
		}
b:
		//dump_pkt(&pkt);
		//发送packet到解码线程  send传NULL后调用多次receive取出所有缓冲帧
		if (avcodec_send_packet(decoderCtx, pkt)) {
            if (pkt->flags & AV_PKT_FLAG_KEY)
                goto b;
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
			//qInfo() << "recv frame " << frame.format << " " << frame.linesize[0] << endl;
			//视频
			if (decoderCtx == vDecodeCtx)
			{
				SwsContext *swsCtx = NULL;
                Rgb24 *rgb24 = new Rgb24();
                rgb24->w = frame->width;
                rgb24->h = frame->height;
                rgb24->lines = rgb24->w * 3;
                rgb24->data = new unsigned char[rgb24->lines * rgb24->h];	// 每个像素4字节
                this->swsCtxGet_FrameToRgb24(&swsCtx, frame);
                ret = this->frameToRgb24(swsCtx, frame, rgb24);
				
				sws_freeContext(swsCtx);

                //qWarning() << "sws_scale = " << ret;
				if (ret <= 0) {				
                    free_Rgb24(rgb24);
					continue;
				}
                rgb24ListMutex.lock();
                if (rgb24List.size() == rgb24ListSize) {

                    PopFrontRgb24(&rgb24List);	// 抛弃第一个元素
				}
                rgb24List.push_back(rgb24);			// 新元素压入链表尾部
                rgb24ListMutex.unlock();
				//CUR;
                clean_frame(&frame);	// frame已经用不着了
            }
			else if ((decoderCtx == aDecodeCtx))//音频包
			{
				//CUR;

				// 为解码后的音频帧准备空间: nb_samples(单通道样本数) * 通道数(2) * 样本大小(S16, 2字节)
				int size = frame->nb_samples * 2 * 2;
				Pcm *pcm = new Pcm();
				pcm->size = size;
				pcm->data = new unsigned char[size];

                ret = this->frameToS16(frame, pcm->data);
				clean_frame(&frame);
                //qInfo() << "swr_convert = " << ret;
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


void Decode::swsCtxGet_FrameToRgb24(SwsContext **swsCtx,AVFrame *frame)
{
	*swsCtx = sws_getCachedContext(
		*swsCtx,	//传NULL会新创建
		frame->width, frame->height,	//输入的宽高
		(AVPixelFormat)frame->format,	//输入格式 YUV420p, h264解压后的格式
        frame->width, frame->height,	//输出的宽高
        /* 注意: 安卓上转为bpp32将失败 */
        AV_PIX_FMT_RGB24,				//输入格式RGB32, 用于QT显示的图片
        SWS_FAST_BILINEAR,					//尺寸变化的算法
		0, 0, 0);
}
int Decode::frameToRgb24(SwsContext *swsCtx, AVFrame *frame, Rgb24 *rgb24)
{
	uint8_t *data[AV_NUM_DATA_POINTERS] = { 0 };
	int lines[AV_NUM_DATA_POINTERS] = { 0 };
    data[0] = rgb24->data;
    lines[0] = rgb24->lines;

	return sws_scale(swsCtx,
		frame->data,		//输入数据
		frame->linesize,	//输入行大小
		0,
        rgb24->h,			//输入高度
		data,		//输出数据和大小
		lines
	);
}




