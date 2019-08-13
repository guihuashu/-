#include <QtWidgets/QApplication>
#include <QDebug>
#include <test.h>
#include <QApplication>
#include <iostream>
#include <iostream>
#include <Decode.h>
#include <Demux.h>
#include <AudioPlay.h>
#include <VideoPlay.h>
using namespace std;

static Demux *demux;
static Decode *decode;
static AudioPlay *audioPlay;
static VideoPlay *vplay;

void playAudio()
{
	while (1) {
		//qWarning() << "pcmList.size = "<< decode->pcmList.size();
		Pcm *pcm = decode->getPcm();
		if (pcm) {
			//free_Pcm(pcm);
			audioPlay->writePcm(pcm);
            //CUR;
		}
        av_usleep(50);
	}
}

void playVideo()
{
	while (1) {

		//qWarning() << decode->rgb32List.size();
        Rgb24 *rgb32 = decode->getRgb24();
		if (rgb32) {
			//free_Rgb32(rgb32);
            vplay->paintRgb24(rgb32);
		}
        av_usleep(1000 * 1000 / 15);    // 最多15帧
	}
}


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	int ret;
    const char *path = "rtmp://hdlcontrol.com/live/stream";


	//av_register_all();	//初始化封装库
	avformat_network_init();	//初始化网络库 （可以打开rtsp rtmp http 协议的流媒体视频）
	//avcodec_register_all();	//注册解码器
	qInfo() << avformat_configuration();
	demux = new Demux(path);
	decode = new Decode(demux);
	decode->start();				// 开启线程来读取并解压包
#if 1
    vplay = new VideoPlay();
    thread thVplay(playVideo);
    thVplay.detach();
#endif

    audioPlay = new AudioPlay();
	if (!audioPlay->open()) {
		qWarning() << "ERR: audioPlay->open()";
		getchar();
	}
    thread thAplay(playAudio);
	thAplay.detach();
	a.exec();
}
