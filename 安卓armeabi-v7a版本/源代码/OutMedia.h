#ifndef OUTMEDIA_H
#define OUTMEDIA_H
#include <iostream>
#include <qtvideocap.h>
#include <MediaEncode.h>
#include <debug.h>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavcodec/mediacodec.h>
    #include <libavutil/avutil.h>
    #include <libavutil/dict.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
    #include <libavutil/time.h>
}

using namespace std;
class OutMedia
{
public:
    OutMedia(string outUrl, string streamFmt, MediaEncode *encode);
	~OutMedia();
    //bool init(string outUrl, string streamFmt, MediaEncode *encode);
	bool addStream(AVCodecContext *encodeCtx);
	void dump_outMediaFmt();
    void write_headerInfo();
	bool send_vPkt(AVPacket *pkt);
	bool send_aPkt(AVPacket *pkt);

	AVStream *get_vStream();
	AVStream *get_aStream();
	AVFormatContext *get_outFmtCtx();
	int get_vStreamIndex();
	int get_aStreamIndex();
private:
	//const char *outUrl = "rtmp://172.16.24.211/live";
	//const char *outUrl = "rtmp://hdlcontrol.com/live/stream";
	//const char *outUrl = "udp://172.16.24.169:2000";
	string outUrl;
	string streamFmt;	// flv, f4v, mpegts
	AVFormatContext *outFmtCtx;
	AVRational sysTimebase = { 1,1000000 };	// 系统时间戳: 微妙, 来之MediaEncode.h


	//视频编码器流
	const AVCodecContext *vEncodeCtx = NULL;
	const AVCodecContext *aEncodeCtx = NULL;
	//const AVCodecContext *subtitleEncodeCtx = NULL;

	// 流信息
	AVStream *vStream = NULL;		//  outFmtCtx->streams[i]
	AVStream *aStream = NULL;		//  outFmtCtx->streams[i]
	//AVStream *subtitleStream = NULL;		//  outFmtCtx->streams[i]
};


#endif	//OUTMEDIA_H
