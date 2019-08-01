#ifndef DECODE_H
#define DECODE_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/mediacodec.h>
#include <libavutil/avutil.h>
#include <libavutil/dict.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>
}
#include <iostream>
#include <QThread>
#include <list>
#include <QImage>
#include <Demux.h>
#include <mutex>


typedef struct Pcm {
	unsigned char *data;
	int size;
}Pcm;

typedef struct Rgb32 {
	unsigned char *data;
	int w;
	int h;
	int lines;
}Rgb32;


void free_Pcm(Pcm *pcm);
void  free_Rgb32(Rgb32 *rgb32);

class Decode : public QThread
{
public:
	Decode(Demux *demux, QObject *parent = Q_NULLPTR);

	int swe_s16Convert(AVFrame *aframe, unsigned char *pcm);
	void sws_getRGB32CachedContext(SwsContext **swsCtx, AVFrame *frame);
	int sws_rgb32Scale(SwsContext *swsCtx, AVFrame *frame, Rgb32 *rgb32);
	virtual void run();

	virtual ~Decode();

public:
	AVCodecContext *vDecodeCtx;
	AVCodecContext *aDecodeCtx;
	SwrContext *swrCtx = NULL;
	Pcm *getPcm();
	Rgb32 *getRgb32();
	Demux *demux;
	//AVPacket pkt;
	//AVFrame frame;
	unsigned char *rgb = NULL;
	//unsigned char *pcm = NULL;	/* 音频解压后的帧数据 */
	std::list<Pcm *> pcmList;
	std::list<Rgb32 *> rgb32List;
	std::mutex pcmListMutex;
	std::mutex rgb32ListMutex;
	int pcmListSize = 5;
	int rgb32ListSize = 5;
};

#endif	// DECODE_H