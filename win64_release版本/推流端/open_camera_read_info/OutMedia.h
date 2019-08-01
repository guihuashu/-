#ifndef OUTMEDIA_H
#define OUTMEDIA_H
#include <iostream>
#include <ffm_videoTst.h>

using namespace std;
class OutMedia
{
public:
	OutMedia();
	~OutMedia();
	bool init_outMedia(string outUrl, string streamFmt);
	bool addStream(AVCodecContext *encodeCtx);
	void dump_outMediaFmt();
	bool write_headerInfo();
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
