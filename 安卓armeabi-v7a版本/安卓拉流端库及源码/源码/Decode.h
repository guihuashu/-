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

typedef struct Rgb24 {
	unsigned char *data;
	int w;
	int h;
	int lines;
}Rgb24;


void free_Pcm(Pcm *pcm);
void  free_Rgb24(Rgb24 *rgb24);

class Decode : public QThread
{
public:
	Decode(Demux *demux, QObject *parent = Q_NULLPTR);

    int frameToS16(AVFrame *aframe, unsigned char *pcm);
    void swsCtxGet_FrameToRgb24(SwsContext **swsCtx, AVFrame *frame);
    int frameToRgb24(SwsContext *swsCtx, AVFrame *frame, Rgb24 *rgb24);
	virtual void run();

	virtual ~Decode();

public:
	AVCodecContext *vDecodeCtx;
	AVCodecContext *aDecodeCtx;
	SwrContext *swrCtx = NULL;
	Pcm *getPcm();
    Rgb24 *getRgb24();
	Demux *demux;
	//AVPacket pkt;
	//AVFrame frame;
	unsigned char *rgb = NULL;
	//unsigned char *pcm = NULL;	/* 音频解压后的帧数据 */
	std::list<Pcm *> pcmList;
    std::list<Rgb24 *> rgb24List;
	std::mutex pcmListMutex;
    std::mutex rgb24ListMutex;
    int pcmListSize = 2;
    int rgb24ListSize = 2;
};

#endif	// DECODE_H
