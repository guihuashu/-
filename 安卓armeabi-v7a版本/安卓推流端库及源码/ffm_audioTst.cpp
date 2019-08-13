#include "ffm_audioTst.h"
#include <QDebug>
using namespace std;

void dump_audio_fmt(AVFormatContext *fmtctx)
{
	qDebug() << "--------------  dump_audio_fmt  --------------------";
	qDebug() << fmtctx->streams[0]->codecpar->format;	// AV_SAMPLE_FMT_S16
	qDebug() << fmtctx->streams[0]->codecpar->bit_rate;	// 1411200
	qDebug() << fmtctx->streams[0]->codecpar->bits_per_coded_sample;	// 样本大小16(bit)
	qDebug() << fmtctx->streams[0]->codecpar->channel_layout;	// 0
	qDebug() << fmtctx->streams[0]->codecpar->channels;			// 2
	qDebug() << fmtctx->streams[0]->codecpar->sample_rate;		// 44100
	qDebug() << fmtctx->streams[0]->codecpar->frame_size;		// 0
}


int audio_push()
{
	//注册所有的编解码器, 封装器, 网络协议
	avcodec_register_all();
	av_register_all();
	avformat_network_init();

	//char *outUrl = "rtmp://172.16.24.211/live";
    const char *outUrl = "rtmp://hdlcontrol.com/live/stream";
	MediaEncode mediaEncode;
	OutMedia outMedia; 
	AVPacket *aPkt;

	if (!mediaEncode.init_avEncode()) {
		qDebug() << "ERR:init_audio";
		exit(0);
	}
	av_usleep(2000000);

	if (!outMedia.init_outMedia(string(outUrl), string("flv")))
			exit(0);
	if (!outMedia.addStream(mediaEncode.aEncodeCtx))
		exit(0);
	//qDebug() << ;
	outMedia.write_headerInfo();

	for (;;)
	{
        qDebug() << mediaEncode._aPktList.size();
		aPkt = mediaEncode.gain_aPkt();
		if (!outMedia.send_aPkt(aPkt))
			continue;
		//av_usleep(1000 * 20);
	}
}
