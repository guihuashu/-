#include <QDebug>
#include <QImage>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <MediaEncode.h>
#include <ffm_videoTst.h>
#include <util.h>
#include <OutMedia.h>
#include <VideoCap.h>
using namespace cv;
using namespace std;

void start_push_flow()
{
	/* 1.初始化所有设备.网络模块.编解码器 */
	ffm_init();
	//const char *outUrl = "rtmp://172.16.24.211/live";
	//const char *outUrl = "rtmp://hdlcontrol.com/live/stream";
	//const char *outUrl = "udp://172.16.24.169:2000";


	MediaEncode videoEncode;
	OutMedia outMedia;
	AVPacket  *vPkt;


	outMedia.init_outMedia(string("rtmp://hdlcontrol.com/live/stream"), string("flv"));

	if (!videoEncode.init_avEncode()) {
		cout << "ERR: structure_videoEncode" << endl;
		exit(0);
	}

	outMedia.addStream(videoEncode.vEncodeCtx);
	outMedia.dump_outMediaFmt();


	/* 7.打开输出流IO, 并写入立媒体头*/
	outMedia.write_headerInfo();
	for (;;)
	{
		qDebug() << videoEncode.vPktList.size();
		vPkt = videoEncode.gain_vPkt();
		if (!outMedia.send_vPkt(vPkt))
			continue;
		av_usleep(1000 * 30);	// 大于35会有重影
	}
}






