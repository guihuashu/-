#ifndef Demux_H
#define Demux_H

extern "C" {
#include <libavformat/avformat.h>
//#include <libavdevice/avdevice.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/mediacodec.h>
#include <libavutil/avutil.h>
#include <libavutil/dict.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>

}
//#include <opencv2/highgui.hpp>		//imread, 包含界面的一些文件
#include <list>
#include <thread>
#include <mutex>
#include <QDebug>
#include <QApplication>
#include <QCoreApplication>
#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QDebug>
#include <QThread>
#include <QIODevice>
#include <QImage>
#include <QThreadPool>
#include <iostream>

#define CUR qWarning()<<__FILE__<<","<<__LINE__

double calc_AVRational(AVRational Ration);

class Demux
{
public:
    Demux(const char *inUrl);

public:
	virtual ~Demux();
    //const char *inUrl;
	const char *path = "rtmp://hdlcontrol.com/live/stream";
	AVDictionary *opts = NULL;
	AVStream *as;
	AVStream *vs;
	AVCodecParameters *aCodecParam;
	AVCodecParameters *vCodecParam;
	int vsIndex;
	int asIndex;
	AVFormatContext *inFmtCtx = NULL;
};

#endif // Demux_H
