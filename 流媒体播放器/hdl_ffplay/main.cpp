#include <QtWidgets/QApplication>
#include <QDebug>
#include <test.h>
#include <QApplication>
#include <iostream>
#include <widget.h>
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
		//qDebug() << "pcmList.size = "<< decode->pcmList.size();
		Pcm *pcm = decode->getPcm();
		if (pcm) {
			//free_Pcm(pcm);
			audioPlay->writePcm(pcm);
		}
	}
}

void playVideo()
{
	while (1) {

		//qDebug() << decode->rgb32List.size();
		Rgb32 *rgb32 = decode->getRgb32();
		if (rgb32) {
			//free_Rgb32(rgb32);
			vplay->paintRgb32(rgb32);

		}
		av_usleep(1000 * 1000 / 30);
	}
}


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Widget w;

	int ret;
	char *path = "rtmp://hdlcontrol.com/live/stream";


	//av_register_all();	//初始化封装库
	avformat_network_init();	//初始化网络库 （可以打开rtsp rtmp http 协议的流媒体视频）
	//avcodec_register_all();	//注册解码器
	demux = new Demux(path);
	decode = new Decode(demux);
	decode->start();				// 开启线程来读取并解压包
	audioPlay = new AudioPlay();
	if (!audioPlay->open()) {
		qDebug() << "ERR: audioPlay->open()";
		exit(0);
	}
	thread thAplay(playAudio);
	thAplay.detach();
	vplay = new VideoPlay();
	thread thVplay(playVideo);
	thVplay.detach();

	a.exec();
}
