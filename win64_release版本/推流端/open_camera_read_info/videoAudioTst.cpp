#include <videoAudioTst.h>
using namespace std;
using namespace cv;
#include <QThreadPool>
#include <QRunnable>
#include <thread>
#include <mutex>




void rtmp_audio_video()
{	//注册所有的编解码器, 封装器, 网络协议
	avcodec_register_all();
	av_register_all();
	avformat_network_init();

	char *outUrl = "rtmp://hdlcontrol.com/live/stream";
	MediaEncode mediaEncode;
	OutMedia outMedia;
	AVPacket *aPkt;
	AVPacket *vPkt;

	if (!mediaEncode.init_avEncode()) {
		qDebug() << "ERR:init_audio";
		exit(0);
	}

	if (!outMedia.init_outMedia(string(outUrl), string("flv")))
		exit(0);

	/* 增加音频流和视频流 */
	outMedia.addStream(mediaEncode.aEncodeCtx);
	outMedia.addStream(mediaEncode.vEncodeCtx);
	outMedia.dump_outMediaFmt();
	outMedia.write_headerInfo();

	while(1)
	{
		aPkt = mediaEncode.gain_aPkt();
		vPkt = mediaEncode.gain_vPkt();
		/* 必须要这样, 否则音频卡顿 */
		if (!aPkt && !vPkt) {
			continue;
		}
		if (aPkt) {
			outMedia.send_aPkt(aPkt);
		}
		if (vPkt) {
			outMedia.send_vPkt(vPkt);
		}
		//av_usleep(1000 * 20);

	}

}
